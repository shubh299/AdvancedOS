#include<stdio.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<bits/stdc++.h>
#include<fcntl.h>
#include<pthread.h>
#include<arpa/inet.h> 
#include<fstream>
#include<iostream>
#include<openssl/sha.h>

using namespace std;

string my_id;

map<string,set<string>> pending_requests;

//template<typename n>
struct fileInfo{
    string path;
    string fileName;
    string hash;
    vector<bool> chunkAvailable;
};

typedef struct fileInfo fileInfo;

struct downloadStatus{
    string fileName;
    string grpName;
    bool downloaded;
    downloadStatus(string f,string g){
        fileName=f;
        grpName=g;
        downloaded=false;
    }
};

typedef struct downloadStatus downloadStatus;

map<string,downloadStatus> downloads;
map<string,fileInfo> files;

struct socketDetails{
    int sockFD;
    struct sockaddr_in sock;
};

typedef struct socketDetails socketDetails;

struct sock{
    string ip_addr;
    int port;
    sock(string addr,int p){
        ip_addr=addr;
        port=p;
    }
};

typedef struct sock sock;

struct downloadData{
    sock peer;
    pair<int,int> chunk;
    string path;
    string file;
};

typedef downloadData downloadData;

vector<sock> tracker;

void * client(void *temp){
    int port=*(int*)temp;
    //free(temp);
    int client_socket;
    client_socket=socket(AF_INET,SOCK_STREAM,0);
    if(client_socket==-1){
        printf("Socket creation error %d\n",errno);
        return 0;
    }
    struct sockaddr_in server_address;
    //int addrlen=sizeof(address);
    char buffer[1024]={0};

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
    string to_send="text.txt";
    write(client_socket,to_send.c_str(),to_send.length());
    while(1){
        int valread=read(client_socket,buffer,1024);
        printf("%d %d %s\n",getpid(),valread,buffer);
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return NULL;
}

void addRequest(vector<char*> command){
    pending_requests[command[1]].insert(command[2]);
}

string getFile(vector<char*> command){
    string bitString="";
    auto f=files[command[1]];
    for(auto it=f.chunkAvailable.begin();it!=f.chunkAvailable.end();it++){
        bitString+=to_string(*it);
    }
    return bitString;
}

int getChunk(vector<char*> command,int client_socket){
    string filePath=files[command[1]].path;
    int chunkNo=atoi(command[2]);
    int chunkSize=512*1024;
    int subChunkSize=16*1024;
    int bytesRead;
    char send_buffer[subChunkSize];
    //int fd=open(command[1],O_RDONLY);
    //ifstream sendFile;
    //sendFile.open(filePath.c_str(),ios::binary);
    int fd=open(filePath.c_str(),O_RDONLY);
    //sendFile.seekg(chunkNo*chunkSize);
    int count=0;
    //cout<<chunkNo<<"\n";
    memset(send_buffer,'\0',subChunkSize);
    lseek(fd,chunkNo*chunkSize,SEEK_SET);
    while((bytesRead=read(fd,send_buffer,subChunkSize))>0){
        write(client_socket,send_buffer,strlen(send_buffer));
        memset(send_buffer,'\0',subChunkSize);
        count++;
        if(bytesRead<1024 || count==32){
            //cout<<"quit sending chunk"<<endl;
            break;
        }
    }
    close(fd);
    /*while(1){
        //cout<<"Written"<<
        char send_buffer[subChunkSize];//=(char*)malloc(subChunkSize*(sizeof(char)));
        sendFile.read(send_buffer,subChunkSize);
        cout<<send_buffer<<endl;
        int val=write(client_socket,send_buffer,strlen(send_buffer));
        //send(client_socket,send_buffer,size,0);
        //send();
        if(val==-1){
            clog<<errno<<endl;
        }
        count++;

        if(count==32) {
            break;
        }
        //cout<<"Test"<<endl;
    }*/
    //sendFile.close();
}

void * acceptCon(void * client_socket1){
    socketDetails client=*(socketDetails*)client_socket1;
    free(client_socket1);
    char buffer[1024]={0};
    int valread=read(client.sockFD,buffer,1024);
    //printf("%s\n",buffer);
    vector<char*> command;
    char *token=strtok(buffer," ");
    while(token!=NULL){
        command.push_back(token);
        token=strtok(NULL," ");
    }
    int status;
    if(strcmp(command[0],"join_request")==0){
        addRequest(command);
        string stat="Received request\n";
        write(client.sockFD,stat.c_str(),stat.length());
    }
    
    if(strcmp(command[0],"get_file")==0){
        string stat=getFile(command);
        write(client.sockFD,stat.c_str(),stat.length());
    }

    if(strcmp(command[0],"get_chunk")==0){
        getChunk(command,client.sockFD);
    }
    //cout<<"Exiting accept"<<endl;
    memset(buffer,0,1024);
    close(client.sockFD);
    return NULL;
}

void * server(void *temp){
    sock server_details=*(sock*)temp;
    //free(temp);
    struct sockaddr_in address;
    int server_socket;

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
        return 0;
        //exit(EXIT_FAILURE); 
    }
    inet_pton(AF_INET,(server_details.ip_addr).c_str(),&address.sin_addr);
    //address.sin_addr.s_addr=INADDR_ANY;
    address.sin_family=AF_INET;
    address.sin_port=htons(server_details.port);
    int addrlen=sizeof(address);
    if(bind(server_socket,(const struct sockaddr *)&address,addrlen)==-1){
        printf("bind error:%d\n",errno);
        return 0;
    }
    if(listen(server_socket,1000)==-1){
        printf("Listen error:%d\n",errno);
        return 0;
    }
    cout<<"Peer started"<<endl;
    while(1){
        struct sockaddr_in client;
        socklen_t client_len=sizeof(client);
        int client_socket=accept(server_socket,(struct sockaddr*)&client,&client_len);
        if(client_socket==-1){
            printf("Accept error:%d\n",errno);
            return 0;
        }
        pthread_t t;
        socketDetails *temp=(socketDetails *)malloc(sizeof(socketDetails));
        (*temp).sock=client;
        (*temp).sockFD=client_socket;
        pthread_create(&t,NULL,acceptCon,temp);
        //acceptCon(client_socket,client);
    }
    close(server_socket);
    return 0;
}

