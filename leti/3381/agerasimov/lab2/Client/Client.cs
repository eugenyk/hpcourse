using System;

using System.Threading;
using System.Collections.Generic;

using System.Net;
using System.Net.Sockets;
using System.Text;

using System.Windows.Forms;

using Messenger.Utils;
using Messenger.Results;

namespace Client
{
    public class Client
    {
        public Socket server;
        public Thread listen_thread = null;

        private string recieved_data = null;

        public string user_name;

        public bool is_connected = false;

        public Client()
        {
            InitializeClient();
        }

        public delegate void AddMessageDeletate(string a, string b, string c);
        public AddMessageDeletate AddMessage = null;

        public void StartListening(AddMessageDeletate add_mes_del = null)
        {
            listen_thread = new Thread(new ThreadStart(new Action(() =>
            {
                try
                {
                    while (true)
                    {
                        int recieved = 0;
                        byte[] data = new byte[256];

                        StringBuilder builder = new StringBuilder();

                        List<byte> encrypted = new List<byte>();

                        do
                        {
                            recieved += server.Receive(data);
                            encrypted.AddRange(data);
                        }
                        while (server.Available > 0);

                        is_connected = true;

                        encrypted.RemoveRange(recieved, encrypted.Count - recieved);
                        string resp = Encryption.DecryptAES(encrypted.ToArray());

                        string[] det0 = resp.Split(' ');

                        if (det0[0] == QueryConsts.RT_NEW_MESSAGE)
                        {
                            string[] param = resp.Split(':');
                            string mes = param[1];

                            string[] info = param[0].Split(' ');

                            AddMessage(info[1], user_name, mes);
                        }
                        else
                            recieved_data = resp;
                    }
                }
                catch (Exception ex) { is_connected = false; Thread.CurrentThread.Abort(); }
            })));
            listen_thread.Start();
        }

        public void InitializeClient()
        {
            string server_ip; int port;
            Tools.GetNetworkConfig(out server_ip, out port);

            try
            {
                IPEndPoint s_ip = new IPEndPoint(IPAddress.Parse(server_ip), port);

                server = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                server.Connect(s_ip);

                is_connected = true;
            }
            catch (SocketException ex)
            {
                MessageBox.Show("Связь с сервером отсуствует!Повторите попытку позже!", "Ошибка",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        public ActionResult RegisterUser(string login, string password)
        {
            string request = RequestBuilder.RegisterRequest(login, password);

            if (SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                {
                    string res = ResponseBuilder.Parse(response);
                    if (res == QueryConsts.RT_SUCCESS)
                        return new ActionResult(true, InfoMessages.INF_SUCCESS_REGISTRATION);
                    else if (res == QueryConsts.RT_USER_EXISTS)
                        return new ActionResult(false, ErrorMessages.ERR_USER_EXISTS);
                    else
                        return new ActionResult(false, ErrorMessages.ERR_UNDEFINED_RESPONSE);
                }
                else
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
            }
            else
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
        }

        public ActionResult LoginUser(string login, string password)
        {
            string request = RequestBuilder.AuthorizationRequest(login, password);
            if (SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                {
                    string res = ResponseBuilder.Parse(response);
                    if (res == QueryConsts.RT_SUCCESS)
                        return new ActionResult(true, InfoMessages.INF_SUCCESS_LOGIN);
                    else if (res == QueryConsts.RT_WRONG_PASSWORD)
                        return new ActionResult(false, ErrorMessages.ERR_WRONG_PASSWORD);
                    else if (res == QueryConsts.RT_USER_ALREADY_ONLINE)
                        return new ActionResult(false, ErrorMessages.ERR_ALREADY_ONLINE);
                    else if (res == QueryConsts.RT_USER_NOT_FOUND)
                        return new ActionResult(false, ErrorMessages.ERR_USER_NOT_REGISTERED);
                    else
                        return new ActionResult(false, ErrorMessages.ERR_UNDEFINED_RESPONSE);
                }
                else
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
            }
            else
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
        }

        public ActionResult GetUsersList()
        {
            string request = RequestBuilder.GetUsersListRequest();
            if(SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                {
                    return new ActionResult(true, response);
                }
                else
                {
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
                }
            }
            else
            {
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
            }
        }

        public ActionResult AddUserToContacts(string user_name)
        {
            string request = RequestBuilder.AddToContactsRequest(user_name);
            if (SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                {
                    return new ActionResult(true, response);
                }
                else
                {
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
                }
            }
            else
            {
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
            }

        }

        public ActionResult RemoveUserFromContacts(string user_name)
        {
            string request = RequestBuilder.RemoveFromContactsRequest(user_name);
            if (SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                {
                    return new ActionResult(true, response);
                }
                else
                {
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
                }
            }
            else
            {
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
            }

        }

        public ActionResult GetContacts(string user_name)
        {
            string request = RequestBuilder.GetContactsRequest(user_name);
            if (SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                    return new ActionResult(true, response);
                else
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
            }
            else
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
        }

        public ActionResult Logout(string user_name)
        {
            string request = RequestBuilder.LogoutRequest(user_name);
            if (SendRequest(request))
            {
                return new ActionResult(true, null);
            }
            else
            {
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
            }

        }

        public void CloseConnection()
        {
            try { server.Close(); }
            catch (Exception ex) { }
        }

        public ActionResult SendMessage(string sender, string recipient, string text)
        {
            string request = RequestBuilder.SendMessageRequest(sender, recipient, text);
            if (SendRequest(request))
            {
                string response;
                if (RecieveResponse(out response))
                {
                    string res = ResponseBuilder.Parse(response);
                    if (res == QueryConsts.RT_SUCCESS)
                    {
                        return new ActionResult(true, InfoMessages.INF_SUCCESS_LOGIN);
                    }
                    else if (res == QueryConsts.RT_FAILURE)
                    {
                        return new ActionResult(false, ErrorMessages.ERR_NOT_ONLINE);
                    }
                    else
                    {
                        return new ActionResult(false, ErrorMessages.ERR_UNDEFINED_RESPONSE);
                    }
                }
                else
                {
                    return new ActionResult(false, ErrorMessages.ERR_TIMEOUT);
                }
            }
            else
            {
                return new ActionResult(false, ErrorMessages.ERR_NOT_CONNECTION);
            }
        }

        public bool SendRequest(string request)
        {
            byte[] data = Encryption.EncryptAES(request);

            try
            {
                server.Send(data);
                return true;
            }
            catch(Exception ex)
            {
                return false;
            }

        }

        public bool RecieveResponse(out string resp)
        {
            try
            {
                while (recieved_data == null) { }
                resp = recieved_data;

                recieved_data = null;

                return true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
                resp = null;
                return false;
            }
        }

    }
}
