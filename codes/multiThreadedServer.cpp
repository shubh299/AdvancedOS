#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<bits/stdc++.h>
#include<fcntl.h>
#include<pthread.h>

using namespace std;

void * acceptCon(void * client_socket1){
    int client_socket=*(int*)client_socket1;
    //struct sockaddr_in client=*client1;
    free(client_socket1);
    //free(client1);
    char buffer[1024]={0};
    //printf("Accepted from %lld:%d\n",client.sin_addr,client.sin_port);
    int valread=read(client_socket,buffer,1024);
    printf("%s\n",buffer);
    char *token=strtok(buffer," ");
    vector<char*> command;
    while(token!=NULL){
        command.push_back(token);
        token=strtok(NULL," ");
    }
    int chunk_no=atoi(command[2]);

    //string to_send="Hi from server";
    char actual_path[1024]={0};
    /*if(realpath(buffer,actual_path)==NULL){
        printf("Path not found\n");
        close(client_socket);
        return NULL;
    }*/
    printf("actual path:%s %s\n",buffer,actual_path);
    int fd=open(command[1],O_RDONLY);
    printf("%d",fd);
    if(fd<0){
        printf("File open error %d",errno);
        close(client_socket);
        return NULL;
    }
    int bytes_read;
    int chunk_size=512*1024;
    printf("reading file\n");
    int sub_chunk_size=16*1024;
    char send_buffer[sub_chunk_size]={0};
        memset(send_buffer,'\0',sub_chunk_size);
    int count=0;
    cout<<chunk_no<<endl;
    lseek(fd,chunk_no*chunk_size,SEEK_SET);
    while((bytes_read=read(fd,send_buffer,sub_chunk_size))>0){
        //bytes_read=read(fd,send_buffer,sub_chunk_size);
        //printf("%d %d\n",count,bytes_read);
        write(client_socket,send_buffer,strlen(send_buffer));
        memset(send_buffer,'\0',sub_chunk_size);
        count++;
        if(bytes_read<1024 || count==32) {
            cout<<"quit"<<endl;
            //strcpy(send_buffer,"quit");
            break;
        }
    }
    printf("FIle sent, con closed\n");
    close(fd);
    close(client_socket);
    return NULL;
}

int main(){
    int server_socket;
    int port=8000;
    struct sockaddr_in address;
    //int addrlen=sizeof(address);
    char buffer[1024]={0};
    int opt=1;
    
    server_socket=socket(AF_INET,SOCK_STREAM,0);
    if(server_socket==-1){
        printf("Socket creation error %d\n",errno);
        return 0;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    //setsockopt(server_socket,)
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_family=AF_INET;
    address.sin_port=htons(port);
    int addrlen=sizeof(address);
    if(bind(server_socket,(const struct sockaddr *)&address,addrlen)==-1){
        printf("bind error:%d\n",errno);
        return 0;
    }
    if(listen(server_socket,4)==-1){
        printf("Listen error:%d\n",errno);
        return 0;
    }

    printf("Listening\n");
    int n=0;
    while(1){
        cout<<"n"<<n<<endl;
        n++;
        struct sockaddr_in client;
        socklen_t client_len=sizeof(client);
        int client_socket=accept(server_socket,(struct sockaddr*)&client,&client_len);
        if(client_socket==-1){
            printf("Accept error:%d\n",errno);
            return 0;
        }
        pthread_t t;
        struct sockaddr_in *temp=(struct sockaddr_in *)malloc(sizeof(client));
        *temp=client;
        int *client_soc=(int*)malloc(sizeof(int));
        *client_soc=client_socket;
        cout<<"Sock"<<client_socket<<endl;
        pthread_create(&t,NULL,acceptCon,client_soc);
        cout<<"Thread Created"<<endl;
        //acceptCon(client_socket,client);
    }
    close(server_socket);
    return 0;
}