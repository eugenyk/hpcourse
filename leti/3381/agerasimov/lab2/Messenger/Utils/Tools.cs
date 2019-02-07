using System;
using System.IO;
using System.Windows.Forms;

using Messenger.Results;

using System.Net;
using System.Net.Sockets;
using System.Linq;

namespace Messenger.Utils
{
    public class Tools
    {
        private const string NC_FILENAME = "network_config.txt";
        public static void GetNetworkConfig(out string ip, out int port)
        {
            try
            {
                StreamReader sr = new StreamReader(NC_FILENAME);

                string ip_string = sr.ReadLine();
                string port_string = sr.ReadLine();

                sr.Close();

                ip = ip_string.Split('=')[1];
                port = Convert.ToInt32(port_string.Split('=')[1]);

                sr.Close();
            }
            catch (FileNotFoundException ex)
            {
                ip = ""; port = -1;
                MessageBox.Show(ErrorMessages.ERR_CONFIG_NOT_FOUND, ErrorMessages.ERR_ERROR,
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        public static string GetIPFromSocket(Socket nw_obj)
        {
            IPEndPoint client_ip = (IPEndPoint)nw_obj.LocalEndPoint;
            return client_ip.Address.ToString() + ":" + client_ip.Port.ToString();
        }

        public static IPAddress GetMyIP()
        {
            //исправить косяк, когда нет ip
            return Dns.GetHostEntry(Dns.GetHostName())
                   .AddressList
                   .First(address => address.AddressFamily == AddressFamily.InterNetwork);
        }
    }
}