int createConnection(){
    sock tracker1=tracker[0];
    int client_socket;
    //cout<<tracker1.ip_addr<<" "<<tracker1.port<<endl;
    client_socket=socket(AF_INET,SOCK_STREAM,0);
    if(client_socket==-1){
        printf("Socket creation error %d\n",errno);
        return -3;
    }
    struct sockaddr_in server_address;

    if(inet_pton(AF_INET,tracker1.ip_addr.c_str(),&server_address.sin_addr)==-1){
        printf("Invalid Address:%d\n",errno);
        return -2;
    }
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(tracker1.port);
    int addrlen=sizeof(server_address);

    int socket=connect(client_socket,(struct sockaddr*)&server_address,addrlen);
    if(socket==-1){
        printf("Connect error:%d\n",errno);
        return -1;
    }
    return client_socket;
}

int createConnection(sock server){
    sock tracker1=server;
    int client_socket;
    //cout<<tracker1.ip_addr<<" "<<tracker1.port<<endl;
    client_socket=socket(AF_INET,SOCK_STREAM,0);
    if(client_socket==-1){
        printf("Socket creation error %d\n",errno);
        return -3;
    }
    struct sockaddr_in server_address;

    if(inet_pton(AF_INET,tracker1.ip_addr.c_str(),&server_address.sin_addr)==-1){
        printf("Invalid Address:%d\n",errno);
        return -2;
    }
    server_address.sin_family=AF_INET;
    server_address.sin_port=htons(tracker1.port);
    int addrlen=sizeof(server_address);

    int socket=connect(client_socket,(struct sockaddr*)&server_address,addrlen);
    if(socket==-1){
        printf("Connect error:%d\n",errno);
        return -1;
    }
    return client_socket;
}

int createUser(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    char buffer[1024]={0};

    //printf("Connected\n");
    string to_send=string(command[0]);
    for(int i=1;i<command.size();i++){
        to_send+=(" "+string(command[i]));
    }
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return atoi(recv.c_str());
}

