using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security.Cryptography;
using System.IO;

namespace Messenger.Utils
{
    public static class Encryption
    {
        private static byte[] aes_iv = { 58, 75, 34, 207, 49, 0, 236, 24, 98, 90, 213, 114, 66, 126, 195, 62 };
        private static byte[] aes_key = {46, 216, 41, 21, 45, 12, 165, 224, 24, 148, 138, 95, 201, 189, 239, 38, 131, 140, 104, 132, 240, 192,
                                         28, 67, 74, 184, 241, 182, 119, 159, 112, 42};

        public static byte[] EncryptAES(string data)
        {
            if (data == null || data.Length <= 0)
                throw new ArgumentNullException("data");

            byte[] encrypted;

            using (Aes aes_syst = Aes.Create())
            {
                aes_syst.Key = aes_key;
                aes_syst.IV = aes_iv;

                ICryptoTransform encryptor = aes_syst.CreateEncryptor(aes_syst.Key, aes_syst.IV);

                using (MemoryStream ms = new MemoryStream())
                {
                    using (CryptoStream cs = new CryptoStream(ms, encryptor, CryptoStreamMode.Write))
                    {
                        using (StreamWriter sw = new StreamWriter(cs))
                        {
                            sw.Write(data);
                        }
                        encrypted = ms.ToArray();
                    }
                }
            }
            return encrypted;
        }

        public static string DecryptAES(byte[] crypted_data)
        {
            if (crypted_data == null || crypted_data.Length <= 0)
                throw new ArgumentNullException("crypted_data");

            string encrypted = null;

            using (Aes aes_syst = Aes.Create())
            {
                aes_syst.Key = aes_key;
                aes_syst.IV = aes_iv;

                ICryptoTransform decryptor = aes_syst.CreateDecryptor(aes_syst.Key, aes_syst.IV);

                using (MemoryStream ms = new MemoryStream(crypted_data))
                {
                    using (CryptoStream cs = new CryptoStream(ms, decryptor, CryptoStreamMode.Read))
                    {
                        using (StreamReader sr = new StreamReader(cs))
                        {
                            encrypted = sr.ReadToEnd();
                        }
                    }
                }

            }

            return encrypted;
        }
    }
}
