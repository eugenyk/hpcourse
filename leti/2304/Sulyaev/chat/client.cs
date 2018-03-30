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

public class Client {
  private static int port;
  private static string hostName;
  private static bool exit;

  public AutoResetEvent connectDone = new AutoResetEvent(false);

  public Client() {
    IPHostEntry host = Dns.GetHostEntry(hostName);
    IPAddress ip = host.AddressList[0];
    IPEndPoint remoteEndPoint = new IPEndPoint(ip, port);
    Socket client = new Socket(ip.AddressFamily, SocketType.Stream, ProtocolType.Tcp); 

    try {
      client.BeginConnect( remoteEndPoint, new AsyncCallback(ConnectCallback), client);  
      connectDone.WaitOne(); 

      protomsg = new Message() { Data = "Join", Sender = "Human", Text = "" };

      Thread receiveThread = new Thread(getServerMsg);
      receiveThread.Start();

      Server.Send(client, protomsg);

      while (true)
      {
        var textMsg = Console.ReadLine();
        protomsg = CreateMsg(textMsg);
        Server.Send(client, protomsg);             
        Server.sendDone.WaitOne();
      }

      client.Shutdown(SocketShutdown.Both);
      client.Close();
    }
    catch (Exception ex) {
      Console.Out.WriteLineAsync($"Client error: {ex.ToString()}");
    }
  }

  public static void ConnectCallback(IAsyncResult cbSocket) {
    try{
      Socket clientSocket = (Socket)ar.AsyncState;
      clientSocket.EndConnect(cbSocket);
      Console.WriteLine($"Connecting: {clientSocket.RemoteEndPoint}");
      Server.connectDone.Set();
    }
    catch (Exception e){
      Console.WriteLine(e.ToString());
    }
  }

  public static void getServerMsg() {
    while (!exit) {
      try {
        Server.Receive(client);
        Server.receiveDone.WaitOne();
        if (Server.message == null) continue;
        var testmsg = Server.message;
        if (testmsg.Text == "Завершение работы сервера") exit = true;
        Console.Out.WriteLineAsync(testmsg.Text);
        Server.message = null;                    
      }
      catch (Exception ex)
      {
        Console.Out.WriteLineAsync("exeption test");
        Console.Out.WriteLineAsync(ex.ToString());
      }
    }
  }

  public static Message CreateMsg(string text) {
    string pText;            
    string pData;
    switch (text){
      case "Join":
        pData = "Join";
        pText = "";
        break;
      default:
        pData = "Message";
        pText = text;
        break;
    }
    return new Message() { Data = pData, Sender = "Human", Text = pText };
  }

  static int Main(string[] args) {
    if (args.Length == 2) {
      hostName = args[0];
      port = int.Parse(args[1]);

      if (port < 1000 || port > 65000) {
        Console.Out.WriteLineAsync("Error port. Port must be in [1000,65000]");
        return 1;
      }

      exit = false;

      new Client();
      return 0;    
    } else {
      Console.Out.WriteLineAsync("Can't set required parameters"); 
      return 1;
    }
  }
}
