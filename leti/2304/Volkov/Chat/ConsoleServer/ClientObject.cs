using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;

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

        public void Process(Object stateInfo)//from ServerObject.cs -> Listen() -> :44
        {
            try
            {
                Stream = client.GetStream();
                // получаем имя пользователя
                string message = GetMessage();
                Sender = message;

                message = Sender + " joined chat";
                // посылаем сообщение о входе в чат всем подключенным пользователям
                server.BroadcastMessage(message, this.Id);
                Console.OutputEncoding = Encoding.UTF8;
                Console.Out.WriteLineAsync(message);
                // в бесконечном цикле получаем сообщения от клиента
                while (true)
                {
                    try
                    {
                        message = GetMessage();
                        message = String.Format("{0}: {1}", Sender, message);
                        Console.Out.WriteLineAsync(message);
                        server.BroadcastMessage(message, this.Id);
                    }
                    catch
                    {
                        message = String.Format("{0}: left chat", Sender);
                        Console.Out.WriteLineAsync(message);
                        server.BroadcastMessage(message, this.Id);
                        break;
                    }
                }
            }
            catch (Exception e)
            {
                Console.Out.WriteLineAsync(e.Message);
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
            byte[] data = new byte[64]; // буфер для получаемых данных
            StringBuilder builder = new StringBuilder();
            int bytes = 0;
            do
            {
                bytes = Stream.Read(data, 0, data.Length);// чтение из потока данных
                builder.Append(Encoding.Unicode.GetString(data, 0, bytes));
            }
            while (Stream.DataAvailable);

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
