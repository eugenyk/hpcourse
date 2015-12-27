using DevoidTalk.Core;
using System;
using System.Net;
using System.Net.Sockets;

namespace DevoidTalk.Server
{
    public sealed class ClientConnection : Connection
    {
        public IPEndPoint EndPoint { get; }

        public ClientConnection(Socket socket)
            : base(socket)
        {
            EndPoint = socket.RemoteEndPoint as IPEndPoint;
        }

        public override string ToString()
        {
            string endPoint = EndPoint == null
                ? "<unknown>" : EndPoint.ToString();
            return $"client@{endPoint}";
        }
    }
}

