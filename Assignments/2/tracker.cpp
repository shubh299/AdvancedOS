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

using namespace std;

struct sock{
    string ip_addr;
    int port;
    sock(string addr,int p){
        ip_addr=addr;
        port=p;
    }
};

typedef struct sock sock;

vector<sock> tracker;

struct FileInfo{
    string name;
    string hash;
    int noOfChunks;
    set<string> seeders;
};

typedef FileInfo fileinfo;

struct socketDetails{
    int sockFD;
    struct sockaddr_in sock;
};

typedef struct socketDetails socketDetails;

struct group{
    string grpName;
    string grpLeader;
    map<string,fileinfo> files;
};

typedef struct group group;

struct peer{
    string ip;
    string port;
    string id;
    string password;
    set<string> grp;
    bool isActive=false;
};

typedef struct peer peer;

map<string,peer> peers;
map<string,group> groups;

//return -1 if user already exists else return 1
int createUser(vector<char*> command){
    if(peers.find(command[1])!=peers.end()){
        return -1;
    }
    peer p1;
    p1.id=string(command[1]);
    p1.password=string(command[2]);
    peers.insert(make_pair(p1.id,p1));
    return 1;
}

//return values:-3 wrong pass, -1 if user does not exist, -2 if user is already logged in, 1 on success
int login(vector<char*> command){
    auto p=peers.find(command[1]);
    if(p==peers.end()) return -1;
    if(p->second.isActive) return -2;
    if(p->second.password!=string(command[2])) return -3;
    p->second.isActive=true;
    string peerDetails=command[3];
    vector<string> details;
    string token;
    stringstream ss(peerDetails);
    while(getline(ss,token,':')){
        details.push_back(token);
    }
    p->second.ip=details[0];
    p->second.port=details[1];
    return 1;
}

//return values: retrun 1 on success, -1 on grp already exists
int createGroup(vector<char*> command){
    if(groups.find(command[1])!=groups.end()){
        return -1;
    }
    if(command.size()!=3 || peers.find(command[2])==peers.end()){
        return -2;
    }
    group g;
    g.grpLeader=command[2];
    g.grpName=command[1];
    groups.insert(make_pair(g.grpName,g));
    auto user=peers.find(command[2])->second.grp.insert(command[1]);
    return 1;
}

//return -1 on error else ip:port of grp leader
string joinGroup(vector<char*> command){
    auto g=groups.find(command[1]);
    if(g==groups.end()) return "-1";
    string leader=g->second.grpLeader;
    auto p=peers[leader];
    return (p.ip+":"+p.port);
}

void leaveGroup(vector<char*> command){
    peer p=peers[command[2]];
    string grp=command[1];
    if(p.grp.find(grp)!=p.grp.end()){
        p.grp.erase(grp);
    }
}

int addToGroup(vector<char*> command){
    peers[command[2]].grp.insert(command[1]);
    return 1;
}

string listGroups(){
    string list="";
    for(auto it=groups.begin();it!=groups.end();it++){
        list+=(it->second.grpName+" "+it->second.grpLeader+"\n");
    }
    return list;
}

string listFiles(vector<char*> command){
    string result="";
    auto grp=groups.find(command[1]);
    if(grp==groups.end()){
        return "-1";
    }
    for(auto it=grp->second.files.begin();it!=grp->second.files.end();it++){
        result+=(it->first+"\n");
    }
    return result;
}

string uploadFile(vector<char*> command){
    auto user=peers.find(command[3]);
    if(user==peers.end()){
        return "-2";
    }
    else{
        if(user->second.grp.find(command[2])==user->second.grp.end()){
            return "-3";
        }
    }
    auto grp=groups.find(command[2]);
    if(grp==groups.end()){
        return "-1";
    }
    //cout<<"Inside upload:"<<endl;
    //cout<<command[4]<<endl;
    auto filesMap=(grp->second.files);
    auto file=(filesMap.find(command[1]));
    if(file==filesMap.end()){
        fileinfo newFileInfo;
        newFileInfo.name=command[1];
        newFileInfo.hash=command[5];
        newFileInfo.noOfChunks=atoi(command[4]);
        newFileInfo.seeders.insert(command[3]);
        grp->second.files.insert(make_pair(command[1],newFileInfo));
    }
    else{
        grp->second.files[command[1]].seeders.insert(command[3]);
    }
    return "1";
}

string downloadFile(vector<char*> command){
    auto user=peers.find(command[3]);
    if(user==peers.end()){
        return "-2";
    }
    else{
        if(user->second.grp.find(command[1])==user->second.grp.end()){
            return "-3";
        }
    }
    auto grp=groups.find(command[1]);
    if(grp==groups.end()){
        return "-1";
    }
    auto filesMap=grp->second.files;
    auto file1=filesMap.find(command[2]);
    string res="";
    if(file1==filesMap.end()){
        return "-4";
    }
    else{
        res+=(file1->second.name+";");
        res+=(to_string(file1->second.noOfChunks)+";");
        //res+=(temp.name+";"+temp.hash+";");
        for(auto it=file1->second.seeders.begin();it!=file1->second.seeders.end();it++){
            auto p=peers[*it];
            if(p.isActive){
                res+=(p.ip+":"+p.port+" ");
            }
        }
        res+=(";"+file1->second.hash);
    }
    return res;
}

