using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Messenger.Results
{ 
    public class ErrorMessages
    {
        public const string ERR_USER_EXISTS = "Пользователь с таким именём уже существует!";
        public const string ERR_UNDEFINED_RESPONSE = "Ответ не распознан!";

        public const string ERR_USER_NOT_REGISTERED = "Пользователь не зарегистрирован!";

        public const string ERR_TIMEOUT = "Время ожидания ответа истекло!";
        public const string ERR_NOT_CONNECTION = "Ну удалось связаться с сервером!";

        public const string ERR_WRONG_PASSWORD = "Неверный пароль!";
        public const string ERR_ALREADY_ONLINE = "Вход был выполнен ранее!";

        public const string ERR_CONFIG_NOT_FOUND = "Отсутствует конфигурационный файл! \nПрограмма будет закрыта.";

        public const string ERR_ERROR = "Ошибка!";

        public const string ERR_NOT_ONLINE = "Сообщение не отправлено!Данный пользователь не в сети!";
    }
}