int login(vector<char*> command,sock my_server){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    char buffer[1024]={0};

    //printf("Connected\n");
    string to_send=string(command[0]);
    for(int i=1;i<command.size();i++){
        to_send+=(" "+string(command[i]));
    }
    to_send+=(" "+my_server.ip_addr+":"+to_string(my_server.port));
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return atoi(recv.c_str());
}

int createGroup(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    char buffer[1024]={0};
    string to_send=string(command[0]);
    for(int i=1;i<command.size();i++){
        to_send+=(" "+string(command[i]));
    }
    to_send+=(" "+my_id);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return atoi(recv.c_str());
}

string listGroups(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    //printf("Connected\n");
    string to_send=string(command[0]);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return recv;
}

string joinGroup(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return "0";
    }
    string to_send=string(command[0]);
    for(int i=1;i<command.size();i++){
        to_send+=(" "+string(command[i]));
    }
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    if(recv=="-1") return recv;
    stringstream ss(recv);
    string token;
    vector<string> peerDetails;
    while(getline(ss,token,':')){
        peerDetails.push_back(token);
    }
    sock peer(peerDetails[0],atoi(peerDetails[1].c_str()));
    //cout<<recv<<endl;
    int peer_socket=createConnection(peer);
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    to_send="join_request "+string(command[1])+" "+my_id;
    write(peer_socket,to_send.c_str(),to_send.length());
    recv="";
    memset(buffer,'\0',1024);
    while(1){
        int valread=read(peer_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(peer_socket);
    return recv;
}

string acceptRequest(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return "0";
    }
    pending_requests[command[1]].erase(command[2]);
    string to_send="accept_request";
    for(int i=1;i<command.size();i++){
        to_send+=(" "+string(command[i]));
    }
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return recv;
}

int leaveGroup(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return -1;
    }
    //printf("Connected\n");
    string to_send=string(command[0]);
    to_send+=(" "+string(command[1])+" "+my_id);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return atoi(recv.c_str());
}

string listFiles(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    //printf("Connected\n");
    string to_send=string(command[0])+" "+string(command[1]);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return recv;
}

string getHash(char *path,int *chunks){
    string hash="";
    int fd=open(path,S_IRUSR);
    if(fd<0){
        return hash;
    }
    int chunkSize=512*1024;
    unsigned char chunk[chunkSize];
    int numread=read(fd,chunk,chunkSize);
    while(numread==chunkSize){
        unsigned char h[SHA_DIGEST_LENGTH];
        SHA1(chunk,numread,h);
        (*chunks)++;
        hash+=string(reinterpret_cast<char*>(h));
        memset(chunk,'\0',chunkSize);
        numread=read(fd,chunk,chunkSize);
    }
    unsigned char h[SHA_DIGEST_LENGTH];
    SHA1(chunk,numread,h);
    (*chunks)++;
    hash+=string(reinterpret_cast<char*>(h));
    return hash;
}

string getFileName(char *path){
    string path1=string(path);
    stringstream ss(path1);
    string token;
    string fileName;
    while(getline(ss,token,':')){
        fileName=token;
    }
    return fileName;
}

string uploadFile(vector<char*> command){
    int chunks=0;
    string hash=getHash(command[1],&chunks);
    if(hash=="") return "File open error";

    string fileName=getFileName(command[1]);
    
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    //printf("Connected\n");
    string to_send=string(command[0])+" ";
    to_send+=(fileName+" ");
    to_send+=(string(command[2])+" ");
    to_send+=(my_id+" ");
    to_send+=(to_string(chunks)+" ");
    to_send+=(hash);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    if(recv=="1"){
        fileInfo temp;
        temp.fileName=fileName;
        temp.hash=hash;
        temp.path=command[1];
        int numberOfChunks=ceil((double)hash.length()/20);
        temp.chunkAvailable=vector<bool>(numberOfChunks,true);
        files.insert(make_pair(temp.fileName,temp));
    }
    return recv;
}

