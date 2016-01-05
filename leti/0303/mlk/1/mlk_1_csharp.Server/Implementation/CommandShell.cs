using NLog;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace DevoidTalk.Server
{
    public sealed class CommandShell
    {
        static readonly Logger logger = LogManager.GetCurrentClassLogger();

        readonly string shellPath;
        readonly Func<string, string> shellArgTransformer;

        readonly AutoResetEvent resetEvent = new AutoResetEvent(true);

        public CommandShell(string shellPath, Func<string, string> shellArgTransformer)
        {
            this.shellPath = shellPath;
            this.shellArgTransformer = shellArgTransformer;
        }

        public Task<string> TryStartExecuting(string command, TimeSpan timeout)
        {
            if (resetEvent.WaitOne(0))
            {
                logger.Info($"Running `{command}`...");
                var task = Run(command, timeout);
                task.ContinueWith(t => resetEvent.Set());
                return task;
            }
            else
            {
                logger.Info($"Failed to concurrently run `{command}`");
                return null;
            }
        }

        private async Task<string> Run(string shellCommand, TimeSpan timeout)
        {
            ProcessStartInfo processStartInfo = new ProcessStartInfo(shellPath, shellArgTransformer(shellCommand))
            {
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                WindowStyle = ProcessWindowStyle.Normal,
                UseShellExecute = false,
            };

            Process process = Process.Start(processStartInfo);

            Task<string> output = process.StandardOutput.ReadToEndAsync();
            Task<string> error = process.StandardError.ReadToEndAsync();

            Task readTask = Task.WhenAll(
                output.ContinueWith(t => process.StandardOutput.Close()),
                error.ContinueWith(t => process.StandardError.Close()));

            await Task.WhenAny(readTask, Task.Delay(timeout));
            if (readTask.IsCompleted)
            {
                string result = $"{await output}{Environment.NewLine}{await error}";
                logger.Info($"Shell command completed with result: {Environment.NewLine}{result}");
                return result;
            }
            else
            {
                try { process.Kill(); }
                catch (Exception ex) { logger.Warn(ex, "Proccess kill error"); }
                throw new OperationCanceledException("Shell command execution timed out");
            }
        }
    }
}