void logout(vector<char*> command){
    auto p=peers.find(command[1]);
    if(p!=peers.end()){
        p->second.isActive=false;
    }
}

void stopSharing(vector<char*> command){
    auto user=peers.find(command[3]);
    if(user==peers.end()){
        return;
    }
    else{
        if(user->second.grp.find(command[1])==user->second.grp.end()){
            return;
        }
    }
    auto grp=groups.find(command[1]);
    if(grp==groups.end()){
        return;
    }
    auto filesMap=grp->second.files;
    auto file=filesMap.find(command[2]);
    
    if(file==filesMap.end()){
        return;
    }
    else{
        file->second.seeders.erase(command[3]);
    }
    return;
}

void * acceptCon(void * client_socket1){
    socketDetails client=*(socketDetails*)client_socket1;
    free(client_socket1);
    char buffer[1024]={0};
    int valread=read(client.sockFD,buffer,1024);
    printf("%s\n",buffer);
    vector<char*> command;
    char *token=strtok(buffer," ");
    while(token!=NULL){
        command.push_back(token);
        token=strtok(NULL," ");
    }
    int status;
    if(strcmp(command[0],"create_user")==0){
        status=createUser(command);
        string stat=to_string(status);
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"login")==0){
        status=login(command);
        string stat=to_string(status);
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"create_group")==0){
        status=createGroup(command);
        string stat=to_string(status);
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"join_group")==0){
        string stat=joinGroup(command);
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"leave_group")==0){
        leaveGroup(command);
        string stat="1";
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"accept_request")==0){
        status=addToGroup(command);
        string stat=to_string(status);
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"list_groups")==0){
        string grps=listGroups();
        if(grps==""){
            string stat="No groups created\n";
            write(client.sockFD,stat.c_str(),stat.length());
        }
        else{
            write(client.sockFD,grps.c_str(),grps.length());
        }
    }
    if(strcmp(command[0],"list_files")==0){
        string files=listFiles(command);
        if(files==""){
            string stat="No files in this group\n";
            write(client.sockFD,stat.c_str(),stat.length());
        }
        else{
            write(client.sockFD,files.c_str(),files.length());
        }
    }
    if(strcmp(command[0],"upload_file")==0){
        string stat=uploadFile(command);
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"download_file")==0){
        string files=downloadFile(command);
        if(files==""){
            string stat="File not found\n";
            write(client.sockFD,stat.c_str(),stat.length());
        }
        else{
            write(client.sockFD,files.c_str(),files.length());
        }
    }
    if(strcmp(command[0],"logout")==0){
        logout(command);
        string stat="1";
        write(client.sockFD,stat.c_str(),stat.length());
    }
    if(strcmp(command[0],"stop_share")==0){
        stopSharing(command);
        string stat="1";
        write(client.sockFD,stat.c_str(),stat.length());
    }
    memset(buffer,0,1024);
    close(client.sockFD);
    return NULL;
}

void * readInput(void * t){
    while(1){
        string s;
        getline(cin,s);
        if(s=="quit"){
            exit(1);
        }
    }
}

int main(int argc,char *argv[]){
    if(argc!=3){
        cout<<"Info missing"<<endl;
        return 0;
    }

    FILE *fp=fopen(argv[1],"r");
    int tracker_no=atoi(argv[2]);
    char tracker_details[21];
    while(tracker_no--){
        fscanf(fp,"%s",tracker_details);
    }
    fclose(fp);
    char *token=strtok(tracker_details,":");
    string ip=token;
    token=strtok(NULL,":");
    int port=atoi(token);

    sock this_tracker(ip,port);
    int server_socket;
    struct sockaddr_in address;
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
    inet_pton(AF_INET,this_tracker.ip_addr.c_str(),&address.sin_addr);
    //address.sin_addr.s_addr=INADDR_ANY;
    address.sin_family=AF_INET;
    address.sin_port=htons(this_tracker.port);
    int addrlen=sizeof(address);
    if(bind(server_socket,(const struct sockaddr *)&address,addrlen)==-1){
        printf("bind error:%d\n",errno);
        return 0;
    }
    if(listen(server_socket,1000)==-1){
        printf("Listen error:%d\n",errno);
        return 0;
    }

    pthread_t t2;
    pthread_create(&t2,NULL,readInput,NULL);

    printf("Listening\n");
    //cout<<" "<<address.sin_port<<endl;
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