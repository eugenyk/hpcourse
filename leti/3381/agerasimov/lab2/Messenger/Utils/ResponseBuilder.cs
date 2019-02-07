using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;

namespace Messenger.Utils
{
    public static class ResponseBuilder
    {
        public static string Build(string type, string result)
        {          
            return type + " " + result;
        }

        public static string Parse(string resp)
        {
            string[] cons = resp.Split(' ');
            return cons[1];
        }
    }
}
