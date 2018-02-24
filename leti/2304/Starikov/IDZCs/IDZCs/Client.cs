using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Tutorial;
using System.Net.Sockets;
using System.Threading;
using Google.Protobuf;

namespace IDZCs
{
    class Client{
        private static bool exit;
        private static Socket sender;
        public static void Send()
        {
            try
            {
                exit = false;
                byte[] bytes = new byte[1024];
                var protomsg = new Message();
                // Соединяемся с удаленным устройством

                // Устанавливаем удаленную точку для сокета
                IPHostEntry ipHost = Dns.GetHostEntry("localhost");
                IPAddress ipAddr = ipHost.AddressList[0];
                IPEndPoint ipEndPoint = new IPEndPoint(ipAddr, 11000);

                sender = new Socket(ipAddr.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

                // Соединяем сокет с удаленной точкой
                sender.Connect(ipEndPoint);
                Console.WriteLine("Сокет соединяется с {0} ", sender.RemoteEndPoint);

                Thread receiveThread = new Thread(ReceiveMessage);
                receiveThread.Start(); //старт потока

                protomsg = new Message() { Data = "Join", Sender = "Human", Text = "" };
                var size = BitConverter.GetBytes(protomsg.CalculateSize());
                int bytesSent = sender.Send(size);
                var message = protomsg.ToByteArray();
                bytesSent = sender.Send(message);

                while (!exit)
                {
                    var text = Console.ReadLine();
                    protomsg = new Message() { Data = "Message", Sender = "Human", Text = text };
                    size = BitConverter.GetBytes(protomsg.CalculateSize());
                    bytesSent = sender.Send(size);
                    message = protomsg.ToByteArray();
                    bytesSent = sender.Send(message);
                }    
                sender.Shutdown(SocketShutdown.Both);
                sender.Close();
               
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

        public static void ReceiveMessage(){
            while (!exit)
            {
                try{
                    var bytes = new byte[1024];
                    var bytesRec = sender.Receive(bytes);
                    var messageSize = BitConverter.ToInt32(bytes, 0);                    
                    var testmsg = Message.Parser.ParseFrom(bytes, 4, messageSize);
                    if (testmsg.Text == "Завершение работы сервера") exit = true;
                    Console.WriteLine(testmsg.Text);

                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex.ToString());
                }
            }
        }
    }
}
