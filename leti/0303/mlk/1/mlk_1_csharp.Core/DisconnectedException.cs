using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DevoidTalk.Core
{
    [Serializable]
    public class DisconnectedException : Exception
    {
        public DisconnectedException() { }
        public DisconnectedException(string message) : base(message) { }
        public DisconnectedException(string message, Exception inner) : base(message, inner) { }
        protected DisconnectedException(
          System.Runtime.Serialization.SerializationInfo info,
          System.Runtime.Serialization.StreamingContext context) : base(info, context)
        { }
    }
}
