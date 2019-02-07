using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Data.Entity;

namespace Server.DataModel
{
    public class ServerDataContext : DbContext
    {
        public ServerDataContext() : base("DBConnection")
        {
            Database.SetInitializer(
                new DropCreateDatabaseIfModelChanges<ServerDataContext>());
        }

        public DbSet<User> Users { get; set; }
        public DbSet<UserContact> Contacts { get; set; }
    }
}
