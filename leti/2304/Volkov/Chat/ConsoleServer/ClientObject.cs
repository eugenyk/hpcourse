using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.IO;

namespace ConsoleServer
{
    public class ClientObject
    {
        protected internal string Id { get; private set; }// unique for a client
        protected internal NetworkStream Stream { get; private set; }// Через данный объект можно передавать сообщения серверу или, наоборот, получать данные с сервера
        string Sender;
        string Data;//< Internal field. Можно использовать для реализации каких-либо собственных фич у своего сервера/клиента. Однако, стоит помнить, что все сервера-клиенты должны быть совместимы друг с другом
        TcpClient client;
        ServerObject server; // объект сервера

        public ClientObject(TcpClient tcpClient, ServerObject serverObject)
        {
            Id = Guid.NewGuid().ToString();
            client = tcpClient;// TCP
            server = serverObject;
            serverObject.AddConnection(this);// добавление в коллекцию подключений класса ServerObject
        }

        public async void Process(Object stateInfo)//from ServerObject.cs -> Listen() -> :44
        {
            try
            {
                Stream = client.GetStream();
                /*
                //string message = GetMessage();
                var reader = new StreamReader(Stream);
                Char[] data = new Char[64]; // буфер для получаемых данных
                StringBuilder builder = new StringBuilder();
                do// collect all strings from the Stream
                {
                    await reader.ReadAsync(data, 0, data.Length);// чтение из потока данных
                    builder.Append(data);
                }
                while (Stream.DataAvailable);
                string message = builder.ToString();// put all strings to message
                */

                // получаем имя пользователя
                byte[] data = new byte[64]; // буфер для получаемых данных
                StringBuilder builder = new StringBuilder();
                int bytes = 0;
                do// collect all strings from the Stream
                {
                    bytes = await Stream.ReadAsync(data, 0, data.Length);
                    builder.Append(Encoding.Unicode.GetString(data, 0, bytes));
                }
                while (Stream.DataAvailable);
                string message = builder.ToString();// put all strings to message
                
                Sender = message;

                message = Sender + " joined chat";
                // посылаем сообщение о входе в чат всем подключенным пользователям
                server.BroadcastMessage(message, this.Id);
                Console.OutputEncoding = Encoding.UTF8;
                await Console.Out.WriteLineAsync(message);
                // в бесконечном цикле получаем сообщения от клиента
                while (true)
                {
                    try
                    {
                        //message = GetMessage();
                        /*
                        var reader2 = new StreamReader(Stream);
                        Char[] data2 = new Char[64]; // буфер для получаемых данных
                        StringBuilder builder2 = new StringBuilder();
                        do// collect all strings from the Stream
                        {
                            await reader2.ReadAsync(data2, 0, data2.Length);// чтение из потока данных
                            builder2.Append(data2);
                        }
                        while (Stream.DataAvailable);
                        message = builder2.ToString();// put all strings to message
                        */

                        // получаем имя пользователя
                        byte[] data2 = new byte[64]; // буфер для получаемых данных
                        StringBuilder builder2 = new StringBuilder();
                        int bytes2 = 0;
                        do// collect all strings from the Stream
                        {
                            bytes2 = await Stream.ReadAsync(data2, 0, data2.Length);
                            builder2.Append(Encoding.Unicode.GetString(data2, 0, bytes2));
                        }
                        while (Stream.DataAvailable);
                        message = builder2.ToString();// put all strings to message
                        
                        message = String.Format("{0}: {1}", Sender, message);
                        await Console.Out.WriteLineAsync(message);
                        server.BroadcastMessage(message, this.Id);
                    }
                    catch
                    {
                        message = String.Format("{0}: left chat", Sender);
                        await Console.Out.WriteLineAsync(message);
                        server.BroadcastMessage(message, this.Id);
                        break;
                    }
                }
            }
            catch (Exception e)
            {
                await Console.Out.WriteLineAsync(e.Message);
            }
            finally
            {
                // в случае выхода из цикла закрываем ресурсы
                server.RemoveConnection(this.Id);
                Close();
            }
        }// process

        // чтение входящего сообщения и преобразование в строку
        private string GetMessage()
        {
            /*
             * Либо так, либо как в ConsoleClient : Program.cs
             * bytes = await stream.ReadAsync
             * и сама функция async
             * */
            byte[] data = new byte[64]; // буфер для получаемых данных
            StringBuilder builder = new StringBuilder();
            int bytes = 0;
            do
            {
                bytes = Stream.Read(data, 0, data.Length);// чтение из потока данных
                builder.Append(Encoding.Unicode.GetString(data, 0, bytes));
            }
            while (Stream.DataAvailable);

            //passStringViaAync = builder.ToString();
            return builder.ToString();
        }

        // закрытие подключения
        protected internal void Close()
        {
            if (Stream != null)
                Stream.Close();
            if (client != null)
                client.Close();
        }
    }// ClientObject
}// namespace
 /*
  * async tcp: http://sunildube.blogspot.ru/2011/12/asynchronous-tcp-client-easy-example.html
  * readlineASYNC : https://bsmadhu.wordpress.com/2012/09/29/simplify-asynchronous-programming-with-c-5-asyncawait/
  * big github tcp server/client project : https://github.com/Luaancz/Networking/blob/master/Networking%20Part%202/Luaan.Networking2.Server/ServerBase.cs
  * long stackoverflow answer : https://codereview.stackexchange.com/questions/82806/asynctcpclient-asynchronous-tcpclient
  * StreamReader MSDN : https://msdn.microsoft.com/en-us/library/system.io.streamreader(v=vs.110).aspx
  * dunno CodeProject : https://www.codeproject.com/Questions/243678/Csharp-Asynchronous-read-and-write-with-NetworkStr
  * 
  * 
  * 
  * */
