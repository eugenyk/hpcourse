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
        private static LockFreeList _lockFreeList;
        private static bool exit;

        static void Main(string[] args){
            exit = false;
            _working = true;
            _clients = new List<Socket>();
            _lockFreeList = new LockFreeList();
            Console.Out.WriteAsync("Для запуска сервера введите '1'\nДля запуска клиента введите '0'\n");
            var choice = Console.ReadLine();
            if (choice == "0") Client.Send();
            Console.Out.WriteLineAsync("Введите количество потоков");
            var threadCapacity = Convert.ToInt32(Console.ReadLine());
            ThreadPool.SetMaxThreads(threadCapacity, threadCapacity);            
            ThreadPool.SetMinThreads(2, 2);
            ServerStart();
            }

        private static void ServerStart(){
            IPHostEntry ipHost = Dns.GetHostEntry("localhost");
            IPAddress ipAddr = ipHost.AddressList[0];
            IPEndPoint ipEndPoint = new IPEndPoint(ipAddr, 11000);
            
            Socket sListener = new Socket(ipAddr.AddressFamily, SocketType.Stream, ProtocolType.Tcp);            
            try{
                sListener.Bind(ipEndPoint);
                sListener.Listen(10);                
                Console.Out.WriteLineAsync($"Ожидаем соединение через порт {ipEndPoint}");
                while (_working){                    
                    ThreadPool.QueueUserWorkItem(ClientHandler, sListener.Accept());                    
                }
            }
            catch (Exception ex)            {
                Console.Out.WriteLineAsync(ex.ToString());
            }
            finally            {
                Console.ReadLine();
            }
        }
        private static void ClientHandler(Object socket)
        {
            var afunc = new AsyncFunctions();
            var handler = (Socket)socket;
            _clients.Add(handler);
            Console.WriteLine();            
            while (!exit) {                
                afunc.Receive(handler);
                afunc.receiveDone.WaitOne();
                if (afunc.message == null) continue;
                var testmsg = afunc.message;
                //var testmsg = ProtoRecieve(handler,bytes);                
                if (!MessageHandler(testmsg)){
                    break;
                }
                afunc.message = null;
            }
            _clients.Remove(handler);
            handler.Shutdown(SocketShutdown.Both);
            handler.Close();
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
                    exit = true;
                    return true;
                default:
                    _lockFreeList.Add(message.Text);
                    line = message.Sender + ": " + message.Text;
                    Console.Out.WriteLineAsync(line);
                    Broadcast(line);
                    return true;
            }
        }

        private static void Broadcast(string text){
            var afunc = new AsyncFunctions();
            foreach (var client in _clients){
                var protomsg = new Message() { Data = "", Sender = "Server", Text = text };
                afunc.sendDone.Set();
                afunc.Send(client,protomsg);                
            }
        }
    }
}
