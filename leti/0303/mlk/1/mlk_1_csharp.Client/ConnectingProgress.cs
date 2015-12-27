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
    public partial class ConnectingProgress : Form
    {
        readonly Task connectingTask;

        public ConnectingProgress(Task connectingTask)
        {
            this.connectingTask = connectingTask;
            InitializeComponent();
        }

        private void ConnectingProgress_Load(object sender, EventArgs e)
        {
            connectingTask.ContinueWith(task =>
            {
                if (DialogResult == DialogResult.None)
                {
                    DialogResult = DialogResult.OK;
                }
            });
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.Abort;
        }
    }
}
