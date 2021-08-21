#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<bits/stdc++.h>
#include<fcntl.h>
#include <arpa/inet.h> 

using namespace std;
/*
int temp(){
    int client_socket;
    int port=8000;
    struct sockaddr_in server_address;
    //int addrlen=sizeof(address);
    client_socket=socket(AF_INET,SOCK_STREAM,0);
    if(client_socket==-1){
        printf("Socket creation error %d\n",errno);
        return 0;
    }
    //setsockopt(client_socket,)
    if(inet_pton(AF_INET,"127.0.0.1",&server_address.sin_addr)==-1){
        printf("Invalid Address:%d\n",errno);
    }
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(port);
    int addrlen=sizeof(server_address);

    int socket=connect(client_socket,(struct sockaddr*)&server_address,addrlen);
    if(socket==-1){
        printf("Connect error:%d\n",errno);
        return 0;
    }
    printf("Connected\n");
    string to_send="get_chunk part0 1";
    cout<<to_send<<endl;
    string dest="temp.txt";
    int chunk_size=512*1024;
    int sub_chunk_size=16*1024;
    char buffer[sub_chunk_size]={0};
    int fd=open(dest.c_str(),O_TRUNC|O_CREAT|O_WRONLY,S_IRWXU|S_IRWXG);
    lseek(fd,1*chunk_size,0);
    cout<<write(client_socket,to_send.c_str(),to_send.length());
    while(1){
        int valread=read(client_socket,buffer,sub_chunk_size);
        //cout<<valread<<endl;
        write(fd,buffer,valread);
        memset(buffer,'\0',sub_chunk_size);
        //printf("%d %s\n",strlen(buffer),buffer);
        //printf("%d %d %s\n",getpid(),valread,buffer);
        if(valread<1024) break;
    }
}
*/
int main(){

    for(int i=0;i<8;i++)
    {
        int client_socket;
        //int port=22441;
        int port=8000;
        struct sockaddr_in server_address;
        //int addrlen=sizeof(address);
        client_socket=socket(AF_INET,SOCK_STREAM,0);
        if(client_socket==-1){
            printf("Socket creation error %d\n",errno);
            return 0;
        }
        //setsockopt(client_socket,)
        if(inet_pton(AF_INET,"127.0.0.1",&server_address.sin_addr)==-1){
            printf("Invalid Address:%d\n",errno);
        }
        server_address.sin_family=AF_INET;
        server_address.sin_port=htons(port);
        int addrlen=sizeof(server_address);

        int socket=connect(client_socket,(struct sockaddr*)&server_address,addrlen);
        if(socket==-1){
            printf("Connect error:%d\n",errno);
            return 0;
        }
        printf("Connected\n");
        string to_send="get_chunk video3.mp "+to_string(i);
        string dest="temp.mp4";
        int chunk_size=512*1024;
        int sub_chunk_size=16*1024;
        char buffer[sub_chunk_size]={0};
        int fd=open(dest.c_str(),O_CREAT|O_WRONLY,S_IRWXU|S_IRWXG);
        lseek(fd,i*chunk_size,0);
        write(client_socket,to_send.c_str(),to_send.length());
        while(1){
            int valread=read(client_socket,buffer,sub_chunk_size);
            //cout<<valread<<endl;
            write(fd,buffer,valread);
            memset(buffer,'\0',sub_chunk_size);
            //printf("%d %s\n",strlen(buffer),buffer);
            //printf("%d %d %s\n",getpid(),valread,buffer);
            if(valread<1024) break;
        }
        cout<<i<<endl;
        close(fd);
        close(client_socket);
    }
    //temp();
    return 0;
}