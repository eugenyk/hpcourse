using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Configuration;
using System.Net.Http;
using System.Net.Sockets;
using Google.Protobuf;
using System.Threading;
using Tutorial;


namespace IDZCs
{
    class Program
    {
        private static bool _working;
        private static List<Socket> _clients;
        private static bool _exit;
        private static List<List<Socket>> _conectionsList;
        private static int threadCapacity;

        static void Main(string[] args){
            _exit = false;
            _working = true;
            _clients = new List<Socket>();
            Console.Out.WriteAsync("Для запуска сервера введите '1'\nДля запуска клиента введите '0'\n");
            var choice = Console.ReadLine();
            if (choice == "0") Client.Send();
            Console.Out.WriteLineAsync("Введите количество потоков"); 
            threadCapacity = Convert.ToInt32(Console.ReadLine());
            ThreadPool.SetMaxThreads(threadCapacity, threadCapacity);            
            ThreadPool.SetMinThreads(2, 2);
            ServerStart();
            }

        private static void ServerStart(){
            IPHostEntry ipHost = Dns.GetHostEntry("localhost");
            IPAddress ipAddr = ipHost.AddressList[0];
            IPEndPoint ipEndPoint = new IPEndPoint(ipAddr, 11000);
            Socket sListener = new Socket(ipAddr.AddressFamily, SocketType.Stream, ProtocolType.Tcp); 
            AsyncFunctions afunc = new AsyncFunctions();
            
            try{
                sListener.Bind(ipEndPoint);
                sListener.Listen(10);                
                Console.Out.WriteLineAsync($"Ожидаем соединение через порт {ipEndPoint}");
                while (_working){
                    afunc.Accept(sListener);
                    afunc.aceptDone.WaitOne();
                }
            }
            catch (Exception ex){
                Console.Out.WriteLineAsync(ex.ToString());
            }
            finally {
                Console.ReadLine();
            }
        }
        private static void ClientHandlerOld(Object socket){
            var afunc = new AsyncFunctions();
            var handler = (Socket)socket;
            _clients.Add(handler);
            Console.WriteLine();            
            while (!_exit) {                
                afunc.Receive(handler);
                afunc.receiveDone.WaitOne();
                var testmsg = afunc.message;
                if (!MessageHandler(testmsg)){
                    break;
                }
            }
            _clients.Remove(handler);
            handler.Shutdown(SocketShutdown.Both);
            handler.Close();
        }

        private static void ClientHandler(Object list)
        {
            var listOfSockets = (List<Socket>)list;
            foreach (var client in listOfSockets){
                //async listen
            }
            while (!_exit){
                var testmsg = LockFreeQueue.Pop();
                if (testmsg != null){
                    if (!MessageHandler(testmsg)){
                        break;
                    }
                }
            }
        }

        public static void AddConnection(Socket socket){
            if (_conectionsList.Count < threadCapacity){
                var list = new List<Socket> {socket};
                _conectionsList.Add(list);
                ThreadPool.QueueUserWorkItem(ClientHandler, list);
            }
            else{
                List<Socket> soсketList = _conectionsList[0];
                foreach (var list in _conectionsList){
                    if (soсketList.Count > list.Count) soсketList = list;
                }
                soсketList.Add(socket);
            }
        }

        private static bool MessageHandler(Message message){
            string line;
            switch (message.Data){
                case "Exit":
                    line = message.Sender + " покинул чат.";
                    Console.Out.WriteLineAsync(line);
                    Broadcast(line);
                    return false;
                case "Join":
                    line = message.Sender + " присоеденился!";
                    Console.Out.WriteLineAsync(line);
                    Broadcast(line);
                    return true;
                case "ServerClose":
                    line = "Завершение работы сервера";
                    Console.Out.WriteLineAsync(line);
                    Broadcast(line);
                    _exit = true;
                    return true;
                default:
                    line = message.Sender + ": " + message.Text;
                    Console.Out.WriteLineAsync(line);
                    Broadcast(line);
                    return true;
            }
        }

        private static void Broadcast(string text){
            var afunc = new AsyncFunctions();
            foreach (var list in _conectionsList){
                foreach (var client in list){
                    var protomsg = new Message() { Data = "", Sender = "Server", Text = text };
                    afunc.sendDone.Set();
                    afunc.Send(client, protomsg);
                }
            }
        }
    }
}
