using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Carramba.Tortuga
{
    static class ElCapitano
    {
        public static async Task<string> MakeCommand(string command)
        {
            ProcessStartInfo info = new ProcessStartInfo("cmd", "/c " + command)
            {
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                WindowStyle = ProcessWindowStyle.Normal,
                UseShellExecute = false,
            };

            Process goOn = Process.Start(info);

            Task<string> outpot = goOn.StandardOutput.ReadToEndAsync();
            Task<string> outerr = goOn.StandardError.ReadToEndAsync();

            Task timeout = Task.Delay(3000);
            Task poterr = Task.WhenAll(outpot, outerr);
            try
            {
                await Task.WhenAny(poterr, timeout);
                if (poterr.IsCompleted)
                {
                    return outpot.Result + "\r\n" + outerr.Result;
                }
                else
                {
                    return "TIMEOUT";
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error whis: {0}",ex);
                return "FAIL: " + ex;
            }
            finally
            {
                try
                {
                    if (!goOn.HasExited)
                    {
                        goOn.CloseMainWindow();
                        goOn.Kill();
                    }
                }
                catch (Exception)
                {}
                finally
                {
                    goOn.Dispose();
                }
            }
        }
    }
}
