using System.Linq;
using System.Collections.Generic;
using System;

namespace Server.DataModel
{
    public class ServerDataManagment
    {
        private ServerDataContext db = null;

        public ServerDataManagment()
        {
            db = new ServerDataContext();
            foreach (User u in db.Users)
                u.IsOnline = false;
        }

        public void CloseConnection()
        {
            db.Database.Connection.Close();
        }

        private void SaveChangesSync()
        {
            lock (this)
            {
                db.SaveChanges();
            }
        }

        public void AddUser(string login, string password)
        {
            lock (db.Users)
            {
                User tmp_user = new User();

                tmp_user.UserName = login;
                tmp_user.Password = password;
                tmp_user.IsOnline = false;

                db.Users.Add(tmp_user);
            }
            SaveChangesSync();
        }

        public void DeleteUser(string login)
        {
            lock (db.Users)
            {
                db.Users.Remove(GetUserByName(login));
            }
            SaveChangesSync();
        }

        public bool IsUserExist(string user_name)
        {
            if (GetUserByName(user_name) != null)
                return true;
            else
                return false;
        }

        public User GetUserByName(string user_name)
        {
            User user = null;
            lock (db.Users)
            {
                user = db.Users.Where(t => (t.UserName == user_name)).FirstOrDefault();
            }
            return user;
        }

        public void SetStatus(string user_name, bool online)
        {
            User tmp_user = GetUserByName(user_name);
            lock(tmp_user)
            {
                tmp_user.IsOnline = online;
            }

            SaveChangesSync();
        }

        public User GetUserByIPAndName(string user_name, string ip)
        {
            lock(db.Users)
            {
                IEnumerable<User> user = db.Users.Where(t => ((t.IPAdress == ip) && (t.UserName == user_name)));
                if (user.Count() > 0)
                    return user.ElementAt(0);
                else
                    return null;
            }
        }

        public void SetUserIP(string user_name, string ip)
        {
            User tmp = GetUserByName(user_name);
            lock(tmp)
            {
                tmp.IPAdress = ip;
            }
        }

        public bool GetUserStatus(string user_name)
        {
            User tmp = GetUserByName(user_name);
            if (tmp.IsOnline)
                return true;
            else
                return false;
        }

        public List<string> GetUserNamesList()
        {
            List<string> res = new List<string>();
            foreach(User tmp in db.Users)
                res.Add(tmp.UserName);

            return res;
        }

        public void AddContact(User m_user, string added_user_name)
        {
            User this_user = m_user;

            lock (this_user)
            {
                UserContact cont = new UserContact();
                cont.Friend = added_user_name;
                cont.UserId = this_user.Id;

                db.Contacts.Add(cont);
            }
            SaveChangesSync();
        }

        public void RemoveContact(User m_user, string rem_user_name)
        {
            User this_user = m_user;
            User rem_user = GetUserByName(rem_user_name);

            lock (this_user)
            {
                db.Contacts.RemoveRange(db.Contacts
                    .Where(c => ((c.UserId == m_user.Id) && (c.Friend == rem_user_name))));
            }

            SaveChangesSync();
        }

        public List<UserContact> GetUserContacts(string user_name)
        {
            var user = db.Users.Where(u => u.UserName == user_name).FirstOrDefault();

            List<UserContact> cont = db.Contacts.Where(c => c.UserId == user.Id).ToList();
            return cont;
        }
    }
}
