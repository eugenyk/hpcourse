using System;
using System.IO;
using System.Net;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ClientConfigurator
{
    public partial class MainForm : Form
    {
        private const string NC_FILENAME = "network_config.txt";

        public MainForm()
        {
            InitializeComponent();
            ParseConfig();
        }

        private void OK_Button_Click(object sender, EventArgs e)
        {
            if (CheckIP(IPBox.Text) && CheckPort(PortBox.Text))
            {
                StreamWriter sw = new StreamWriter(NC_FILENAME, false);

                sw.WriteLine("IP=" + IPBox.Text);
                sw.WriteLine("Port=" + PortBox.Text);

                sw.Close();

                MessageBox.Show("Настройки успешно применены!", "Информация", MessageBoxButtons.OK, MessageBoxIcon.Information);
                Application.Exit();
            }
        }

        private bool CheckIP(string s)
        {
            IPAddress tmp_addr;
            if (!IPAddress.TryParse(s, out tmp_addr))
            {
                MessageBox.Show("Некорректный IP-адрес!", "Предупреждение", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
                return false;
            }
            return true;
        }
        private bool CheckPort(string s)
        {
            int tmp_port;
            if (!(Int32.TryParse(s, out tmp_port) && tmp_port < 65536))
            {
                MessageBox.Show("Некорректный номер порта!", "Предупреждение", MessageBoxButtons.OK, MessageBoxIcon.Asterisk);
                return false;
            }
            return true;
        }

        private void ParseConfig()
        {
            if (File.Exists(NC_FILENAME))
            {
                StreamReader sr = new StreamReader(NC_FILENAME);

                string ip_string = sr.ReadLine();
                string port_string = sr.ReadLine();

                sr.Close();

                string ip = ip_string.Split('=')[1];
                string port = port_string.Split('=')[1];

                IPBox.Text = ip;
                PortBox.Text = port;
            }
            else
                Console.Beep();
        }
    }
}
