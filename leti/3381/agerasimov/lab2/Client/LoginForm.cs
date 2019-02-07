using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

using Messenger.Results;

namespace Client
{
    public partial class LoginForm : Form
    {
        private Client client = null;
        private Thread client_thread = null;

        public LoginForm()
        {
            InitializeComponent();

            InitListenThread();
        }

        public void InitListenThread()
        {
            client_thread = new Thread(new ThreadStart(new Action(() =>
            {
                client = new Client();
                client.StartListening();
            })));
            client_thread.Start();
        }

        private void RegisterButton_Click(object sender, EventArgs e)
        {
            if (!client.is_connected)
                InitListenThread();

            if (client.is_connected)
            {
                WaitingForm waiting = null;
                Thread wait_thread = new Thread(new ThreadStart(new Action(() =>
                {
                    waiting = new WaitingForm("Запрос к серверу...");
                    waiting.ShowDialog();
                })));

                wait_thread.Start();
                ActionResult res = client.RegisterUser(LoginBox.Text, PasswordBox.Text);
                try { wait_thread.Abort(); }
                catch (Exception ex) { }

                if (!res.Result)
                    MessageBox.Show(res.Data, ErrorMessages.ERR_ERROR,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                else
                    MessageBox.Show(res.Data, InfoMessages.INF_INFORMATION,
                        MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void LoginButton_Click(object sender, EventArgs e)
        {
            if (!client.is_connected)
                InitListenThread();

            Thread.Sleep(10);

            if (client.is_connected)
            {
                WaitingForm waiting = null;
                Thread wait_thread = new Thread(new ThreadStart(new Action(() =>
                {
                    waiting = new WaitingForm("Запрос к серверу...");
                    waiting.ShowDialog();
                })));

                wait_thread.Start();
                ActionResult res = client.LoginUser(LoginBox.Text, PasswordBox.Text);
                try { wait_thread.Abort(); }
                catch (Exception ex) { }

                if (!res.Result)
                    MessageBox.Show(res.Data, ErrorMessages.ERR_ERROR,
                        MessageBoxButtons.OK, MessageBoxIcon.Error);
                else
                {
                    MainForm app = new MainForm(client, LoginBox.Text);
                    client.AddMessage = app.AddMessage;
                    client.user_name = LoginBox.Text;
                    app.Show();

                    this.ShowInTaskbar = false;
                    this.Hide();
                }
            }
        }

        private void ExitButton_Click(object sender, EventArgs e)
        {
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
