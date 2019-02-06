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

        public AutoResetEvent connectDone = new AutoResetEvent(false);
        public AutoResetEvent sendDone = new AutoResetEvent(false);
        public AutoResetEvent receiveDone = new AutoResetEvent(false);
        public AutoResetEvent aceptDone = new AutoResetEvent(false);
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

        public void Connect(IPEndPoint remoteEP, Socket client)        {
            client.BeginConnect(remoteEP, ConnectCallback, client);
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

        public void Accept(Socket client)
        {
            client.BeginAccept(AcceptCallback, client);
            connectDone.WaitOne();
        }

        private void AcceptCallback(IAsyncResult ar)
        {
            try{
                var socket = (Socket)ar.AsyncState;
                var socket1 = socket.EndAccept(ar);
                Program.AddConnection(socket1);
                aceptDone.Set();
            }
            catch (Exception e){
                Console.WriteLine(e.ToString());
            }

        }

        public void Send(Socket soket, Message protomsg){
            var message = protomsg.ToByteArray();
            var size = BitConverter.GetBytes(protomsg.CalculateSize());
            var bufferToSend = new byte[size.Length + message.Length];
            Buffer.BlockCopy(size, 0, bufferToSend, 0, size.Length);            
            Buffer.BlockCopy(message, 0, bufferToSend, size.Length, message.Length);
            soket.BeginSend(bufferToSend, 0, bufferToSend.Length, SocketFlags.None, SendCallback, soket);
            
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
                client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
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
                if (!state.sizeIsKnown){
                    if ((state.sizeOfMessageSizeBuffer + bytesRead) > 3){
                        Buffer.BlockCopy(state.buffer, 0, state.messageSizeBuffer, state.sizeOfMessageSizeBuffer, 4 - state.sizeOfMessageSizeBuffer);
                        state.messageSize = BitConverter.ToInt32(state.messageSizeBuffer, 0);
                        var amountOfMainBytes = bytesRead + state.sizeOfMessageSizeBuffer - 4;
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
                        
                    }
                    else{
                        //(state.sizeOfMessageSizeBuffer + bytesRead) < 3
                        Buffer.BlockCopy(state.buffer, 0, state.messageSizeBuffer, state.sizeOfMessageSizeBuffer, bytesRead);
                        state.sizeOfMessageSizeBuffer += bytesRead;
                        client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
                    }
                }
                else if (state.DataSize < state.messageSize)
                {
                    Buffer.BlockCopy(state.buffer, 0, state.mainBuffer, state.DataSize, bytesRead);
                    state.DataSize += bytesRead;
                    if (state.DataSize == state.messageSize){
                        LockFreeQueue.Push(Message.Parser.ParseFrom(state.mainBuffer, 0, state.DataSize));
                        receiveDone.Set();
                    }
                    else{
                        client.BeginReceive(state.buffer, 0, StateObject.BufferSize, 0, ReceiveCallback, state);
                    }
                }
                else {
                    LockFreeQueue.Push(Message.Parser.ParseFrom(state.mainBuffer, 0, state.DataSize));
                    receiveDone.Set();
                }  
            }
            catch (Exception e){
                Console.WriteLine(e.ToString());
            }
        }

    }
}
