using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Net;
using System.Threading;

namespace ConsoleServer
{
    public class ServerObject
    {
        static TcpListener tcpListener; // сервер для прослушивания
        List<ClientObject> clients = new List<ClientObject>(); // все подключения
        public static ManualResetEvent tcpClientConnected = new ManualResetEvent(false);// Thread signal.

        protected internal void AddConnection(ClientObject clientObject)
        {
            clients.Add(clientObject);
        }
        protected internal void RemoveConnection(string id)
        {
            // получаем по id закрытое подключение
            ClientObject client = clients.FirstOrDefault(c => c.Id == id);
            // и удаляем его из списка подключений
            if (client != null)
                clients.Remove(client);
        }
        // прослушивание входящих подключений
        protected internal void Listen(object threadsByUser = null)// from Program.cs
        {
            try
            {
                tcpListener = new TcpListener(IPAddress.Any, 8888);
                tcpListener.Start();
                Console.OutputEncoding = Encoding.UTF8;
                Console.Out.WriteLineAsync("Server started. Waiting for connections...");
                
                if (threadsByUser != null)// if we ran Console with arguments
                {
                    int threadsByUserInt = Convert.ToInt32(threadsByUser);
                    //Console.Out.WriteLineAsync(threadsByUserInt.ToString());// debug Console with arguments
                    ThreadPool.SetMaxThreads(threadsByUserInt, threadsByUserInt);
                    ThreadPool.SetMinThreads(2, 2);// By default, the minimum number of threads is set to the number of processors on a system.You can use the
                }// if


                while (true)
                {
                    /*
                     * http://forundex.ru/yap/Asinhronnij-TCP-server-klient-ne-vihodit-schitatj-potok-578193
                     * https://msdn.microsoft.com/ru-ru/library/system.net.sockets.tcplistener.beginaccepttcpclient(v=vs.110).aspx
                     * 
                     * */
                    try
                    {
                        tcpClientConnected.Reset();
                        tcpListener.BeginAcceptTcpClient(new AsyncCallback(DoAcceptTcpClientCallback), tcpListener);
                        tcpClientConnected.WaitOne();
                    }
                    catch (Exception e)
                    {
                        Console.Out.WriteLineAsync(e.Message);
                    }
                }// while true
                
                //while (true)
                //{// получаем входящее подключение
                    /*try
                    {
                        var tcpClient = await tcpListener.AcceptTcpClientAsync();
                    }
                    catch (SocketException e)
                    {
                        Console.WriteLine(e.Message);
                    }
                    */
                    /*ClientObject clientObject = new ClientObject(tcpClient, this);                    
                    
                    ThreadPool.QueueUserWorkItem(new WaitCallback(clientObject.Process));// пул потоков
                    */
                    //Thread clientThread = new Thread(new ThreadStart(clientObject.Process));
                    //clientThread.Start(); commented 'cause we have ThreadPool
                    //}
                }
            catch (Exception ex)
            {
                Console.Out.WriteLineAsync(ex.Message);
                Disconnect();
            }
        }

        protected internal void DoAcceptTcpClientCallback(IAsyncResult ar)
        {
            // Get the listener that handles the client request.
            TcpListener listener = (TcpListener)ar.AsyncState;

            // End the operation and display the received data on 
            // the console.
            TcpClient tcpClient = listener.EndAcceptTcpClient(ar);

            ClientObject clientObject = new ClientObject(tcpClient, this);
            ThreadPool.QueueUserWorkItem(new WaitCallback(clientObject.Process));// пул потоков

            // Signal the calling thread to continue.
            tcpClientConnected.Set();

        }

        // трансляция сообщения подключенным клиентам
        protected internal void BroadcastMessage(string message, string id, string Data = "")
        {
            byte[] data = Encoding.Unicode.GetBytes(message);
            for (int i = 0; i < clients.Count; i++)
            {
                if (clients[i].Id != id) // если id клиента не равно id отправляющего
                {
                    clients[i].Stream.WriteAsync(data, 0, data.Length); //передача данных
                }
            }
        }
        // отключение всех клиентов
        protected internal void Disconnect()
        {
            tcpListener.Stop(); //остановка сервера

            for (int i = 0; i < clients.Count; i++)
            {
                clients[i].Close(); //отключение клиента
            }
            Environment.Exit(0); //завершение процесса
        }
    }// ServerObject
}// ConsoleServer
