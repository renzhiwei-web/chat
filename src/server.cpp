#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include <vector>
using namespace std;



void receive(int clientfd,const sockaddr_in& caddr,const string& name){
    string clientip = inet_ntoa(caddr.sin_addr);
    cout << clientfd << "\t";
    cout << "ip address: " << clientip << " has connected\n";
    //TODO:多线程处理多个客户端连接
    // 第5步：与客户端通信，接收客户端发过来的报文后，回复ok。
    string buffer;
    while (true)
    {
        int iret;
        buffer.clear();
        buffer.resize(1024);
        // 接收客户端的请求报文，如果客户端没有发送请求报文，recv()函数将阻塞等待。
        // 如果客户端已断开连接，recv()函数将返回0。
        if ((iret = recv(clientfd, &buffer[0], 1024, 0)) <= 0)
        {
            cout << "iret=" << iret << endl;
            break;
        }
        if (buffer == "quit")
        {
            cout << "ip address: " << clientip << " has disconnected\n";
            break;
        }else{
            cout << "[" << name << "]: "  << buffer << endl;
        }
    }

    // 第6步：关闭socket，释放资源。
    close(clientfd); // 关闭客户端连上来的socket。
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Using:./demo6 通讯端口\nExample:./demo6 5005\n\n"; // 端口大于1024，不与其它的重复。
        cout << "注意：运行服务端程序的Linux系统的防火墙必须要开通5005端口。\n";
        cout << "      如果是云服务器，还要开通云平台的访问策略。\n\n";
        return -1;
    }

    // 第1步：创建服务端的socket。
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("socket");
        return -1;
    }

    // 第2步：把服务端用于通信的IP和端口绑定到socket上。
    struct sockaddr_in servaddr; // 用于存放协议、端口和IP地址的结构体。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;                // ①协议族，固定填AF_INET。
    servaddr.sin_port = htons(atoi(argv[1]));     // ②指定服务端的通信端口。
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // ③如果操作系统有多个IP，全部的IP都可以用于通讯。
    // servaddr.sin_addr.s_addr=inet_addr("192.168.101.138"); // ③指定服务端用于通讯的IP(大端序)。
    //  绑定服务端的IP和端口。
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        close(listenfd);
        return -1;
    }

    // 第3步：把socket设置为可连接（监听）的状态。
    if (listen(listenfd, 5) == -1)
    {
        perror("listen");
        close(listenfd);
        return -1;
    }

    vector<thread> threads;

    while(true){

        // 第4步：受理客户端的连接请求，如果没有客户端连上来，accept()函数将阻塞等待。
        struct sockaddr_in caddr;
        socklen_t addrlen = sizeof(caddr);

        int clientfd = accept(listenfd,(struct sockaddr *)&caddr, &addrlen);
        if (clientfd == -1)
        {
            perror("accept");
            close(listenfd);
            return -1;
        }

        string name;
        name.clear();
        name.resize(1024);
        int readn = recv(clientfd,&name[0],1024,0);
        name.resize(readn);
        cout << name << endl;
        thread t1(receive,clientfd,ref(caddr),name);
        threads.push_back(move(t1));
        
    }
    
    for(int i = 0;i < threads.size();i++){
        if (threads[i].joinable())
        {
            threads[i].join();
        }
        
    }
    close(listenfd); // 关闭服务端用于监听的socket。
}