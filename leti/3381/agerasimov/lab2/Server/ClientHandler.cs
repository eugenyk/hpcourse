using System;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Collections.Generic;

using Messenger.Utils;

using Server.DataModel;

namespace Server
{
    public class ClientHandler
    {
        private Socket client;
        private ServerDataManagment db;
        private MainForm log;
        private List<KeyValuePair<string, Socket>> connections;

        private User m_user;

        public ClientHandler(Socket _client, ServerDataManagment dbc, MainForm log_form, List<KeyValuePair<string, Socket>> conns)
        {
            client = _client;
            db = dbc;
            log = log_form;
            connections = conns;
        }

        public void Start()
        {
            try
            {
                while (true)
                {
                    StringBuilder builder = new StringBuilder();

                    int received = 0;
                    byte[] buffer = new byte[256];

                    List<byte> encrypted = new List<byte>();
                    do
                    {
                        received += client.Receive(buffer);
                        encrypted.AddRange(buffer);
                    }
                    while (client.Available > 0);

                    encrypted.RemoveRange(received, encrypted.Count - received);
                    string data = Encryption.DecryptAES(encrypted.ToArray());

                    log.WriteLog("Принят запрос от " + Tools.GetIPFromSocket(client) + " : " +
                                 data);

                    ParseRequest(data);
                }
            }
            catch (SocketException ex)
            {
                log.WriteLog("Пользователь " + Tools.GetIPFromSocket(client) + " был отключён");


                client.Dispose();
            }



        }

        private bool SendResponse(string resp_str, Socket dst = null)
        {
            byte[] data = Encryption.EncryptAES(resp_str);

            if (dst == null)
                dst = client;

            try
            {
                dst.Send(data);
                return true;
            }
            catch (Exception ex)
            {
                return false;
            }
        }

        private void ParseRequest(string req)
        {
            string[] cons = req.Split(' ');

            if (cons[0] == QueryConsts.RT_LOGIN)
                LoginUser(cons);
            else if (cons[0] == QueryConsts.RT_REGISTER)
                RegisterUser(cons);
            else if (cons[0] == QueryConsts.RT_USERS_LIST)
                GetUserList();
            else if (cons[0] == QueryConsts.RT_ADD_TO_CONTACTS)
                AddUserToContacts(cons);
            else if (cons[0] == QueryConsts.RT_REMOVE_FROM_CONTACTS)
                RemoveUserToContacts(cons);
            else if (cons[0] == QueryConsts.RT_GET_CONTACTS)
                GetContacts(cons);
            else if (cons[0] == QueryConsts.RT_SEND_MESSAGE)
                SendMessage(cons);
            else if (cons[0] == QueryConsts.RT_LOGOUT_USER)
                LogoutUser(cons);

        }

        private void RegisterUser(string[] user_data)
        {
            string response;
            if (!db.IsUserExist(user_data[1]))
            {
                db.AddUser(user_data[1], user_data[2]);

                log.WriteLog("Пользователь " + user_data[1] + " успешно зарегистрирован");

                response = ResponseBuilder.Build(QueryConsts.RT_REGISTER, QueryConsts.RT_SUCCESS);
                bool result = SendResponse(response);
                if (!result)
                {
                    log.WriteLog("Пользователь " + user_data[1] + " был удален, т.к. он не принял подтверждение");
                    db.DeleteUser(user_data[1]);
                }
            }
            else
            {
                log.WriteLog("Пользователь " + user_data[1] + " не был зарегистрирован, т.к. такое имя уже существует");

                response = ResponseBuilder.Build(QueryConsts.RT_REGISTER, QueryConsts.RT_USER_EXISTS);
                SendResponse(response);
            }
        }

        private void LoginUser(string[] user_data)
        {
            string response;

            User tmp_user = db.GetUserByName(user_data[1]);
            if (tmp_user != null)
            {
                if (!tmp_user.IsOnline)
                {
                    if (tmp_user.Password == user_data[2])
                    {
                        m_user = tmp_user;

                        KeyValuePair<string, Socket> cur_conn = new KeyValuePair<string, Socket>(user_data[1], client);
                        connections.Add(cur_conn);

                        db.SetStatus(user_data[1], true);
                        db.SetUserIP(user_data[1], Tools.GetIPFromSocket(client));

                        log.WriteLog("Пользователь " + user_data[1] + " успешно авторизовался");

                        response = ResponseBuilder.Build(QueryConsts.RT_LOGIN, QueryConsts.RT_SUCCESS);

                        bool result = SendResponse(response);
                        if (!result)
                        {
                            db.SetStatus(user_data[1], false);
                            log.WriteLog("Пользователь " + user_data[1] + " принудительно покинул систему, т.к. он не принял подтверждение");
                        }
                    }
                    else
                    {
                        log.WriteLog("Пользователь " + user_data[1] + " не смог войти в систему, т.к. ввел неправильный пароль");

                        response = ResponseBuilder.Build(QueryConsts.RT_LOGIN, QueryConsts.RT_WRONG_PASSWORD);
                        SendResponse(response);
                    }
                }
                else
                {
                    log.WriteLog("Пользователь " + user_data[1] + " не смог войти в систему, т.к. он уже находится в системе");

                    response = ResponseBuilder.Build(QueryConsts.RT_LOGIN, QueryConsts.RT_USER_ALREADY_ONLINE);
                    SendResponse(response);
                }
            }
            else
            {
                log.WriteLog("Пользователь " + user_data[1] + " не смог войти в систему, т.к. он не зарегистрирован");

                response = ResponseBuilder.Build(QueryConsts.RT_LOGIN, QueryConsts.RT_USER_NOT_FOUND);
                SendResponse(response);
            }

        }