void * downloadThread(void * downloadDetails){
    downloadData details=*(downloadData*)downloadDetails;
    int chunkStart=details.chunk.first;
    int chunkEnd=details.chunk.second;
    sock peer=details.peer;
    string path=details.path;
    string fileName=details.file;
    
    for(int i=chunkStart;i<chunkEnd-1;i++){
        string to_send="get_chunk "+fileName+" "+to_string(i);
        int chunk_size=512*1024;
        int sub_chunk_size=16*1024;
        char buffer[sub_chunk_size]={0};
        memset(buffer,'\0',sub_chunk_size);
        
        int client_socket=createConnection(peer);
        if(client_socket<0) continue; 
        //ofstream recvFile;
        //cout<<"Downloading:::"<<i<<endl;
        //recvFile.open(path.c_str(),ios::binary);
        int fd=open(path.c_str(),O_CREAT|O_WRONLY,S_IRWXU|S_IRWXG);
        lseek(fd,chunk_size*i,SEEK_SET);
        //recvFile.seekp(chunk_size*i);
        write(client_socket,to_send.c_str(),to_send.length());
        while(1){
            int valread=read(client_socket,buffer,sub_chunk_size);
            write(fd,buffer,valread);
            memset(buffer,'\0',sub_chunk_size);
            if(valread<1024) break;
        }
        //cout<<"ChunkNo."<<i<<endl;
        close(fd);
        close(client_socket);
        /*while(1){
            int valread=read(client_socket,buffer,sub_chunk_size);
            //cout<<valread<<endl;
            //write(fd,buffer,valread);
            recvFile.write(buffer,valread);
            memset(buffer,'\0',sub_chunk_size);
            //printf("%d %s\n",strlen(buffer),buffer);
            //printf("%d %d %s\n",getpid(),valread,buffer);
            if(valread<1024) break;
            //cout<<valread<<endl;
        }*/
        //cout<<endl;
        //recvFile.close();
        //close(client_socket);
    }
    return NULL;
}

string download(vector<char*> command,string recv){
    stringstream ss(recv);
    string token;
    vector<string> tokens;
    while(getline(ss,token,';')){
        tokens.push_back(token);
    }
    string fileName=tokens[0];
    string hash=tokens[3];
    int noOfChunks=atoi(tokens[1].c_str());
    vector<sock> seeders;
    stringstream peersStream(tokens[2]);
    while(getline(peersStream,token,' ')){
        stringstream temp(token);
        vector<string> temp1;
        string s1;
        while(getline(temp,s1,':')){
            temp1.push_back(s1);
        }
        seeders.push_back(sock(temp1[0],atoi(temp1[1].c_str())));
    }
    //cout<<hash<<endl;
    for(auto it=seeders.begin();it!=seeders.end();it++){
        cout<<(*it).ip_addr<<"-"<<(*it).port<<endl;
    }
    vector<int> seedersAvailable;
    //vector<string> seedersChunk;
    for(int i=0;i<seeders.size();i++){
        int peer_fd=createConnection(seeders[i]);
        if(peer_fd<0){
            //cout<<"Failed"<<endl;
            continue;
        }
        //cout<<peer_fd<<"Success"<<endl;
        string to_send="get_file "+fileName;
        write(peer_fd,to_send.c_str(),to_send.length());
        string recv;
        char buffer[1024]={0};
        while(1){
            int valread=read(peer_fd,buffer,1024);
            recv+=buffer;
            memset(buffer,'\0',1024);
            if(valread<1024) break;
        }
        int flag=0;
        for(int j=0;j<noOfChunks;j++){
            if(recv[j]=='0'){
                flag=1;
                break;
            }
        }
        if(flag==0){
            seedersAvailable.push_back(i);
        }
        close(peer_fd);
    }

    if(seedersAvailable.size()==0){
        return "-10";
    }

    downloads.insert(make_pair(fileName,downloadStatus(fileName,command[1])));
    
    fileInfo f;
    f.fileName=fileName;
    f.hash=hash;
    string path=command[3];
    if(path[path.length()-1]!='/'){
        path+='/';
    }
    path+=fileName;
    f.path=path;
    f.chunkAvailable=vector<bool>(noOfChunks,false);
    files.insert(make_pair(fileName,f));
    
    string upload="upload_file "+path+" "+command[3];
    char temp[upload.length()];
    strcpy(temp,upload.c_str());
    char *tok=strtok(temp," ");
    vector<char*> t;
    while(tok!=NULL){
        t.push_back(tok);
        tok=strtok(NULL," ");
    }
    uploadFile(t);

    int chunkCompleted=0;
    int chunksPerSeeder=noOfChunks/seedersAvailable.size();
    for(int i=0;i<seedersAvailable.size();i++){
        downloadData *newChunk=(downloadData*)malloc(sizeof(downloadData));
        if(i==seedersAvailable.size()-1)
            newChunk->chunk=make_pair(chunkCompleted,noOfChunks);
        else{
            newChunk->chunk=make_pair(chunkCompleted,chunkCompleted+chunksPerSeeder);
        }
        chunkCompleted+=chunksPerSeeder;
        newChunk->peer=seeders[seedersAvailable[i]];
        newChunk->path=path;
        newChunk->file=fileName;
        pthread_t t;
        pthread_create(&t,NULL,downloadThread,newChunk);
    }

    return "1";
}

