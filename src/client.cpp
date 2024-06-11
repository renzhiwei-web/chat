#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <vector>
#include <sstream>
#include <thread>
#include "message.h"
#include <sys/stat.h>
#include <fstream>
using namespace std;

// 实现客户端之间的相互感知
vector<string> clientnames;

// 多线程接收服务端的消息
void receive(int socketfd);
void send_file(int socketfd,Message& message);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Using:./demo5 服务端的IP 服务端的端口\nExample:./demo5 192.168.101.138 5005\n\n";
        return -1;
    }

    // 第1步：创建客户端的socket。
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return -1;
    }

    // 第2步：向服务器发起连接请求。
    struct sockaddr_in servaddr; // 用于存放协议、端口和IP地址的结构体。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;            // ①协议族，固定填AF_INET。
    servaddr.sin_port = htons(atoi(argv[2])); // ②指定服务端的通信端口。

    struct hostent *h;                           // 用于存放服务端IP地址(大端序)的结构体的指针。
    if ((h = gethostbyname(argv[1])) == nullptr) // 把域名/主机名/字符串格式的IP转换成结构体。
    {
        cout << "gethostbyname failed.\n"
             << endl;
        close(sockfd);
        return -1;
    }
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length); // ③指定服务端的IP(大端序)。

    // servaddr.sin_addr.s_addr=inet_addr(argv[1]); // ③指定服务端的IP，只能用IP，不能用域名和主机名。
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) // 向服务端发起连接清求。
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    // 首先将名字发送给服务端
    Message message = {0};
    message.message_flag = 0;
    cout << "input your name\n";
    string name;
    cin >> name;
    strcpy(message.content,name.data());
    send(sockfd,&message,sizeof(message),0);
    //  实现客户端之间的相互感知
    message = {0};
   
    recv(sockfd,&message,sizeof(message),0);
    istringstream iss(message.content);
    string data;
    while (iss >> data)
    {
        clientnames.push_back(data);
    }
    for(auto it = clientnames.begin();it != clientnames.end();it++){
        cout << *it << "\n";
    }
    // 客户端连接之后，服务器端将所维护的客户端列表发送给每一个客户端
    thread t1(receive,sockfd);
    // 客户端断链之后，服务器端同样将客户端列表发送给每一个客户端


    // 第3步：与服务端通讯，客户发送一个请求报文后等待服务端的回复，收到回复后，再发下一个请求报文。
    // TODO: 实现客户端之间的通讯
    while(true)
    {
        int iret;
        message = {0};
        // TODO: 文件发送功能
        std::cout << "plz to choose function\n" 
                  << "1:direct message\t" << "2:group chat\t" << "3:file\t" << "4:quit\n";
        unsigned short flag;
        cin >> flag;
        if (std::cin.fail())
        {
            std::cout << "plz input correct option\n";
            std::cin.clear();
            std::cin.ignore();
            continue;
        }
        switch (flag)
        {
        case 1:{
            message.message_flag = 1;
            strcpy(message.source,name.data());
            std::cout << "current online friends\n";
            for(int i = 0;i < clientnames.size();i++){
                cout << clientnames[i] << endl;
            }
            std::cout << "input name whose you want to chat\n";
            cin >> message.dest;
            std::cout << "input message\n";
            cin >> message.content;
            send(sockfd,&message,sizeof(message),0);
            break;
        }
        case 2:
            message.message_flag = 1;
            strcpy(message.source,name.data());
            std::cout << "current online friends\n";
            for(int i = 0;i < clientnames.size();i++){
                cout << clientnames[i] << endl;
            }
            strcpy(message.dest,all.data());
            std::cout << "input message\n";
            cin >> message.content;
            send(sockfd,&message,sizeof(message),0);
            break;
        case 3:
            // TODO file
            // 发送文件时，采用同步阻塞的方式
            message.message_flag = 2;
            strcpy(message.source,name.data());
            send_file(sockfd,message);
            break;
        case 4:
            message.message_flag = 0;
            strcpy(message.content,quit.data());
            send(sockfd,&message,sizeof(message),0);
            cout << name << "has quit\n";
            goto here;
        default:
            break;
        }
    }
    here:
    if (t1.joinable())
    {
        t1.join();
    }
    
    // 第4步：关闭socket，释放资源。
    close(sockfd);
}

void receive(int socketfd){
    Message message;
    while (true)
    {
        int iret;
        message = {0};
        // 接收客户端的请求报文，如果客户端没有发送请求报文，recv()函数将阻塞等待。
        // 如果客户端已断开连接，recv()函数将返回0。
        if ((iret = recv(socketfd, &message, sizeof(message), 0)) <= 0)
        {
            cout << "iret=" << iret << endl;
            break;
        }
        // 如果是系统消息，则进行专门处理
        if (message.message_flag == 0){
            if(message.content == quit){
                return;
            }
            clientnames.clear();
            istringstream iss(message.content);
            string data;
            while (iss >> data)
            {
                clientnames.push_back(data);
            }
            cout << "friends online or offline\n";
            for(auto it = clientnames.begin();it != clientnames.end();it++){
                cout << *it << "\n";
            }
        }else if(message.message_flag == 1){ // 用户消息，则直接进行显示
            cout << "[" << message.source << "]:" << message.content << endl;
        } else{ // 接收文件
            istringstream iss(message.content);
            st_fileinfo fileinfo;
            int n = 0;
            iss >> fileinfo.filename >> fileinfo.filesize >> n;
            string filename = string("./file/") + fileinfo.filename;
            ofstream fout(filename,ios::binary);
            size_t totalbytes = fileinfo.filesize;
            size_t onread = 0;
            char buffer[maxlen];
            for(int i = 0;i < n;i++){
                onread = min(totalbytes,maxlen);
                recv(socketfd,buffer,onread,0);
                fout.write(buffer,onread);
                totalbytes -= onread;
            }
            fout.close();
            cout << "receive file completed\n";
        }
    }
}

void send_file(int socketfd,Message& message){
    st_fileinfo fileinfo;

    // 获得文件名
    cout << "input filename\n";
    cin >> fileinfo.filename;

    // 获得文件大小
    struct stat st;
    stat(fileinfo.filename,&st);
    fileinfo.filesize = st.st_size;

    // 尝试打开文件
    ifstream fin(fileinfo.filename,ios::binary);
    if (fin.is_open() == false)
    {
        cout << "open file failed\n";
        return;
    }

    // 准备消息体内容
    message.message_flag = 2;
    cout << "input name whose you want to send\n";
    cin >> message.dest;

    // 需要发送几次消息块
    int n = (fileinfo.filesize - 1) / maxlen + 1;
    // 序列化文件信息
    ostringstream oss;
    oss << fileinfo.filename << " " << fileinfo.filesize << " " << n;
    strcpy(message.content,oss.str().data());

    // 发现文件信息以及需要发送的次数
    send(socketfd,&message,sizeof(message),0);

    // 读取文件内容
    size_t onread = 0;
    size_t totalbytes = fileinfo.filesize;
    char buffer[maxlen];

    for(int i = 0;i < n;i++){
        onread = min(totalbytes,maxlen);
        fin.read(buffer,onread);
        send(socketfd,buffer,onread,0);
        totalbytes -= onread;
    }
    fin.close();
    cout << "send completed\n";
}