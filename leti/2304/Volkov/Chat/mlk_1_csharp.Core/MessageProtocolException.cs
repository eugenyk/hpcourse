using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevoidTalk.Core
{
    [Serializable]
    public class MessageProtocolException : Exception
    {
        public MessageProtocolException() { }
        public MessageProtocolException(string message) : base(message) { }
        public MessageProtocolException(string message, Exception inner) : base(message, inner) { }
        protected MessageProtocolException(
          System.Runtime.Serialization.SerializationInfo info,
          System.Runtime.Serialization.StreamingContext context) : base(info, context)
        { }
    }
}
