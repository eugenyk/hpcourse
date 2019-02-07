using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;



namespace Server
{
    public partial class MainForm : Form
    {
        Thread server_thread = null;

        public MainForm()
        {
            InitializeComponent();
        }

        Server serv;
        private void StartServerButton_Click(object sender, EventArgs e)
        {
            server_thread = new Thread(new ThreadStart(new Action(() =>
            {
                serv = new Server(this);
                serv.Start();
            })));
            server_thread.Start();

            ServerStatusBar.Style = ProgressBarStyle.Marquee;
        }

        public void WriteLog(string log_str)
        {
            if (LogBox.InvokeRequired)
            {
                LogBox.Invoke(new Action(() =>
                {
                    LogBox.Text += (DateTime.Now.ToString() + " : " + log_str);
                    LogBox.AppendText("\r\n");
                }));
            }
        }

        private void StopServerButton_Click(object sender, EventArgs e)
        {
            serv.Stop();
        }
    }
}
