using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace DevoidTalk.Client
{
    public partial class ConnectionDialog : Form
    {
        public string ServerAddress
        {
            get { return textBoxServerAddress.Text; }
            set { textBoxServerAddress.Text = value; }
        }

        public string ChatName
        {
            get { return textBoxChatName.Text; }
            set { textBoxChatName.Text = value; }
        }

        public ConnectionDialog()
        {
            InitializeComponent();
        }

        private void buttonConnect_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
        }

        private void buttonExit_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
        }
    }
}