string downloadFile(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return 0;
    }
    //printf("Connected\n");
    string to_send=(string(command[0])+" ");
    to_send+=(string(command[1])+" ");
    to_send+=(string(command[2])+" ");
    to_send+=(my_id);
    string dest_path=string(command[3]);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    if(recv=="-1" || recv=="-2" || recv=="-3" || recv=="-4")
        return recv;
    recv=download(command,recv);
    return recv;   
}

string logout(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return "-1";
    }
    //printf("Connected\n");
    string to_send=("logout "+my_id);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return recv;
}

string stopSharing(vector<char*> command){
    int client_socket=createConnection();
    if(client_socket<0){
        cout<<"Connection Error"<<endl;
        return "-1";
    }
    //printf("Connected\n");
    string to_send=string(command[0]);
    to_send+=(" "+string(command[1]));
    to_send+=(" "+string(command[2]));
    to_send+=(" "+my_id);
    write(client_socket,to_send.c_str(),to_send.length());
    string recv;
    char buffer[1024]={0};
    while(1){
        int valread=read(client_socket,buffer,1024);
        //printf("%s\n",buffer);
        recv+=buffer;
        memset(buffer,'\0',1024);
        if(valread<1024) break;
    }
    close(client_socket);
    return recv;
}

void showDownloads(){
    for(auto it=downloads.begin();it!=downloads.end();it++){
        if(it->second.downloaded){
            cout<<"[C] ";
        }
        else{
            int flag=1;
            auto available=files[it->second.fileName].chunkAvailable;
            for(auto it=available.begin();it!=available.end();it++){
                if(*it){
                    continue;
                }
                else{
                    flag=0;
                    break;
                }
            }
            if(flag==0)
                cout<<"[D] ";
            else{
                it->second.downloaded=true;
                cout<<"[C] ";
            }
        }
        cout<<it->second.grpName<<" ";
        cout<<it->second.fileName<<endl;
    }
}

