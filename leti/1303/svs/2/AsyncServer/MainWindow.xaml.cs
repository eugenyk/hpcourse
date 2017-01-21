using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Net;
using System.Net.Sockets;

namespace AsyncServer
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>

    public partial class MainWindow : Window
    {
        ManualResetEvent manualResetEvent = new ManualResetEvent(false);
        List<Thread> threadPool = new List<Thread>();
        TcpListener tcpListener = new TcpListener(IPAddress.Parse("127.0.0.1"), 1234);
        public MainWindow()
        {
            InitializeComponent();
            tcpListener.Start();
            Thread thread = new Thread(new ThreadStart(() => {
                while (true)
                {
                    tcpListener.BeginAcceptSocket(Handler, tcpListener);
                    manualResetEvent.WaitOne();
                    manualResetEvent.Reset();

                }
            }));
            thread.Start();
            threadPool.Add(thread);
        }
        private void Handler(IAsyncResult result) {
            Thread thread = new Thread(new ThreadStart(() => {
                while (true) {
                    try
                    {
                        ManualResetEvent manualResetEvent1 = new ManualResetEvent(false);
                        TcpListener tcpListener = result.AsyncState as TcpListener;
                        TcpClient tcpClient = tcpListener.EndAcceptTcpClient(result);
                        byte[] count = new byte[1];
                        tcpClient.GetStream().BeginRead(count, 0, 1, new AsyncCallback((IAsyncResult iAsyncResult) => {
                            tcpClient.GetStream().BeginWrite(new byte[1], 0, 1, new AsyncCallback((IAsyncResult iAsyncResult2) => {
                                byte[] buffer = new byte[count[0]];
                                tcpClient.GetStream().BeginRead(buffer, 0, buffer.Length, new AsyncCallback((iAsyncResultLast) =>
                                {
                                    string message = Encoding.UTF8.GetString(buffer);
                                    Dispatcher.Invoke(new ThreadStart(() => {
                                        TextBox textBox = new TextBox();
                                        textBox.Text = message;
                                        textBox.Margin = new Thickness(4, 4, 4, 0);
                                        textBox.TextWrapping = TextWrapping.Wrap;
                                        textBox.Width = 400;
                                        textBox.IsReadOnly = true;
                                        textBox.HorizontalAlignment = HorizontalAlignment.Left;
                                        MainArea.Children.Add(textBox);
                                    }));
                                }), tcpListener);
                            }), tcpListener);
                            manualResetEvent1.Set();
                        }), tcpListener);
                        manualResetEvent1.WaitOne();
                        manualResetEvent1.Reset();
                    }
                    catch (Exception e) {
                        return;
                    }
                }
            }));
            thread.Start();
            threadPool.Add(thread);
            try
            {
                manualResetEvent.Set();
            }
            catch (Exception e) {
                manualResetEvent.Close();
            }
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            foreach (Thread thread in threadPool) {
                thread.Abort();
            }
            tcpListener.Stop();
            manualResetEvent.Reset();
            manualResetEvent.Close();
        }
    }
}
