using BenchmarkDotNet.Attributes;
using BenchmarkDotNet.Configs;
using BenchmarkDotNet.Jobs;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace HopscotchHashMap
{
    [Config(typeof(Config))]
    public class MapBenchmarks
    {
        private class Config : ManualConfig
        {
            public Config()
            {
                Add(Job.Default.With(Mode.SingleRun).WithLaunchCount(1).WithWarmupCount(1).WithTargetCount(1));
            }
        }

        private const int MapCapacity = 16383;

        public MapBenchmarks()
        {
        }

        [Benchmark]
        public void HopscotchMap()
        {
            var map = new HopscotchMap<int, int>(8, 10000);
            var bag = new ConcurrentBag<int>();
            var completitionSources = new TaskCompletionSource<bool>[8];
            int initialID = new Random().Next(1000000);
            for (int i = 0; i < completitionSources.Length; i++)
            {
                int j = i;
                completitionSources[j] = new TaskCompletionSource<bool>();
                new Thread(() =>
                {
                    ComplexTest(map, MapCapacity, bag, j + initialID);
                    completitionSources[j].SetResult(true);
                }).Start();
            }
            Task[] tasks = completitionSources.Select(t => t.Task).ToArray();
            Task.WaitAll(tasks);
            var errors = tasks.Where(t => t.IsFaulted).Select(t => t.Exception).ToArray();
            if (errors.Length > 0)
                throw new AggregateException(errors);
        }

        [Benchmark]
        public void ConcurrentDictionary()
        {
            var map = new ConcurrentDictionary<int, int>(8, 10000);
            var bag = new ConcurrentBag<int>();
            var completitionSources = new TaskCompletionSource<bool>[8];
            int initialID = new Random().Next(1000000);
            for (int i = 0; i < completitionSources.Length; i++)
            {
                int j = i;
                completitionSources[j] = new TaskCompletionSource<bool>();
                new Thread(() =>
                {
                    ComplexTest(map, MapCapacity, bag, j + initialID);
                    completitionSources[j].SetResult(true);
                }).Start();
            }
            Task[] tasks = completitionSources.Select(t => t.Task).ToArray();
            Task.WaitAll(tasks);
            var errors = tasks.Where(t => t.IsFaulted).Select(t => t.Exception).ToArray();
            if (errors.Length > 0)
                throw new AggregateException(errors);
        }

        static void ComplexTest(HopscotchMap<int, int> map, int capacity, ConcurrentBag<int> bag, int id)
        {
            var random = new Random(id);
            var clone = map;
            for (int i = 0; i < capacity * 1000; i++)
            {
                if (random.Next(100) < 40 || bag.Count > capacity * 2 / 3)
                {
                    int element;
                    if (bag.TryTake(out element))
                    {
                        int data;
                        if (!map.Remove(element, out data))
                        {
                            throw new InvalidOperationException("Cannot remove element");
                        }
                        if (data != element)
                            throw new InvalidOperationException("Data doesn't match key");
                    }
                }
                else
                {
                    int key = random.Next(capacity * 3);
                    var result = map.PutIfAbsent(key, key);

                    if (result == PutResult.Success)
                    {
                        if (!map.ContainsKey(key))
                        {
                            throw new InvalidOperationException("no key after insert");
                        }
                        bag.Add(key);
                    }
                }
            }
        }

        static void ComplexTest(ConcurrentDictionary<int, int> map, int capacity, ConcurrentBag<int> bag, int id)
        {
            var random = new Random(id);
            var clone = map;
            for (int i = 0; i < capacity * 1000; i++)
            {
                if (random.Next(100) < 40 || bag.Count > capacity * 2 / 3)
                {
                    int element;
                    if (bag.TryTake(out element))
                    {
                        int data;
                        if (!map.TryRemove(element, out data))
                        {
                            throw new InvalidOperationException("Cannot remove element");
                        }
                        if (data != element)
                            throw new InvalidOperationException("Data doesn't match key");
                    }
                }
                else
                {
                    int key = random.Next(capacity * 3);
                    var result = map.TryAdd(key, key);

                    if (result)
                    {
                        if (!map.ContainsKey(key))
                        {
                            throw new InvalidOperationException("no key after insert");
                        }
                        bag.Add(key);
                    }
                }
            }
        }
    }
}
