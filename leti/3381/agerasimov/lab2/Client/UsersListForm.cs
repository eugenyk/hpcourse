using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Client
{
    public partial class UsersListForm : Form
    {
        public UsersListForm(string[] list)
        {
            InitializeComponent();
            for (int i = 1; i < list.Length; i++)
                UsersListBox.Items.Add(list[i]);
        }

        private string sel_user = null;
        public string SelectedUser { get { return sel_user; } set {sel_user = value; } }

        private void UsersListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedUser = (string)UsersListBox.SelectedItem;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            this.Close();
        }
    }
}
