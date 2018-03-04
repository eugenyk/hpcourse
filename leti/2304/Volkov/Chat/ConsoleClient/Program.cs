using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Threading;
using ProtoBuf;

namespace ConsoleClient
{
    class Program
    {
        private const string host = "127.0.0.1";
        private const int port = 8888;
        static TcpClient client;
        static NetworkStream stream;
        static Message protomsg;// protobuf message

        static void Main(string[] args)
        {
            Console.OutputEncoding = Encoding.UTF8;
            Console.InputEncoding = Encoding.UTF8;
            Console.Write("Your name: ");

            protomsg = new Message();
            protomsg.Sender = Console.ReadLine();// get username
            client = new TcpClient();
            try
            {
                client.Connect(host, port); //подключение клиента
                stream = client.GetStream(); // получаем поток
                
                string message = protomsg.Sender;
                byte[] data = Encoding.Unicode.GetBytes(message);
                stream.WriteAsync(data, 0, data.Length);// send UserName to Stream

                // запускаем новый поток для получения данных
                Thread receiveThread = new Thread(new ThreadStart(ReceiveMessage));
                receiveThread.Start(); //старт потока
                Console.Out.WriteLineAsync("Welcome "+ protomsg.Sender);
                SendMessage();
            }
            catch (Exception ex)
            {
                Console.Out.WriteLineAsync(ex.Message);
            }
            finally
            {
                Disconnect();
            }
        }
        // отправка сообщений
        static void SendMessage()
        {
            Console.Out.WriteLineAsync("Message: ");
            //protomsg = new Message();
            while (true)
            {// in an infinite loop We get messages user types
                protomsg.Text = Console.ReadLine();
                protomsg.Text = protomsg.Text.Length + "|" + protomsg.Text;// add Length of a message
                byte[] data = Encoding.Unicode.GetBytes(protomsg.Text);
                //int i = message.Length;
                stream.WriteAsync(data, 0, data.Length);
            }
        }
        // получение сообщений
        static async void ReceiveMessage()
        {
            while (true)
            {
                try
                {
                    byte[] data = new byte[64]; // буфер для получаемых данных
                    StringBuilder builder = new StringBuilder();
                    int bytes = 0;
                    do
                    {
                        bytes = await stream.ReadAsync(data, 0, data.Length);
                        builder.Append(Encoding.Unicode.GetString(data, 0, bytes));
                    }
                    while (stream.DataAvailable);

                    string message = builder.ToString();
                    await Console.Out.WriteLineAsync(message);//вывод сообщения
                }
                catch(Exception e)
                {
                    await Console.Out.WriteLineAsync("Connection terminated!"); //соединение было прервано
                    await Console.Out.WriteLineAsync(e.Message);
                    Console.ReadLine();
                    Disconnect();
                }
            }
        }

        static void Disconnect()
        {
            if (stream != null)
                stream.Close();//отключение потока
            if (client != null)
                client.Close();//отключение клиента
            Environment.Exit(0); //завершение процесса
        }
    }// Program
}
/*
 * http://putridparrot.com/blog/using-protobuf-net-in-c/
 * https://code.google.com/archive/p/protobuf-net/wikis/GettingStarted.wiki
 * 
 * 
 * 
 * 
 * 
 * 
 * */
