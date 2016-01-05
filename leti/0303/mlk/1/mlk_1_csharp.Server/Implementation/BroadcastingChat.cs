using DevoidTalk.Core;
using NLog;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevoidTalk.Server
{
    public sealed class BroadcastingChat
    {
        static readonly Logger logger = LogManager.GetCurrentClassLogger();

        readonly ConnectionManager connectionManager;
        readonly string welcomeMessage;

        public Func<IncomingMessage, Task> IncomingMessageStrategy { get; set; }

        public BroadcastingChat(ConnectionManager connectionManager, string welcomeMessage)
        {
            this.connectionManager = connectionManager;
            this.welcomeMessage = welcomeMessage;

            connectionManager.ClientConnected += ClientConnected;
            connectionManager.ClientDisconnected += ClientDisconnected;
            connectionManager.IncomingMessage += IncomingMessage;
        }

        private async void ClientConnected(object sender, ClientConnection connection)
        {
            try
            {
                await BroadcastToAll(new Message { Sender = "<server>", Text = $"{connection} connected" });
                await ReplyTo(connection, new Message { Sender = "<server>", Text = welcomeMessage });
            }
            catch (Exception ex)
            {
                logger.Warn(ex, "{0} connected handling", connection);
            }
        }

        private async void ClientDisconnected(object sender, ClientConnection connection)
        {
            try
            {
                await BroadcastToAll(new Message { Sender = "<server>", Text = $"{connection} disconnected" });
            }
            catch (Exception ex)
            {
                logger.Warn(ex, "{0} disconnected handling", connection);
            }
        }

        private async void IncomingMessage(object sender, IncomingMessage e)
        {
            var strategy = IncomingMessageStrategy;
            if (strategy == null) { return; }
            try
            {
                await strategy(e);
            }
            catch (Exception ex)
            {
                logger.Warn(ex, "Incoming message from {0} handling", e.Sender);
            }
        }

        public async Task BroadcastToAll(Message message)
        {
            await Task.Yield();
            var tasks = from client in connectionManager.Clients
                        select client.SendMessage(message);
            await Task.WhenAll(tasks);
        }

        public async Task ReplyTo(ClientConnection client, Message message)
        {
            await Task.Yield();
            await client.SendMessage(message);
        }
    }
}
