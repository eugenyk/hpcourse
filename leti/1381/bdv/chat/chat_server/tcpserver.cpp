#include "tcpserver.h"

int TcpServer::socket_descr;
int TcpServer::new_socket_descr[10];
pthread_t TcpServer::threads[10];
int TcpServer::clients_number;
int TcpServer::port;
struct sockaddr_in TcpServer::server_addr;
struct sockaddr_in TcpServer::client_addr;
std::vector<TcpServer::NameId> TcpServer::name_id;

TcpServer::TcpServer()
{

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

void* TcpServer::wait_client(void *id)
{
    socklen_t client = sizeof(client_addr);
    long i = (long)id;
    new_socket_descr[i] = accept(socket_descr, (struct sockaddr*) &client_addr, &client);
    if(new_socket_descr[i] < 0)
    {
        std::cout << "Waiting client error" << std::endl;
        pthread_exit(-1);
    }

    std::string name = receive(i);
    std::cout << name << " connected" << std::endl;
    NameId nd;
    nd.name = name;
    nd.id = i;
    name_id.push_back(nd);

    while(true)
    {
        std::string rec_data = receive(i);
        if(rec_data == "")
            break;
        std::string snd_name,rec_name, rec_msg;
        if(getnameandmsg(rec_data, rec_name, rec_msg))
        {
            snd_name = findNameById(i);
            setnameandmsg(rec_data, snd_name, rec_msg);
            int rec_id = findIdByName(rec_name);
            if(rec_id != -1)
                send(rec_data, rec_id);
        }
        else
            break;
    }

    pthread_exit(0);
}

void TcpServer::wait_clients(int max_number)
{
    clients_number = max_number;
    for(int i = 0; i < max_number; i++)
    {
        int rc = pthread_create(&threads[i], 0, wait_client, (void *)i);
        if(rc < 0)
            std::cout << "Creating thread " << i << " error" << std::endl;
    }

    for(int i = 0; i < max_number; i++)
    {
        void* status;
        pthread_join(threads[i], &status);
    }
}

void TcpServer::send(std::string msg, int receiver_id)
{
    int n = write(new_socket_descr[receiver_id], msg.data(), msg.size()*sizeof(char));
    if(n < 0)
        std::cout << "Sending error" << std::endl;
}

std::string TcpServer::receive(int sender_id)
{
    char buffer[256];
    int n = read(new_socket_descr[sender_id], buffer, 255);
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
    for(int i = 0; i < clients_number; i++)
        close(new_socket_descr[i]);
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

std::string TcpServer::findNameById(int id)
{
    for(int i = 0; i < name_id.size(); i++)
        if(name_id[i].id == id)
            return name_id[i].name;
    return "";
}

int TcpServer::findIdByName(std::string name)
{
    for(int i = 0; i < name_id.size(); i++)
        if(name_id[i].name == name)
            return name_id[i].id;
    return -1;
}
