using System;
using System.Collections.Generic;
using System.ComponentModel.DataAnnotations;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Server.DataModel
{
    public class UserContact
    {
        [Key]
        public int Id { get; set; }

        public string Friend { get; set; }
        public int UserId { get; set; }
    }
}
