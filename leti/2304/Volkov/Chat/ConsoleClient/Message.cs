using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using ProtoBuf;

namespace ConsoleClient
{

    [ProtoContract]
    class Message
    {
        [ProtoMember(1)]
        public string Sender { get; set; }///< Идентификатор отправителя
        [ProtoMember(2)]
        public string Text { get; set; }///< Содержание сообщения
        [ProtoMember(3)]
        public string Data { get; set; }
    }// class
}
