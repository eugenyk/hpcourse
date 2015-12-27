using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace DevoidTalk.Core
{
    public static class AsyncExtensions
    {
        public static Task ConnectTaskAsync(
            this Socket socket, IPAddress[] adresses, int port)
        {
            var tcs = new TaskCompletionSource<bool>(socket);
            socket.BeginConnect(adresses, port, iar =>
            {
                var t = (TaskCompletionSource<bool>)iar.AsyncState;
                var s = (Socket)t.Task.AsyncState;
                try
                {
                    s.EndConnect(iar);
                    t.TrySetResult(true);
                }
                catch (Exception exc) { t.TrySetException(exc); }
            }, tcs);
            return tcs.Task;
        }

        public static Task DisconnectTaskAsync(
            this Socket socket, bool reuseSocket)
        {
            var tcs = new TaskCompletionSource<bool>(socket);
            socket.BeginDisconnect(reuseSocket, iar =>
            {
                var t = (TaskCompletionSource<bool>)iar.AsyncState;
                var s = (Socket)t.Task.AsyncState;
                try
                {
                    s.EndDisconnect(iar);
                    t.TrySetResult(true);
                }
                catch (Exception exc) { t.TrySetException(exc); }
            }, tcs);
            return tcs.Task;
        }

        public static Task<int> ReceiveTaskAsync(
            this Socket socket, byte[] buffer, int offset, int size, SocketFlags socketFlags)
        {
            var tcs = new TaskCompletionSource<int>(socket);
            socket.BeginReceive(buffer, offset, size, socketFlags, iar =>
            {
                var t = (TaskCompletionSource<int>)iar.AsyncState;
                var s = (Socket)t.Task.AsyncState;
                try { t.TrySetResult(s.EndReceive(iar)); }
                catch (Exception exc) { t.TrySetException(exc); }
            }, tcs);
            return tcs.Task;
        }

        public static Task<int> SendTaskAsync(
            this Socket socket, byte[] buffer, int offset, int size, SocketFlags socketFlags)
        {
            var tcs = new TaskCompletionSource<int>(socket);
            socket.BeginSend(buffer, offset, size, socketFlags, iar =>
            {
                var t = (TaskCompletionSource<int>)iar.AsyncState;
                var s = (Socket)t.Task.AsyncState;
                try { t.TrySetResult(s.EndSend(iar)); }
                catch (Exception exc) { t.TrySetException(exc); }
            }, tcs);
            return tcs.Task;
        }
    }
}
