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
                var afunc = new AsyncFunctions();
                afunc.Connect(ipEndPoint, sender);                
                
                Thread receiveThread = new Thread(ReceiveMessage);
                receiveThread.Start();

                protomsg = new Message() { Data = "Join", Sender = "Human", Text = "" };
                afunc.Send(sender, protomsg);

                while (!exit)
                {
                    var text = Console.ReadLine();
                    ClearLine();                    
                    protomsg = MessageCreator(text);
                    afunc.sendDone.WaitOne();
                    afunc.Send(sender, protomsg);                    
                }    
                sender.Shutdown(SocketShutdown.Both);
                sender.Close();
               
            }
            catch (Exception ex){
                Console.WriteLine(ex.ToString());
            }
            finally{
                Console.ReadLine();
            }
            
        }
        
        public static void ClearLine(){
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
            var afunc = new AsyncFunctions();
            while (!exit){
                try{
                    afunc.Receive(sender);
                    afunc.receiveDone.WaitOne();
                    if (afunc.message == null) continue;
                    var testmsg = afunc.message;
                    if (testmsg.Text == "Завершение работы сервера") exit = true;
                    Console.Out.WriteLineAsync(testmsg.Text);
                    afunc.message = null;                    
                }
                catch (Exception ex)
                {
                    Console.Out.WriteLineAsync("exeption test");
                    Console.Out.WriteLineAsync(ex.ToString());
                }
            }
        }
    }
}
