using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Diagnostics;
using System.Threading.Tasks;

using System.Windows.Forms;

using System.Net;
using System.Net.Sockets;

using Messenger.Utils;
using Server.DataModel;

namespace Server
{
    public class Server
    {
        private const int PORT = 33501;
        private const int MAX_REQUEST_COUNT = 32;

        private ServerDataManagment db;

        private MainForm log;

        private List<KeyValuePair<string, Socket>> connections = new List<KeyValuePair<string, Socket>>(); 

        public Server(MainForm log_dest)
        {
            db = new ServerDataManagment();

            log = log_dest;

            log.WriteLog("Установлено соединение с базой данных.");
        }

        public void Start()
        {
            IPEndPoint my_ip = new IPEndPoint(Tools.GetMyIP(), PORT);

            Socket nw_obj = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                nw_obj.Bind(my_ip);

                nw_obj.Listen(MAX_REQUEST_COUNT);

                log.WriteLog("Сервер запущен. Ожидание подключений.");

                ThreadPool.SetMaxThreads(MAX_REQUEST_COUNT, MAX_REQUEST_COUNT);
                while (true)
                {
                    Socket client_connection = nw_obj.Accept();

                    //Может быть косяк
                    log.WriteLog(Tools.GetIPFromSocket(client_connection) + " - Подключен");

                    ThreadPool.QueueUserWorkItem(new WaitCallback(delegate 
                    {
                        ClientHandler client = new ClientHandler(client_connection, db, log, connections);
                        client.Start();
                    }));
                }
            }
            catch (Exception ex) {/*дописать*/ MessageBox.Show(ex.Message); }
        }

        public void Stop()
        {
            db.CloseConnection();
            foreach (var conn in connections)
            {
                if (conn.Value.Connected)
                    conn.Value.Close();
            }
            Process.GetCurrentProcess().Kill();    
        }
    }
}
