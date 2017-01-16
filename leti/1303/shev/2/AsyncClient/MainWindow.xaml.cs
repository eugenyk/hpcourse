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
namespace AsyncClient
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }
        public void SendMessage(object sender, RoutedEventArgs e) {
            TcpClient tcpClient = new TcpClient("127.0.0.1", 1234);
            string message = Name.Text + "\t" + Message.Text;
            byte[] buffer = Encoding.UTF8.GetBytes(message);
            tcpClient.GetStream().BeginWrite(new byte[] { (byte)buffer.Length }, 0, 1, new AsyncCallback((IAsyncResult iAsyncResult) => {
                tcpClient.GetStream().BeginRead(new byte[1], 0, 1, new AsyncCallback((IAsyncResult iAsyncResult1) => {
                    tcpClient.GetStream().BeginWrite(buffer, 0, buffer.Length, new AsyncCallback((IAsyncResult iAsyncResult2) => {
                        tcpClient.Close();
                    }), tcpClient);
                }), tcpClient);
            }), tcpClient);
        }
    }
}
