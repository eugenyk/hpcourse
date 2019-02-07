using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Messenger.Utils
{
    public class QueryConsts
    {
        public const string RT_SEND_MESSAGE = "SEND";
        public const string RT_GET_STATUS = "GETSTATUS";
        public const string RT_LOGIN = "LOGIN";
        public const string RT_REGISTER = "REGISTER";

        public const string RT_NEW_MESSAGE = "NEW_MES";

        public const string RT_SUCCESS = "SUCCESS";
        public const string RT_FAILURE = "FAILURE";

        public const string RT_USER_EXISTS = "USER_EX";
        public const string RT_WRONG_PASSWORD = "WRONG_PAS";
        public const string RT_USER_NOT_FOUND = "USER_NF";
        public const string RT_USER_ALREADY_ONLINE = "USER_AO";

        public const string RT_USERS_LIST = "USER_LIST";
        public const string RT_ADD_TO_CONTACTS = "ADD_TO_CONT";

        public const string RT_REMOVE_FROM_CONTACTS = "REMOVE_FROM_CONT";
        public const string RT_GET_CONTACTS = "GET_CONTACTS";

        public const string RT_EMPTY_CONTACTS = "EMPTY_CONT";

        public const string RT_LOGOUT_USER = "LOGOUT";
    }
}
