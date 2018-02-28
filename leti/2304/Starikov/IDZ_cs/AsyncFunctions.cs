using Google.Protobuf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Tutorial;

namespace IDZCs
{
    class AsyncFunctions{

        public ManualResetEvent connectDone = new ManualResetEvent(false);
        public ManualResetEvent sendDone = new ManualResetEvent(false);
        public ManualResetEvent receiveDone = new ManualResetEvent(false);
        public Message message;

        public class StateObject{             
            public Socket workSocket = null;
            public const int BufferSize = 256;
            public byte[] buffer = new byte[BufferSize];
            public byte[] mainBuffer = new byte[256];
            public int DataSize = 0;            
        }

        public void Connect(IPEndPoint remoteEP, Socket client)        {
            client.BeginConnect(remoteEP, new AsyncCallback(ConnectCallback), client);
            connectDone.WaitOne();
        }

        private void ConnectCallback(IAsyncResult ar){
            try{
                Socket client = (Socket)ar.AsyncState;
                client.EndConnect(ar);
                Console.WriteLine("Сокет соединяется с {0} ", client.RemoteEndPoint);
                connectDone.Set();
            }
            catch (Exception e){
                Console.WriteLine(e.ToString());
            }
        }

        public void Send(Socket soket, Message protomsg){
            var message = protomsg.ToByteArray();
            soket.BeginSend(message, 0, protomsg.CalculateSize(), SocketFlags.None, new AsyncCallback(SendCallback), soket);
        }

        private void SendCallback(IAsyncResult ar){
            try{
                Socket soket = (Socket)ar.AsyncState;
                int bytesSent = soket.EndSend(ar);
                sendDone.Set();
            }
            catch (Exception e){
                Console.WriteLine(e.ToString());
            }
        }

        public void Receive(Socket client){
            try{
                StateObject state = new StateObject();
                state.workSocket = client; 
                client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                    new AsyncCallback(ReceiveCallback), state);
            }
            catch (Exception e){
                Console.WriteLine(e.ToString());
            }
        }

        private void ReceiveCallback(IAsyncResult ar){
            try{
                StateObject state = (StateObject)ar.AsyncState;
                Socket client = state.workSocket;                
                int bytesRead = client.EndReceive(ar);
                if (bytesRead > 0){
                    //state.sb.Append(Encoding.ASCII.GetString(state.buffer, 0, bytesRead));
                    // bufer ++?
                    state.DataSize += bytesRead;                    
                    /*client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0,
                        new AsyncCallback(ReceiveCallback), state);*/
                    message = Message.Parser.ParseFrom(state.buffer, 0, state.DataSize);
                    receiveDone.Set();
                }
                else{                     
                    if (state.DataSize > 1){                        
                        message = Message.Parser.ParseFrom(state.buffer, 0, state.DataSize);
                    }                    
                    receiveDone.Set();
                }
            }
            catch (Exception e){
                Console.WriteLine(e.ToString());
            }
        }

    }
}
