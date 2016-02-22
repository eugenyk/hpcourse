using Carramba.Codex;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Carramba.Tortuga
{
    class Server
    {
        ConcurrentDictionary<TcpClient, Connection> clientBase = new ConcurrentDictionary<TcpClient, Connection>();

        public Server()
        {

        }

        public Task Start()
        {
            return Listen();
        }

        private async Task Listen()
        {
            TcpListener tcpListener = new TcpListener(IPAddress.Any, 10000);
            tcpListener.Start();

            while (true)
            {
                TcpClient client = await tcpListener.AcceptTcpClientAsync();
                var unnecessary = WorkWith(client).ContinueWith(task=>ClientDisconnection(task, client));
            }  
        }

        private async Task WorkWith(TcpClient client)
        {
            var connection = new Connection(client); 
            clientBase.TryAdd(client, connection);
            while (true)
            {
                Message message = await connection.ReadMessage();
                foreach (Connection con in clientBase.Values)
                {
                    try
                    {
                        await con.SendMessage(message);
                    }
                    catch (Exception ex) { Console.WriteLine("POLUNDRA!!!: {0}", ex); }
                }

            }
        }

        private void ClientDisconnection(Task zabort, TcpClient client)
        {
            Connection connect;
            clientBase.TryRemove(client, out connect);
        }
    }
}
