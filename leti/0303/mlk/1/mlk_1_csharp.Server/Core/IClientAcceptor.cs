using System;
using System.Threading;
using System.Threading.Tasks;

namespace DevoidTalk.Server
{
    public interface IClientAcceptor
    {
        event EventHandler<ClientConnection> ClientAccepted;
        Task Listen(CancellationToken cancellation);
    }
}