void commandExecute(string input_command,sock my_server){ 
    vector<char*> command;
    int len=input_command.length();
    char temp[len];
    strcpy(temp,input_command.c_str());
    char *token=strtok(temp," ");
    while(token!=NULL){
        command.push_back(token);
        token=strtok(NULL," ");
    }
    int status;
    if(strcmp(command[0],"create_user")==0){
        if(command.size()!=3){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        status=createUser(command);
        if(status==-1){
            cout<<"User already exists"<<endl;
        }
        if(status==1){
            cout<<"User created"<<endl;
        }
        return;
    }
    
    if(strcmp(command[0],"login")==0){
        if(command.size()!=3){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        status=login(command,my_server);
        if(status==-1){
            cout<<"User does not exist"<<endl;
        }
        if(status==1){
            cout<<"Logged in"<<endl;
            my_id=command[1];
        }
        if(status==-2){
            cout<<"User already logged in"<<endl;
        }
        if(status==-3){
            cout<<"Wrong password"<<endl;
        }
        return;
    }
    
    if(strcmp(command[0],"create_group")==0){
        if(command.size()!=2){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        status=createGroup(command);
        if(status==-2){
            cout<<"User not found"<<endl;
        }
        if(status==-1){
            cout<<"Group already exists"<<endl;
        }
        if(status==1){
            cout<<"Group created"<<endl;
        }
        return;
    }
    
    if(strcmp(command[0],"list_groups")==0){
        if(command.size()!=1){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=listGroups(command);
        cout<<stat;
        return;
    }

    if(strcmp(command[0],"join_group")==0){
        if(command.size()!=2){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=joinGroup(command);
        //cout<<stat<<endl;
        return;
    }
    
    if(strcmp(command[0],"list_requests")==0){
        if(command.size()!=2){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        auto req=pending_requests[command[1]];
        for(auto it=req.begin();it!=req.end();it++){
            //string grpName=it->first;
            cout<<(*it)<<endl;
        }
        return;
    }
    
    if(strcmp(command[0],"accept_request")==0){
        if(command.size()!=3){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=acceptRequest(command);
        if(stat=="1"){
            pending_requests[command[1]].erase(command[2]);
        }
        return;
    }
    
    if(strcmp(command[0],"leave_group")==0){
        if(command.size()!=2){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        status=leaveGroup(command);
        if(status==1){
            cout<<"Left";
        }
        return;
    }
    
    if(strcmp(command[0],"list_files")==0){
        if(command.size()!=2){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=listFiles(command);
        cout<<stat<<endl;
        return;
    }
    
    if(strcmp(command[0],"upload_file")==0){
        if(command.size()!=3){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=uploadFile(command);
        //cout<<"Status:"<<stat<<endl;
        if(stat=="1")
            cout<<"Successful upload"<<endl;
        if(stat=="-3")
            cout<<"User not in my group"<<endl;
        if(stat=="-1")
            cout<<"Group not found"<<endl;
        return;
    }
    
    if(strcmp(command[0],"download_file")==0){
        if(command.size()!=4){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=downloadFile(command);
        //cout<<stat<<endl;
        if(stat=="1")
            cout<<"Downloading"<<endl;
        if(stat=="-10")
            cout<<"Chunks missing from seeders"<<endl;
        if(stat=="-3")
            cout<<"User not in my group"<<endl;
        if(stat=="-1")
            cout<<"Group not found"<<endl;
        if(stat=="-4")
            cout<<"File not found";
        return;
    }
    
    if(strcmp(command[0],"logout")==0){ 
        if(command.size()!=1){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=logout(command);
        if(stat=="1")
            cout<<"Logged out"<<endl;
        return;
    }
    
    if(strcmp(command[0],"stop_share")==0){
        if(command.size()!=3){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        string stat=stopSharing(command);
        if(stat=="1")
            cout<<"Stopped sharing"<<endl;
        return;
    }
    
    if(strcmp(command[0],"show_downloads")==0){
        if(command.size()!=1){
            //cout<<command.size()<<endl;
            cout<<"Invalid number of arguments"<<endl;
            return;
        }
        showDownloads();
        return;
    }

    cout<<"Invalid command"<<endl;
    return;
}

int main(int argc,char *argv[]){
    pthread_t t1,t2;
    if(argc!=3){
        printf("No IP/Port or tracker info\n");
        return 0;
    }

    FILE *fp=fopen(argv[2],"r");
    char tracker_details[21];
    fscanf(fp,"%s",tracker_details);
    fclose(fp);
    char *token=strtok(tracker_details,":");
    string t_ip=token;
    token=strtok(NULL,":");
    int t_port=atoi(token);
    sock tracker1(t_ip,t_port);
    tracker.push_back(tracker1);

    token=strtok(argv[1],":");
    string my_ip=token;
    token=strtok(NULL,":");
    int my_port=atoi(token);
    sock my_server(my_ip,my_port);
    
    pthread_create(&t1,NULL,server,&my_server);
    while(1){
        string input_command;
        getline(cin,input_command);
        commandExecute(input_command,my_server);
    }
    cout<<"exiting main while"<<endl;
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
}