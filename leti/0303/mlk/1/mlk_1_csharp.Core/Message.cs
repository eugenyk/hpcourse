using System;
using ProtoBuf;

namespace DevoidTalk.Core
{
    [ProtoContract]
    public class Message
    {
        [ProtoMember(1)]
        public string Sender { get; set; }
        [ProtoMember(2)]
        public string Text { get; set; }
        [ProtoMember(3, IsRequired = false)]
        public string Data { get; set; }
    }
}
