using System;
using CommandLine;
using CommandLine.Text;
using System.Reflection;
using System.IO;
using System.Threading;
using DevoidTalk.Core;
using NLog;
using System.Threading.Tasks;

namespace DevoidTalk.Server
{
    static class Program
    {
        static readonly Logger logger = LogManager.GetCurrentClassLogger();

        private sealed class Options
        {
            [Option('p', "port", DefaultValue = 10000, HelpText = "The network port for incoming connections.")]
            public int Port { get; set; }

            [Option("welcome", DefaultValue = "Welcome to DevoidTalk.Server", HelpText = "Welcoming message for connecting users.")]
            public string WelcomeMessage { get; set; }

            [HelpOption]
            public string GetUsage()
            {
                var serverAssembly = typeof(Program).Assembly;
                var assemblyName = serverAssembly.GetName();
                var title = serverAssembly.GetCustomAttribute<AssemblyTitleAttribute>();
                var company = serverAssembly.GetCustomAttribute<AssemblyCompanyAttribute>();

                var help = new HelpText {
                    Heading = new HeadingInfo(title.Title, assemblyName.Version.ToString()),
                    Copyright = new CopyrightInfo(company.Company, 2015),
                    AdditionalNewLineAfterOption = true,
                    AddDashesToOption = true
                };
                var assemblyExecutableName = Path.GetFileNameWithoutExtension(serverAssembly.Location);
                help.AddPreOptionsLine(string.Format("Usage: {0} -p 10000", assemblyExecutableName));
                help.AddOptions(this);
                return help;
            }
        }

        public static void Main(string[] args)
        {
            var options = new Options();
            if (Parser.Default.ParseArguments(args, options))
            {
                StartServer(options);
            }
            else
            {
                // Display the default usage information
                Console.WriteLine(options.GetUsage());
            }
        }

        private static void StartServer(Options options)
        {
            logger.Info("==========================");
            logger.Info("Starting server...");

            var cancellationSource = new CancellationTokenSource();
            var cancellation = cancellationSource.Token;

            Console.CancelKeyPress += (sender, e) =>
            {
                e.Cancel = true;
                logger.Info("Gracefully stopping server...");
                cancellationSource.Cancel();
            };

            var threadPool = new Core.ThreadPool(10, cancellation);
            IClientAcceptor acceptor = new TcpClientAcceptor(options.Port);
            var connectionManager = new ConnectionManager(acceptor, cancellation);
            var broadcastingChat = new BroadcastingChat(connectionManager, options.WelcomeMessage);
            var commandShell = new CommandShell("cmd", command => $"/c {command}");

            Func<ClientConnection, string, Task> processCommand = async (client, command) =>
            {
                Message reply;
                if (command.StartsWith("/c "))
                {
                    var task = commandShell.TryStartExecuting(command.Substring(3), TimeSpan.FromSeconds(10));
                    if (task == null)
                        reply = new Message { Sender = "<server-shell>", Text = "Another command is already running." };
                    else
                    {
                        await broadcastingChat.ReplyTo(client, new Message { Sender = "<server-shell>", Text = $"Running `{command}`..." });

                        try { reply = new Message { Sender = "<server-shell>", Text = $"Execution result: {Environment.NewLine}{await task}" }; }
                        catch (OperationCanceledException) { reply = new Message { Sender = "<server-shell>", Text = $"Execution timed out." }; }
                    }
                }
                else
                    reply = new Message { Sender = "<server>", Text = $"Invalid command '{command}'" };

                await broadcastingChat.ReplyTo(client, reply);
            };

            broadcastingChat.IncomingMessageStrategy = incoming =>
            {
                var message = incoming.Message.Text.TrimStart();
                if (message.StartsWith("/"))
                    return processCommand(incoming.Sender, message);
                else
                    return broadcastingChat.BroadcastToAll(incoming.Message);
            };

            var tcs = new TaskCompletionSource<bool>();
            threadPool.Post(async () =>
            {
                logger.Info($"Start accepting clients at port {options.Port}");
                try
                {
                    await acceptor.Listen(cancellation);
                    logger.Info("Finish accepting clients");
                    tcs.TrySetResult(true);
                }
                catch (OperationCanceledException)
                {
                    tcs.SetCanceled();
                }
                catch (Exception ex)
                {
                    logger.Error(ex, "Error accepting clients");
                    tcs.SetException(ex);
                }
            });

            try
            {
                tcs.Task.Wait();
            }
            catch (AggregateException ex)
            {
                try { ex.Handle(exc => exc is OperationCanceledException); }
                catch (Exception innerEx) { logger.Error(innerEx); }
            }
            logger.Info("Server stopped");
        }
    }
}
