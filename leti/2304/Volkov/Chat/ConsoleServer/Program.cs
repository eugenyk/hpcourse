using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace ConsoleServer
{
    class Program
    {
        static ServerObject server; // сервер
        static Thread listenThread; // поток для прослушивания
        static void Main(string[] args)
        {
            try
            {
                server = new ServerObject();
                listenThread = new Thread(new ThreadStart(server.Listen));
                listenThread.Start(); //старт потока
            }
            catch (Exception ex)
            {
                server.Disconnect();
                Console.OutputEncoding = Encoding.UTF8;
                Console.Out.WriteLineAsync(ex.Message);
            }
        }// Main
    }// Program
}