        private void LogoutUser(string[] cons)
        {
            db.SetStatus(cons[1], false);
            connections.Remove(connections.Where(u => u.Key == cons[1]).FirstOrDefault());

            log.WriteLog("Пользователь " + Tools.GetIPFromSocket(client) + " вышел из системы");
        }

        private void SendMessage(string[] req_data)
        {
            User sender = db.GetUserByName(req_data[1]);
            User recipient = db.GetUserByName(req_data[2]);

            if (recipient.IsOnline)
            {
                Socket recip = (connections
                    .Where(t => (t.Key == recipient.UserName) && (Tools.GetIPFromSocket(t.Value) == recipient.IPAdress)))
                    .FirstOrDefault().Value;


                lock (recip)
                {
                    string mess0 = "";
                    if (req_data.Length > 4)
                    {
                        for (int i = 3; i < req_data.Length; i++)
                            mess0 += req_data[i] + " ";
                        mess0 = mess0.Remove(mess0.Length - 1, 1);
                    }
                    else
                        mess0 = req_data[3];

                    string response = RequestBuilder.RidirectedMessageRequest(sender.UserName, mess0);
                    if (SendResponse(response, recip))
                    {
                        string sender_resp = ResponseBuilder.Build(QueryConsts.RT_SEND_MESSAGE, QueryConsts.RT_SUCCESS);
                        if (SendResponse(sender_resp))
                        {
                            log.WriteLog("Сообщение от " + sender.IPAdress + " к " + recipient.IPAdress + " успешно доставлено");
                        }
                        else
                            log.WriteLog("Сообщение от " + sender.IPAdress + " к " + recipient.IPAdress + " не доставлено, т.к. не было ответа от отправителя");

                    }
                    else
                        log.WriteLog("Сообщение от " + sender.IPAdress + " к " + recipient.IPAdress + " не доставлено, т.к. не было ответа от получателя");
                }
            }
            else
            {
                string sender_resp = ResponseBuilder.Build(QueryConsts.RT_SEND_MESSAGE, QueryConsts.RT_FAILURE);
                if (SendResponse(sender_resp))
                    log.WriteLog("Сообщение от " + sender.IPAdress + " к " + recipient.IPAdress + " не доставлено, т.к получатель не онлайн");
            }
        }


        private void GetUserList()
        {
            StringBuilder sb = new StringBuilder();

            foreach (string t in db.GetUserNamesList())
            {
                if (t != m_user.UserName)
                    sb.Append(t + " ");
            }

            if (sb.Length > 0)
                sb.Remove(sb.Length - 1, 1);

            string response = ResponseBuilder.Build(QueryConsts.RT_USERS_LIST, sb.ToString());
            if (SendResponse(response))
                log.WriteLog("Пользователь " + Tools.GetIPFromSocket(client) + " получил список пользователей");
        }

        private void AddUserToContacts(string[] req)
        {
            db.AddContact(m_user, req[1]);

            string response = ResponseBuilder.Build(QueryConsts.RT_ADD_TO_CONTACTS, QueryConsts.RT_SUCCESS);
            if (SendResponse(response))
                log.WriteLog("Пользователь " + Tools.GetIPFromSocket(client) + " добавил в список контакта пользователя " + req[1]);
        }

        private void RemoveUserToContacts(string[] req)
        {
            db.RemoveContact(m_user, req[1]);

            string response = ResponseBuilder.Build(QueryConsts.RT_ADD_TO_CONTACTS, QueryConsts.RT_SUCCESS);
            if (SendResponse(response))
                log.WriteLog("Пользователь " + Tools.GetIPFromSocket(client) + " удалил из списка контактов пользователя " + req[1]);
        }

        private void GetContacts(string[] req)
        {
            var cont = db.GetUserContacts(req[1]);

            StringBuilder sb = new StringBuilder();


            if (cont.Count != 0)
            {
                foreach (UserContact tmp_cont in cont)
                    sb.Append(tmp_cont.Friend + " ");

                sb.Remove(sb.Length - 1, 1);
            }
            else
                sb.Append(QueryConsts.RT_EMPTY_CONTACTS);

            string response = ResponseBuilder.Build(QueryConsts.RT_GET_CONTACTS, sb.ToString());
            if (SendResponse(response))
                log.WriteLog("Пользователю " + Tools.GetIPFromSocket(client) + " был отправлен список его контактов");
        }
    }
}
