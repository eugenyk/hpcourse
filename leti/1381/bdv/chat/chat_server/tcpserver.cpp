#include "tcpserver.h"
/*
int TcpServer::socket_descr;
int TcpServer::new_socket_descr[10];
pthread_t TcpServer::threads[10];
int TcpServer::clients_number;
int TcpServer::port;
struct sockaddr_in TcpServer::server_addr;
struct sockaddr_in TcpServer::client_addr;
std::vector<TcpServer::NameId> TcpServer::name_id;
*/
void* wait_client(void *ar)
{
    struct wait_client_args* args = (struct wait_client_args*) ar;
    TcpServer* srv = args->srv;
    struct sockaddr_in client_addr;
    socklen_t client = sizeof(client_addr);
    int new_socket_descr = accept(args->socket_descr, (struct sockaddr*) &client_addr, &client);
    if(new_socket_descr < 0)
    {
        std::cout << "Waiting client error" << std::endl;
        pthread_exit(0);
    }

    std::string name = srv->receive(new_socket_descr);
    std::cout << name << " connected" << std::endl;
    srv->add_client(name, new_socket_descr);
    /*
    NameId nd;
    nd.name = name;
    nd.id = i;
    name_id.push_back(nd);
    */

    while(true)
    {
        std::string rec_data = srv->receive(new_socket_descr);
        if(rec_data == "")
            break;
        std::string snd_name,rec_name, rec_msg;
        if(TcpServer::getnameandmsg(rec_data, rec_name, rec_msg))
        {
            snd_name = srv->findNameBySD(new_socket_descr);
            TcpServer::setnameandmsg(rec_data, snd_name, rec_msg);
            int rec_sd = srv->findSDByName(rec_name);
            if(rec_sd != -1)
                srv->send(rec_data, rec_sd);
        }
        else
            break;
    }
    close(new_socket_descr);
    pthread_exit(0);
}

TcpServer::TcpServer()
{

}

TcpServer::~TcpServer()
{
    delete wcargs;
}

void TcpServer::start(int portnum)
{
    port = portnum;
    socket_descr = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_descr < 0)
        std::cout << "Creating socket error" << std::endl;
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    int res = bind(socket_descr, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (res < 0)
        std::cout << "Binding error" << std::endl;
    listen(socket_descr, 5);
}

void TcpServer::wait_clients(int max_number)
{
    wcargs = new struct wait_client_args;
    wcargs->socket_descr = socket_descr;
    wcargs->srv = this;
    clients_number = max_number;
    for(int i = 0; i < max_number; i++)
    {
        int rc = pthread_create(&threads[i], 0, wait_client, (void *)wcargs);
        if(rc < 0)
            std::cout << "Creating thread " << i << " error" << std::endl;
    }

    for(int i = 0; i < max_number; i++)
    {
        void* status;
        pthread_join(threads[i], &status);
    }
}

void TcpServer::send(std::string msg, int socket_descriptor)
{
    int n = write(socket_descriptor, msg.data(), msg.size()*sizeof(char));
    if(n < 0)
        std::cout << "Sending error" << std::endl;
}

std::string TcpServer::receive(int socket_descriptor)
{
    char buffer[256];
    int n = read(socket_descriptor, buffer, 255);
    if(n < 0)
        std::cout << "Receiving error" << std::endl;
    else
    {
        std::string res(buffer, n);
        return res;
    }
    return "";
}

void TcpServer::close_()
{
    /*
    for(int i = 0; i < clients_number; i++)
        close(new_socket_descr[i]);
        */
    close(socket_descr);
}

bool TcpServer::getnameandmsg(std::string rec_data, std::string& name, std::string& msg)
{
    if(rec_data == "--")
        return false;
    int name_size = (int)rec_data[0];
    rec_data.erase(rec_data.begin());
    name = rec_data.substr(0, name_size);
    msg = rec_data.erase(0, name_size);
    return true;
}

void TcpServer::setnameandmsg(std::string& snd_data, std::string name, std::string msg)
{
    snd_data = "0" + name + msg;
    char name_size = name.size();
    snd_data[0] = name_size;
}

std::string TcpServer::findNameBySD(int socket_descriptor)
{
    for(int i = 0; i < name_id.size(); i++)
        if(name_id[i].socket_descriptor == socket_descriptor)
            return name_id[i].name;
    return "";
}

int TcpServer::findSDByName(std::string name)
{
    for(int i = 0; i < name_id.size(); i++)
        if(name_id[i].name == name)
            return name_id[i].socket_descriptor;
    return -1;
}

void TcpServer::add_client(std::string name, int socket_descriptor)
{
    struct NameSD ns;
    ns.name = name;
    ns.socket_descriptor = socket_descriptor;
    name_id.push_back(ns);
}
