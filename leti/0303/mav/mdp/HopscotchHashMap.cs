using System;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;

namespace HopscotchHashMap
{
    public enum PutResult { Success, Overflow, AlreadyExists }

    public sealed class HopscotchMap<TKey, TValue>
    {
        const int HashMask = 0x7FFFFFFF;
        const int EmptyHash = -1;
        const int BusyHash = -2;

        const int HopRange = 32;
        const int InsertRange = 4 * 1024;
        const int ResizeFactor = 2;

        public struct Bucket
        {
            public volatile uint hopInfo;
		    public volatile int hash;
		    public TKey key;
		    public TValue data;
            
		    public void Init()
            {
                hopInfo = 0;
                hash = EmptyHash;
                key = default(TKey);
                data = default(TValue);
            }
        }

        private sealed class Segment
        {
            public int timestamp;
        }

        IEqualityComparer<TKey> keyComparer = EqualityComparer<TKey>.Default;

        volatile int segmentShift;
	    volatile int segmentMask;
	    volatile int bucketMask;
	    volatile Segment[] segments;
	    public volatile Bucket[] table;

        public int Capacity => bucketMask;

        public int ApproximateCount
        {
            get
            {
                int counter = 0;
                int numElm = bucketMask + InsertRange;
                for (int iElm = 0; iElm < numElm; ++iElm)
                {
                    if (table[iElm].hash != EmptyHash)
                    {
                        ++counter;
                    }
                }
                return counter;
            }
        }

        public HopscotchMap(int concurrencyLevel, int initialCapacity)
	    {
            checked
            {
                segmentMask = (int)(NearestPowerOfTwo((uint)concurrencyLevel) - 1);
                segmentShift = CalcDivideShift(NearestPowerOfTwo(
                    (uint)concurrencyLevel / (NearestPowerOfTwo((uint)concurrencyLevel))) - 1);

                uint adjInitCap = NearestPowerOfTwo((uint)initialCapacity);
                uint adjConcurrencyLevel = NearestPowerOfTwo((uint)concurrencyLevel);
                int numBuckets = (int)(adjInitCap + InsertRange + 1);
                bucketMask = (int)(adjInitCap - 1);
                //_segmentShift = first_msb_bit_indx(_bucketMask) - first_msb_bit_indx(_SEGMENTS_MASK);

                segments = new Segment[segmentMask + 1];
                table = new Bucket[numBuckets];
            }

		    for (int i = 0; i < segments.Length; ++i)
            {
                segments[i] = new Segment();
            }

            for (int i = 0; i < table.Length; ++i)
            {
                table[i].Init();
            }

            Interlocked.MemoryBarrier();
	    }

        private HopscotchMap(HopscotchMap<TKey, TValue> original)
        {
            keyComparer = original.keyComparer;
            segmentShift = original.segmentShift;
            segmentMask = original.segmentMask;
            bucketMask = original.bucketMask;
            segments = new Segment[original.segments.Length];
            for (int i = 0; i < segments.Length; i++)
            {
                segments[i] = new Segment();
                segments[i].timestamp = original.segments[i].timestamp;
            }
            table = (Bucket[])original.table.Clone();
        }

        private bool FindCloserFreeBacket(Segment startSegment, ref int freeBacket, ref int freeDistance)
        {
            int moveBacket = freeBacket - (HopRange - 1);

		    for (int moveFreeDist = HopRange - 1; moveFreeDist > 0; --moveFreeDist) {
			    uint startHopInfo = table[moveBacket].hopInfo;

                int moveNewFreeDistance = -1;
                uint mask = 1;
			    for (int i = 0; i < moveFreeDist; ++i, mask <<= 1)
                {
				    if ((mask & startHopInfo) != 0)
                    {
                        moveNewFreeDistance = i;
					    break;
				    }
                }

			    if (moveNewFreeDistance != -1)
                {
				    Segment moveSegment = segments[((uint)moveBacket >> segmentShift) & segmentMask];
				
				    if (startSegment != moveSegment)
					    Monitor.Enter(moveSegment);

				    if (startHopInfo == table[moveBacket].hopInfo) {
					    int newFreeBacket = moveBacket + moveNewFreeDistance;
                        table[moveBacket].hopInfo |= (1u << moveFreeDist);
                        table[freeBacket].data = table[newFreeBacket].data;
					    table[freeBacket].key = table[newFreeBacket].key;
                        table[freeBacket].hash = table[newFreeBacket].hash;
                        
                        Interlocked.Increment(ref moveSegment.timestamp);

					    table[moveBacket].hopInfo &= ~(1u << moveNewFreeDistance);

                        freeDistance -= (freeBacket - newFreeBacket); // the correct way
                        freeBacket = newFreeBacket;
                        //freeDistance -= moveFreeDist; // error in article

                        if (startSegment != moveSegment)
                            Monitor.Exit(moveSegment);
					    return true;
				    }
				    if (startSegment != moveSegment)
					    Monitor.Exit(moveSegment);
			    }

			    ++moveBacket;
		    }

            return false;
	    }

        public HopscotchMap<TKey, TValue> Clone()
        {
            return new HopscotchMap<TKey, TValue>(this);
        }

        private bool BucketContainsKey(ref Bucket bucket, TKey key, int keyHash)
        {
            return keyHash == bucket.hash && keyComparer.Equals(key, bucket.key);
        }

