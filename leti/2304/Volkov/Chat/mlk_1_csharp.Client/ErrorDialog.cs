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
    public partial class ErrorDialog : Form
    {
        public string ErrorMessage
        {
            get { return textBoxErrorMessage.Text; }
            set { textBoxErrorMessage.Text = value; }
        }

        public ErrorDialog()
        {
            InitializeComponent();
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
        }
    }
}
