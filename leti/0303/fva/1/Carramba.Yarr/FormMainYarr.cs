using Carramba.Codex;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Carramba.Yarr
{
    public partial class FormMainYarr : Form
    {
        Connection connect;
        public FormMainYarr()
        {
            InitializeComponent();
        }

        private async void FormMainYarr_Shown(object sender, EventArgs e)
        {
            TcpClient client = new TcpClient();
            await client.ConnectAsync("localhost", 10000);
            connect = new Connection(client);

            while (true)
            {
                Codex.Message message = await connect.ReadMessage();
                textBoxICanSee.AppendText(string.Format("{0}:[{1}]: {2}\r\n", DateTime.Now, message.Sender, message.Text));
                textBoxICanSee.Select(textBoxICanSee.TextLength, 0);
                textBoxICanSee.ScrollToCaret();
            }
        }

        private async void buttonSend_Click(object sender, EventArgs e)
        {
            await Send();
        }

        private async Task Send()
        {
            Codex.Message message = new Codex.Message();
            message.Sender = usernametext.Text;
            message.Text = textBoxICanTalk.Text;
            buttonSend.Enabled = false;
            await connect.SendMessage(message);
            textBoxICanTalk.Clear();
            buttonSend.Enabled = true;
        }

        private async void textBoxICanTalk_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
            {
                await Send();
            }
        }
    }
}