        public bool ContainsKey(TKey key)
        {
            int hash = keyComparer.GetHashCode(key) & HashMask;

		    // check if already contains
            Segment segment = segments[(hash >> segmentShift) & segmentMask];
            int elmAryBucket = hash & bucketMask;
            uint hopInfo = table[elmAryBucket].hopInfo;

		    if (hopInfo == 0)
			    return false;
		    else if (hopInfo == 1u)
            {
			    return BucketContainsKey(ref table[elmAryBucket], key, hash);
		    }
            //else if(2U == hopInfo)
            //{
            //    int currElmBucket = elmAryBucket + 1;
            //    return BucketContainsKey(ref table[currElmBucket], key, hash);
            //}

            int startTimestamp = Volatile.Read(ref segment.timestamp);
		    while (hopInfo != 0)
            {
			    int i = IndexOfLeastSignificantBitSet(hopInfo);
                int currElmBucket = elmAryBucket + i;
			    if (BucketContainsKey(ref table[currElmBucket], key, hash))
				    return true;
			    hopInfo &= ~(1u << i);
		    }
            
		    if (Volatile.Read(ref segment.timestamp) == startTimestamp)
			    return false;
            
		    int currBucket = hash & bucketMask;
		    for (int i = 0; i < HopRange; ++i, ++currBucket)
            {
			    if (BucketContainsKey(ref table[currBucket], key, hash))
				    return true;
		    }
		    return false;
	    }

        public PutResult PutIfAbsent(TKey key,  TValue data)
        {
            int hash = keyComparer.GetHashCode(key) & HashMask;

		    // lock hash entry
		    Segment segment = segments[(hash >> segmentShift) & segmentMask];
            Monitor.Enter(segment);
		    int startBucket = hash & bucketMask;

            // check if already contains
            uint hopInfo = table[startBucket].hopInfo;
		    while (hopInfo != 0)
            {
                int i = IndexOfLeastSignificantBitSet(hopInfo);
                int currElmBucket = startBucket + i;
			    if (BucketContainsKey(ref table[currElmBucket], key, hash))
                {
                    Monitor.Exit(segment);
				    return PutResult.AlreadyExists;
			    }
                hopInfo &= ~(1u << i);
		    }

            // look for free bucket
            int freeBucket = startBucket;
            int freeDistance = 0;
		    for(; freeDistance < InsertRange; ++freeDistance, ++freeBucket)
            {
                int bucketHash = table[freeBucket].hash;
                if (bucketHash == EmptyHash && Interlocked.CompareExchange(ref table[freeBucket].hash, BusyHash, EmptyHash) == EmptyHash)
                    break;
		    }

		    // place new key
		    if (freeDistance < InsertRange)
            {
			    do
                {
				    if (freeDistance < HopRange)
                    {
                        table[freeBucket].data = data;
                        table[freeBucket].key = key;
                        table[freeBucket].hash = hash;
					    table[startBucket].hopInfo |= (1u << freeDistance);
					    Monitor.Exit(segment);
					    return PutResult.Success;
				    }
			    }
                while (FindCloserFreeBacket(segment, ref freeBucket, ref freeDistance));
		    }

            // need to resize
            //throw new InvalidOperationException($"Resize is not implemented yet (current size: {Count})");
            Monitor.Exit(segment);
            return PutResult.Overflow;
	    }

        public bool Remove(TKey key, out TValue data)
        {
            int hash = keyComparer.GetHashCode(key) & HashMask;

            // check if already contains
            Segment segment = segments[(hash >> segmentShift) & segmentMask];
            Monitor.Enter(segment);
		    int startBucket = hash & bucketMask;
            uint hopInfo = table[startBucket].hopInfo;

		    if (hopInfo == 0)
            {
			    Monitor.Exit(segment);
                data = default(TValue);
			    return false;
		    }
            else if (hopInfo == 1u)
            {
                if (BucketContainsKey(ref table[startBucket], key, hash))
                {
                    table[startBucket].hopInfo &= ~1u;
                    table[startBucket].hash = EmptyHash;
                    table[startBucket].key = default(TKey);
                    data = table[startBucket].data;
                    table[startBucket].data = default(TValue);
                    Monitor.Exit(segment);
                    return true;
                }
                else
                {
                    Monitor.Exit(segment);
                    data = default(TValue);
                    return false;
                }
		    }

		    do
            {
			    int i = IndexOfLeastSignificantBitSet(hopInfo);
                if (i < 0) { throw new InvalidOperationException(); }
                int currElmBucket = startBucket + i;
			    if (BucketContainsKey(ref table[currElmBucket], key, hash))
                {
				    uint mask = 1u << i;
				    table[startBucket].hopInfo &= ~mask;
				    table[currElmBucket].hash = EmptyHash;
                    table[currElmBucket].key = default(TKey);
				    data = table[currElmBucket].data;
                    table[currElmBucket].data = default(TValue);
                    Monitor.Exit(segment);
				    return true;
			    }

			    hopInfo &= ~(1U << i);
		    }
            while (hopInfo != 0);

            Monitor.Exit(segment);
            data = default(TValue);
            return false;
	    }

        static uint NearestPowerOfTwo(uint value)
        {
            uint rc = 1;
		    while (rc < value)
            {
			    rc <<= 1;
		    }
		    return rc;
	    }

        static int CalcDivideShift(uint value)
        {
            int numShift = 0;
            uint curr = 1;
            while (curr < value)
            {
                curr <<= 1;
                ++numShift;
            }
            return numShift;
        }

        static int IndexOfLeastSignificantBitSet(uint value)
        {
            if (value == 0)
                return -1;
            int position = 0;
            while ((value & (1u << position)) == 0)
            {
                ++position;
            }
            return position;
        }
    }
}
