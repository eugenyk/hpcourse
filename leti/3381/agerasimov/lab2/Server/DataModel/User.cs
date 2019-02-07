using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;

namespace Server.DataModel
{
    public class User
    {
        [Key]
        public int Id { get; set; }

        public string UserName { get; set; }
        public string Password { get; set; }

        public bool IsOnline { get; set; }

        public string IPAdress { get; set; }
    }
}
