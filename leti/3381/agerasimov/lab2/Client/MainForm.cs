using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;

using Messenger.Results;
using Messenger.Utils;

namespace Client
{
    public partial class MainForm : Form
    {
        private Client client;
        private string user_name;


        public MainForm(Client _client, string _username)
        {
            InitializeComponent();
            client = _client;

            user_name = _username;

            LoadContactList();

            Text = "Вход выполнен - " + user_name;
        }


        private void AddUserButton_Click(object sender, EventArgs e)
        {
            UsersListForm users_list = new UsersListForm(client.GetUsersList().Data.Split(' ')
                .Where(u => !UserList.Items.Contains(u)).ToArray());

            users_list.ShowDialog();

            if (users_list.SelectedUser != null)
            {


                ActionResult res = client.AddUserToContacts(users_list.SelectedUser);
                if (res.Result)
                    UserList.Items.Add(users_list.SelectedUser);
                else
                    MessageBox.Show(ErrorMessages.ERR_UNDEFINED_RESPONSE, ErrorMessages.ERR_ERROR,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void LoadContactList()
        {
            ActionResult cont = client.GetContacts(user_name);
            if (cont.Result)
            {
                if (cont.Data != ResponseBuilder.Build(QueryConsts.RT_GET_CONTACTS, QueryConsts.RT_EMPTY_CONTACTS))
                {
                    string[] users = cont.Data.Split(' ');
                    for (int i = 1; i < users.Length; i++)
                    {
                        UserList.Items.Add(users[i]);
                    }
                }
            }
            else
                MessageBox.Show(ErrorMessages.ERR_UNDEFINED_RESPONSE, ErrorMessages.ERR_ERROR,
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private void DeleteUserButton_Click(object sender, EventArgs e)
        {
            if (UserList.SelectedIndex >= 0)
            {
                ActionResult res = client.RemoveUserFromContacts((string)UserList.SelectedItem);
                if (res.Result)
                    UserList.Items.RemoveAt(UserList.SelectedIndex);
                else
                    MessageBox.Show(ErrorMessages.ERR_UNDEFINED_RESPONSE, ErrorMessages.ERR_ERROR,
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else                 
                MessageBox.Show("Контакт не выбран!", "Ошибка", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
        }

        public void AddMessage(string sender, string recipient, string message)
        {
            try
            {
                MessagesBox.Invoke(new Action(() =>
                {
                    AppendTextExt(MessagesBox, sender + " -> " + recipient + " : ", Color.Red);
                    MessagesBox.AppendText("\r\n");
                    MessagesBox.AppendText(message);
                    MessagesBox.AppendText("\r\n");
                }));
            }
            catch(Exception ex) { }

        }

        private void SendButton_Click(object sender, EventArgs e)
        {
            if(UserList.SelectedIndex >= 0 && MessageTextBox.Text != "")
            {
                Thread waiting_thread = new Thread(new ThreadStart(new Action(() =>
                {
                    WaitingForm form = new WaitingForm("Отправка...");
                    form.ShowDialog();
                })));
                waiting_thread.Start();

                ActionResult result = client.SendMessage(user_name, UserList.SelectedItem.ToString(), MessageTextBox.Text);

                try { waiting_thread.Abort(); }
                catch (Exception ex) { }

                if (!result.Result)
                    MessageBox.Show(result.Data, ErrorMessages.ERR_ERROR,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                else
                {
                    AddMessage(user_name, UserList.SelectedItem.ToString(), MessageTextBox.Text);
                    MessageTextBox.Clear();
                }
            }
            else
            {
                MessageBox.Show("Вы не выбрали получателя либо не ввели текст сообщения!", "Предупреждение",
                    MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        public void AppendTextExt(RichTextBox box, string text, Color color)
        {
            box.SelectionStart = box.TextLength;
            box.SelectionLength = 0;
            box.SelectionColor = color;
            box.AppendText(text);
            box.SelectionColor = box.ForeColor;
            box.Select(0, 0);

        }

        private void ExitButton_Click(object sender, EventArgs e)
        {
            ActionResult res = client.Logout(user_name);

            client.CloseConnection();

            try
            {
                Application.Exit();
            }
            catch (Exception ex)
            {
                client.listen_thread.Abort();
                Application.Exit();
            }

        }
    }
}
