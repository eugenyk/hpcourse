using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace DevoidTalk.Core
{
    public class Connection
    {
        const int MaxMessageSize = 1024 * 1024; // 1MB
        const int MinBytesToReceive = 1024;

        readonly Socket socket;
        readonly AsyncLock sendingLock = new AsyncLock();

        byte[] receiveBuffer = new byte[4 * 1024]; // default is 4KB

        public Connection(Socket socket)
        {
            this.socket = socket;
        }

        public async Task<Message> ReadMessage()
        {
            await ReadBytes(4);

            int payloadLength = ReadInt32BigEndian(receiveBuffer, 0);
            if (payloadLength > MaxMessageSize)
            {
                throw new MessageProtocolException(
                    $"Message is too big ({payloadLength} bytes).");
            }
            if (payloadLength > receiveBuffer.Length)
            {
                receiveBuffer = new byte[payloadLength];
            }

            await ReadBytes(payloadLength);

            using (var stream = new MemoryStream(receiveBuffer, 0, payloadLength))
            {
                return ProtoBuf.Serializer.Deserialize<Message>(stream);
            }
        }

        public async Task SendMessage(Message message)
        {
            using (var releaser = await sendingLock.LockAsync())
            {
                await SendMessageWithoutInterleaving(message);
            }
        }

        private async Task SendMessageWithoutInterleaving(Message message)
        {
            using (var sendStream = new MemoryStream())
            {
                sendStream.SetLength(4);
                sendStream.Position = 4;
                ProtoBuf.Serializer.Serialize(sendStream, message);

                int messageLength = checked((int)sendStream.Position);
                int payloadLength = messageLength - 4;

                byte[] buffer = sendStream.GetBuffer();
                WriteInt32BigEndian(payloadLength, sendStream.GetBuffer(), 0);

                int bytesWritten = 0;
                while (bytesWritten < messageLength)
                {
                    int written = await socket.SendTaskAsync(
                        buffer, bytesWritten, messageLength - bytesWritten, SocketFlags.None);
                    bytesWritten += written;
                }
            }
        }

        public async Task Disconnect(bool ignoreExceptions = true)
        {
            try
            {
                await socket.DisconnectTaskAsync(true);
            }
            catch (Exception)
            {
                if (!ignoreExceptions) { throw; }
            }
        }

        private async Task ReadBytes(int count)
        {
            int bytesRead = 0;
            while (bytesRead < count)
            {
                int readBytes = await socket.ReceiveTaskAsync(
                    receiveBuffer, bytesRead, count - bytesRead, SocketFlags.None);
                if (readBytes == 0) { throw new DisconnectedException(); }
                bytesRead += readBytes;
            }
        }

        private static int ReadInt32BigEndian(byte[] buffer, int offset)
        {
            return
                (buffer[offset + 0] << 24) |
                (buffer[offset + 1] << 16) |
                (buffer[offset + 2] << 8) |
                (buffer[offset + 3] << 0);
        }

        private static void WriteInt32BigEndian(int value, byte[] buffer, int offset)
        {
            buffer[offset + 0] = (byte)((value >> 24) & 0xFF);
            buffer[offset + 1] = (byte)((value >> 16) & 0xFF);
            buffer[offset + 2] = (byte)((value >> 8) & 0xFF);
            buffer[offset + 3] = (byte)(value & 0xFF);
        }
    }
}
