using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

namespace Carramba.Codex
{
    public class Connection
    {
        TcpClient client;
        Stream stream;
        byte[] receiveBuffer = new byte[4096];
        
        public Connection(TcpClient client)
        {
            this.client = client;
            this.stream = client.GetStream();
        }

        public async Task<Message> ReadMessage()
        {
            await ReadBytes(4);
            int messageLength = ReadInt32BigEndian(receiveBuffer, 0);
            if (messageLength > 4096)
            {
                throw new IOException("too big message");
            }

            await ReadBytes(messageLength);

            using (var stream = new MemoryStream(receiveBuffer, 0, messageLength))
            {
                return ProtoBuf.Serializer.Deserialize<Message>(stream);
            }
        }

        public async Task SendMessage(Message dove)
        {
            var ms = new MemoryStream();
            ms.SetLength(4);
            ms.Position = 4;
            ProtoBuf.Serializer.Serialize(ms, dove);
            int length = (int)ms.Length - 4;
            WriteInt32BigEndian(length, ms.GetBuffer(), 0);
            ms.Position = 0;
            await ms.CopyToAsync(stream);
        }

        private async Task ReadBytes(int count)
        {
            int bytesRead = 0;
            while (bytesRead < count)
            {
                int readBytes = await stream.ReadAsync(
                    receiveBuffer, bytesRead, count - bytesRead);
                if (readBytes == 0) { throw new IOException("disconnect"); }
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
