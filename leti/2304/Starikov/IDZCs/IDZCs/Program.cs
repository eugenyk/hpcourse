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
        private static List<string> _chatRoomList;
        private static bool _working;
        private static List<Socket> _clients;
        private static LockFreeList _lockFreeList;
        private static bool exit;

        static void Main(string[] args){
            exit = false;
            _chatRoomList = new List<string>();
            _working = true;
            _clients = new List<Socket>();
            _lockFreeList = new LockFreeList();
            Console.Write("Для запуска сервера введите '1'\nДля запуска клиента введите '0'\n");
            var choice = Console.ReadLine();
            if (choice == "0") Client.Send();
            Console.Write("Введите количество потоков\n");
            var threadCapacity = Convert.ToInt32(Console.ReadLine());
            ThreadPool.SetMaxThreads(30, 30);
            ThreadPool.SetMinThreads(2, 2);
            ServerStart();
            }

        private static void ServerStart(){
            IPHostEntry ipHost = Dns.GetHostEntry("localhost");
            IPAddress ipAddr = ipHost.AddressList[0];
            IPEndPoint ipEndPoint = new IPEndPoint(ipAddr, 11000);


            // Создаем сокет Tcp/Ip
            Socket sListener = new Socket(ipAddr.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
            // Назначаем сокет локальной конечной точке и слушаем входящие сокеты
            try
            {
                sListener.Bind(ipEndPoint);
                sListener.Listen(10);

                
                while (_working)
                {
                    Console.WriteLine($"Ожидаем соединение через порт {ipEndPoint}");
                    ThreadPool.QueueUserWorkItem(ClientHandler, sListener.Accept());
                    
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
            }
            finally
            {
                Console.ReadLine();
            }
        }
        private static void ClientHandler(Object socket)
        {
            var handler = (Socket)socket;
            _clients.Add(handler);
            Console.WriteLine();
            while (!exit)            {
                var bytes = new byte[1024];
                var bytesRec = handler.Receive(bytes);
                var messageSize = BitConverter.ToInt32(bytes, 0);
                //bytesRec = handler.Receive(bytes);
                var testmsg = Message.Parser.ParseFrom(bytes, 4, messageSize);
                if (!MessageHandler(testmsg)){
                    break;
                }
                
            }
            handler.Shutdown(SocketShutdown.Both);
            handler.Close();
        }

        private static bool MessageHandler(Message message){
            string line;
            switch (message.Data){
                case "Exit":
                    line = message.Sender + " покинул чат.";
                    Console.WriteLine(line);
                    Broadcast(line);
                    return false;
                case "Join":
                    line = message.Sender + " присоеденился!";
                    Console.WriteLine(line);
                    Broadcast(line);
                    return true;
                case "ServerClose":
                    line = "Завершение работы сервера";
                    Console.WriteLine(line);
                    Broadcast(line);
                    exit = true;
                    return true;
                default:
                    _lockFreeList.Add(message.Text);
                    line = message.Sender + ": " + message.Text;
                    Console.WriteLine(line);
                    Broadcast(line);
                    return true;
            }
        }

        private static void Broadcast(string text){
            foreach (var client in _clients){
                var protomsg = new Message() { Data = "", Sender = "Server", Text = text };
                var size = BitConverter.GetBytes(protomsg.CalculateSize());
                var bytesSent = client.Send(size);
                var message = protomsg.ToByteArray();
                bytesSent = client.Send(message);
            }
        }
    }
}
