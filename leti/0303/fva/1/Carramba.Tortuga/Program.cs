using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Carramba.Tortuga
{
    class Program
    {
        static void Main(string[] args)
        {
            new Server().Start().Wait();
        }


    }
}
