using Messenger.Utils;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Messenger.Utils
{
    public static class RequestBuilder
    {
        public static string AuthorizationRequest(string login, string password)
        {
            return QueryConsts.RT_LOGIN + " " + login + " " + password;
        }

        public static string SendMessageRequest(string sender, string recipient, string mes)
        {
            return QueryConsts.RT_SEND_MESSAGE + " " + sender + " " + recipient + " " + mes;
        }

        public static string GetStatusRequest(string user_name)
        {
            return QueryConsts.RT_GET_STATUS + user_name;
        }

        public static string RegisterRequest(string login, string password)
        {
            return QueryConsts.RT_REGISTER + " " + login + " " + password;
        }

        public static string RidirectedMessageRequest(string sender, string mes)
        {
            return QueryConsts.RT_NEW_MESSAGE + " " + sender + ":" + mes;
        }

        public static string GetUsersListRequest()
        {
            return QueryConsts.RT_USERS_LIST;
        }

        public static string AddToContactsRequest(string user_name)
        {
            return QueryConsts.RT_ADD_TO_CONTACTS + " " + user_name;
        }

        public static string RemoveFromContactsRequest(string user_name)
        {
            return QueryConsts.RT_REMOVE_FROM_CONTACTS + " " + user_name;
        }

        public static string GetContactsRequest(string user_name)
        {
            return QueryConsts.RT_GET_CONTACTS + " " + user_name;
        }

        public static string LogoutRequest(string user_name)
        {
            return QueryConsts.RT_LOGOUT_USER + " " + user_name;
        }
    }
}
