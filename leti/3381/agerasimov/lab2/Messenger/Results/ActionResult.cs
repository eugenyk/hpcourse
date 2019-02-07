using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Messenger.Results
{
    public class ActionResult
    {
        private bool act_result;
        public bool Result { get { return act_result; } }

        private string data;
        public string Data { get { return data; } }

        public ActionResult(bool res, string message)
        {
            act_result = res;
            data = message;
        }
    }
}
