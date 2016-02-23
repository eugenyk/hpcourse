using ProtoBuf;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Carramba.Codex
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
