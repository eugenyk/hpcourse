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

                IPHostEntry ipHost = Dns.GetHostEntry("localhost");
                IPAddress ipAddr = ipHost.AddressList[0];
                IPEndPoint ipEndPoint = new IPEndPoint(ipAddr, 11000);

                sender = new Socket(ipAddr.AddressFamily, SocketType.Stream, ProtocolType.Tcp);                
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
                    ClearLine();                    
                    protomsg = MessageCreator(text);
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
        public static void ClearLine()
        {
            Console.SetCursorPosition(0, Console.CursorTop - 1);
            Console.Out.WriteAsync(new string(' ', Console.WindowWidth));
            Console.SetCursorPosition(0, Console.CursorTop - 1);
        }

        public static Message MessageCreator(string text){
            string pText;            
            string pData;
            switch (text){
                case "Join":
                    pData = "Join";
                    pText = "";
                    break;
                case "/e":
                    pData = "Exit";
                    pText = "";
                    break;
                case "/c":
                    pData = "ServerClose";
                    pText = "";
                    break;
                default:
                    pData = "Message";
                    pText = text;
                    break;
            }
            return new Message() { Data = pData, Sender = "Human", Text = pText };
        }

        public static void ReceiveMessage(){
            while (!exit)
            {
                try{
                    var bytes = new byte[1024];
                    var testmsg = ProtoRecieve(sender,bytes);
                    if (testmsg.Text == "Завершение работы сервера") exit = true;
                    Console.Out.WriteLineAsync(testmsg.Text);
                }
                catch (Exception ex)
                {
                    Console.Out.WriteLineAsync("exeption test");
                    Console.Out.WriteLineAsync(ex.ToString());
                }
            }
        }
        private static Message ProtoRecieve(Socket socket, byte[] buffer)        {
            var readedLength = 0;
            while (readedLength < 4)            {
                readedLength += socket.Receive(buffer, readedLength, 1, SocketFlags.None);
            }
            var dataLength = BitConverter.ToInt32(buffer, 0);
            readedLength -= 4;
            while (readedLength < dataLength)            {
                readedLength += socket.Receive(buffer, readedLength, 1, SocketFlags.None);
            }
            return Message.Parser.ParseFrom(buffer, 0, dataLength);
        }
    }
}
