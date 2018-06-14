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

public class Server {
  private static int port, cThreads;
  private static string hostName;
  private static List<List<Socket>> connections;
  private static bool _exit;

  // signals
  public static AutoResetEvent allDone = new AutoResetEvent(false);
  public static AutoResetEvent sendDone = new AutoResetEvent(false);
  public AutoResetEvent receiveDone = new AutoResetEvent(false);
  public Message message;

  public class StateObject{             
    public Socket workSocket = null;
    public bool sizeIsKnown=false;
    public int messageSize;
    public const int BufferSize = 256;
    public byte[] buffer = new byte[BufferSize];
    public byte[] mainBuffer = new byte[1024];
    public byte[] messageSizeBuffer = new byte[4];
    public int DataSize = 0;
    public int sizeOfMessageSizeBuffer = 0;
  }

  public Server() {
    IPHostEntry host = Dns.GetHostEntry(hostName);
    IPAddress ip = host.AddressList[0];
    IPEndPoint endPoint = new IPEndPoint(ip, port);
    Socket server = new Socket(ip.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

    try {
      server.Bind(endPoint);
      server.Listen(10);

      while (true) {
        Console.WriteLine("Waiting for a connection..."); 
        server.BeginAccept(new AsyncCallback(AcceptCallback), server);  
        allDone.WaitOne();
      }
    }
    catch (Exception ex) {
      Console.Out.WriteLineAsync($"Server error: {ex.ToString()}");
    }
    finally {
      Console.ReadLine();
    }
  }

  public static void AcceptCallback(IAsyncResult cbSocket) {
    var socket = (Socket)cbSocket.AsyncState;
    var tmpSocket = socket.EndAccept(cbSocket);
    AddConnection(tmpSocket);
  }

  public static void AddConnection(Socket socket) {
    if (connections.Count < cThreads) {
      var socketList = new List<Socket> {socket};
      connections.Add(socketList);
      ThreadPool.QueueUserWorkItem(ClientHandler, socketList);
    } else {
      List<Socket> soсketList = connections[0];
        foreach (var list in connections){
          if (soсketList.Count > list.Count) soсketList = list;
        }
      soсketList.Add(socket);
    }

    allDone.Set();
  }

  public static void ClientHandler(Object sList) {
    var listOfSockets = (List<Socket>)sList;
    foreach (var client in listOfSockets){}
    while (!_exit){
      var testmsg = LockFreeQueue.Pop();
      if (testmsg != null){
        if (!MessageHandler(testmsg)){
          break;
        }
      }
    }
  }

  public static void Send(Socket socket, Message msg) {
    var arrMsg = msg.ToByteArray();
    var sizeMsg = BitConverter.GetBytes(msg.CalculateSize());
    var bufferToSend = new byte[sizeMsg.Length + msg.Length];

    Buffer.BlockCopy(sizeMsg, 0, bufferToSend, 0, sizeMsg.Length);            
    Buffer.BlockCopy(msg, 0, bufferToSend, sizeMsg.Length, msg.Length);
    soket.BeginSend(bufferToSend, 0, bufferToSend.Length, SocketFlags.None, SendCallback, socket);
  }

  public static void SendCallback(IAsyncResult cbSocket) {
    try{
      Socket soket = (Socket)cbSocket.AsyncState;
      int bytesSent = soket.EndSend(cbSocket);
      sendDone.Set();
    }
    catch (Exception e){
      Console.WriteLine(e.ToString());
    }
  }

  public static void Receive(Socket socket) {
    try {
      StateObject state = new StateObject();
      state.workSocket = socket; 
      socket.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
    }
    catch (Exception e) {
      Console.WriteLine(e.ToString());
    }
  }

  public static void ReceiveCallback(IAsyncResult cbState) {
    try {
      StateObject state = (StateObject)cbState.AsyncState;
      Socket sender = state.workSocket;                
      int bytesRead = sender.EndReceive(cbState);

      if (!state.sizeIsKnown) {
        if ((state.sizeOfMessageSizeBuffer + bytesRead) > 3) {
          var amountOfMainBytes = bytesRead + state.sizeOfMessageSizeBuffer - 4;

          Buffer.BlockCopy(state.buffer, 0, state.messageSizeBuffer, state.sizeOfMessageSizeBuffer, 4 - state.sizeOfMessageSizeBuffer);
          state.messageSize = BitConverter.ToInt32(state.messageSizeBuffer, 0);
          Buffer.BlockCopy(state.buffer, bytesRead - amountOfMainBytes, state.mainBuffer, state.DataSize, amountOfMainBytes);
          state.DataSize += amountOfMainBytes;
          state.sizeIsKnown = true;
          if (state.DataSize == state.messageSize){
            LockFreeQueue.Push(Message.Parser.ParseFrom(state.mainBuffer, 0, state.DataSize));
            receiveDone.Set();
          }
          else {
            client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
          }
        } else {
          Buffer.BlockCopy(state.buffer, 0, state.messageSizeBuffer, state.sizeOfMessageSizeBuffer, bytesRead);
          state.sizeOfMessageSizeBuffer += bytesRead;
          client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
        }
      } else if (state.DataSize < state.messageSize) {
        Buffer.BlockCopy(state.buffer, 0, state.mainBuffer, state.DataSize, bytesRead);
        state.DataSize += bytesRead;
        if (state.DataSize == state.messageSize){
          LockFreeQueue.Push(Message.Parser.ParseFrom(state.mainBuffer, 0, state.DataSize));
          receiveDone.Set();
        }
        else{
          client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
        }
      } else {
        LockFreeQueue.Push(Message.Parser.ParseFrom(state.mainBuffer, 0, state.DataSize));
        receiveDone.Set();
      }
    }
    catch (Exception e) {
      Console.WriteLine(e.ToString());
    }
  }

  private static bool MessageHandler(Message message){
    string line;
    switch (message.Data){
      case "Join":
        line = message.Sender + " присоединился!";
        Console.Out.WriteLineAsync(line);
        Broadcast(line);
        return true;
      case "ServerClose":
        line = "Завершение работы сервера";
        Console.Out.WriteLineAsync(line);
        Broadcast(line);
        _exit = true;
        return true;
      default:
        line = message.Sender + ": " + message.Text;
        Console.Out.WriteLineAsync(line);
        Broadcast(line);
        return true;
    }
  }

  private static void Broadcast(string text){
    foreach (var list in connections){
      foreach (var client in list){
        var protomsg = new Message() { Data = "", Sender = "Server", Text = text };
        sendDone.Set();
        Send(client, protomsg);
      }
    }
  }

  static int Main(string[] args) {
    if (args.Length == 3) {
      hostName = args[0];
      port = int.Parse(args[1]);
      cThreads = int.Parse(args[2]);

      _exit = false;

      ThreadPool.SetMinThreads(cThreads, cThreads);
      ThreadPool.SetMaxThreads(cThreads, cThreads);

      connections = new List<List<Socket>>();
    

      if (port < 1000 || port > 65000) {
        Console.Out.WriteLineAsync("Error port. Port must be in [1000,65000]");
        return 1;
      }

      if (cThreads <= 0 || cThreads > 100) {
        Console.Out.WriteLineAsync("Error count threads. Number of threads must be in [1,100]");
        return 1;
      }

      new Server();

      return 0;
    } else {
      Console.Out.WriteLineAsync("Can't set required parameters");
      return 1;
    }
  }
}