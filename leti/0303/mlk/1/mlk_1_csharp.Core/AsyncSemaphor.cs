using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevoidTalk.Core
{
    public sealed class AsyncSemaphore
    {
        readonly static Task completed = Task.FromResult(true);
        readonly Queue<TaskCompletionSource<bool>> waiters = new Queue<TaskCompletionSource<bool>>();
        int currentCount;

        public AsyncSemaphore(int initialCount)
        {
            if (initialCount < 0) { throw new ArgumentOutOfRangeException(nameof(initialCount)); }
            currentCount = initialCount;
        }

        public Task WaitAsync()
        {
            lock (waiters)
            {
                if (currentCount > 0)
                {
                    --currentCount;
                    return completed;
                }
                else
                {
                    var waiter = new TaskCompletionSource<bool>();
                    waiters.Enqueue(waiter);
                    return waiter.Task;
                }
            }
        }

        public void Release()
        {
            TaskCompletionSource<bool> toRelease = null;
            lock (waiters)
            {
                if (waiters.Count > 0)
                    toRelease = waiters.Dequeue();
                else
                    ++currentCount;
            }
            if (toRelease != null)
                toRelease.SetResult(true);
        }
    }
}
