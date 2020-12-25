using namespace std;
void file_copy(struct stat inode,const char *source,const char *dest_file){
    char c[10]={0};
    int source_fd=open(source,O_RDONLY);
    int dest_fd=open(dest_file,O_CREAT|O_WRONLY);
    //cout<<dest_file;
    if(source_fd==-1 || dest_fd==-1){
        close(source_fd);
        close(dest_fd);
        return;
    }
    int num_read=0;
    while((num_read=read(source_fd,c,10))!=0){
        write(dest_fd,c,num_read);
        memset(c,0,10);
    }
    close(source_fd);
    close(dest_fd);
    chmod(dest_file,inode.st_mode);
    chown(dest_file,inode.st_uid,inode.st_gid);
}

void dir_copy(struct stat inode,const char *source,const string dest){
    //cout<<dest<<endl;
    struct stat inode1;
    DIR *d=opendir(source);
    if(d==NULL){
        return;
    }
    mkdir(dest.c_str(),inode.st_mode);
    chown(dest.c_str(),inode.st_uid,inode.st_gid);
    dirent *dirent=readdir(d);
    //cout<<"Dest:"<<dest<<endl;
    //cout<<"Source:"<<source<<endl;
    while(dirent!=NULL){
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0){
            dirent=readdir(d);
            continue;
        }
        string temp;
        temp.assign(source);
        temp=temp+"/"+string(dirent->d_name);
        //cout<<temp<<endl;
        lstat(temp.c_str(),&inode1);
        string temp2=dest+"/"+string(dirent->d_name);
        if((S_IFDIR&inode1.st_mode)==S_IFDIR){
            dir_copy(inode1,temp.c_str(),temp2);
        }
        else{
            //cout<<"andlas"<<endl;
            file_copy(inode1,temp.c_str(),temp2.c_str());
        }
        dirent=readdir(d);
    }
    closedir(d);
}

void copy_files(vector<char*> command){
    string destination;
    int last=command.size()-1;
    destination.assign(command[last]);
    //strcpy(destination,command[last]);
    char* dest_path;
    string temp;
    if(destination[0]=='~'){
        temp=string(home);
        //cout<<"Home"<<temp<<endl;
        string temp1=destination;
        destination=temp;
        for(int i=1;i<temp1.length();i++){
            destination=destination+temp1[i];
            //cout<<i<<destination<<endl;
        }
    }
    for(int i=1;i<last;i++){
        char *c;
        struct stat inode;
        lstat(command[i],&inode);
        temp.assign(command[i]);
        auto destination1=destination+"/"+temp;
        //cout<<destination1<<endl;
        const char *dest_file=destination1.c_str();

        //cout<<"\n\n\n"<<destination1<<endl;
        if((S_IFDIR&inode.st_mode)==S_IFDIR){
            //cout<<"eurhwieubr"<<endl;
            dir_copy(inode,command[i],destination1);
        }
        else{
            //cout<<"eurhwieubr"<<endl;
            //cout<<i<<command[i]<<endl;
            file_copy(inode,command[i],dest_file);
            //cout<<dest_file;
        }
    }
}

void move_recursive(struct stat inode,const char *source,const string dest){
    struct stat inode1;
    DIR *d=opendir(source);
    if(d==NULL){
        return;
    }
    mkdir(dest.c_str(),inode.st_mode);
    chown(dest.c_str(),inode.st_uid,inode.st_gid);
    dirent *dirent=readdir(d);
    while(dirent!=NULL){
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0){
            dirent=readdir(d);
            continue;
        }
        string temp;
        temp.assign(source);
        temp=temp+"/"+string(dirent->d_name);
        lstat(temp.c_str(),&inode1);
        string temp2=dest+"/"+string(dirent->d_name);
        if((S_IFDIR&inode1.st_mode)==S_IFDIR){
            move_recursive(inode1,temp.c_str(),temp2);
            remove(temp.c_str());
        }
        else{
            //cout<<"andlas"<<endl;
            file_copy(inode1,temp.c_str(),temp2.c_str());
            remove(temp.c_str());
        }
        dirent=readdir(d);
    }
    closedir(d);
}

void move_files(vector<char*> command){
    string destination;
    int last=command.size()-1;
    destination.assign(command[last]);
    //strcpy(destination,command[last]);
    char* dest_path;
    string temp;
    if(destination[0]=='~'){
        temp=string(home);
        //cout<<"Home"<<temp<<endl;
        string temp1=destination;
        destination=temp;
        for(int i=1;i<temp1.length();i++){
            destination=destination+temp1[i];
            //cout<<i<<destination<<endl;
        }
    }
    for(int i=1;i<last;i++){
        char *c;
        struct stat inode;
        lstat(command[i],&inode);
        temp.assign(command[i]);
        auto destination1=destination+"/"+temp;
        //cout<<destination1<<endl;
        const char *dest_file=destination1.c_str();

        //cout<<"\n\n\n"<<destination1<<endl;
        if((S_IFDIR&inode.st_mode)==S_IFDIR){
            //cout<<"eurhwieubr"<<endl;
            move_recursive(inode,command[i],destination1);
            remove(command[i]);
        }
        else{
            //cout<<"eurhwieubr"<<endl;
            //cout<<i<<command[i]<<endl;
            file_copy(inode,command[i],dest_file);
            remove(command[i]);
        }
    }

}

void rename_file(vector<char*> command){
    if(rename(command[1],command[2])){
        write(STDERR_FILENO,"RENAME ERROR",12);
    }
}

void create_file(vector<char*> command){
    string path=string(command[2])+"/"+string(command[1]);
    if(path[0]=='~'){
        string temp=path;
        path=get_current_dir_name();
        for(int i=1;i<temp.length();i++){
            path+=temp[i];
        }
    }
    if(creat(path.c_str(),S_IRWXU|S_IRWXG|S_IRWXO)==-1){
        write(STDERR_FILENO,"File create Error",17);
    }
}

void create_dir(vector<char*> command){
    string path=string(command[2])+"/"+string(command[1]);
    if(path[0]=='~'){
        string temp=path;
        path=get_current_dir_name();
        for(int i=1;i<temp.length();i++){
            path+=temp[i];
        }
    }
    if(mkdir(path.c_str(),S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)==-1){
        write(STDERR_FILENO,"Dir create Error",16);
    }
}

void delete_file(vector<char*> command){
    string temp(command[1]);
    string path=home;
    for(int i=1;i<temp.length();i++){
        path+=temp[i];
    }
    //write(STDOUT_FILENO,path.c_str(),path.size());
    if(remove(path.c_str())){
        write(STDERR_FILENO,"Error deleting file",19);
    };
}

void recursive_delete(string path){
    DIR *d=opendir(path.c_str());
    dirent *dirent=readdir(d);
    struct stat inode;
    while(dirent!=NULL){
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0){
            dirent=readdir(d);
            continue;
        }
        string new_path=path+"/"+dirent->d_name;
        lstat(new_path.c_str(),&inode);
        if((S_IFDIR&inode.st_mode)==S_IFDIR){
            recursive_delete(new_path);
            remove(new_path.c_str());
        }
        else{
            remove(new_path.c_str());
        }
        dirent=readdir(d);
    }
    closedir(d);
}

void delete_dir(vector<char*> command){
    string temp(command[1]);
    string path=home;
    for(int i=1;i<temp.length();i++){
        path+=temp[i];
    }
    DIR *d=opendir(path.c_str());
    dirent *dirent=readdir(d);
    struct stat inode;
    
    while(dirent!=NULL){
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0){
            dirent=readdir(d);
            continue;
        }
        string new_path=path+"/"+dirent->d_name;
        lstat(new_path.c_str(),&inode);
        if((S_IFDIR&inode.st_mode)==S_IFDIR){
            recursive_delete(new_path);
            remove(new_path.c_str());
        }
        else{
            remove(new_path.c_str());
        }
        dirent=readdir(d);
    }
    closedir(d);
    remove(path.c_str());
}

void go_to(vector<char*> command){
    string temp=command[1];
    string path=home;
    for(int i=1;i<temp.length();i++){
        path+=temp[i];
    }
    back_stack.push(get_current_dir_name());
    clearForwardStack();
    if(chdir(path.c_str())){
        write(STDOUT_FILENO,"Location not found",18);
    }
}

bool recursive_search(string dir_path,char *file_name){
    DIR *d=opendir(dir_path.c_str());
    struct dirent* dirent=readdir(d);
    //write(STDOUT_FILENO,dir_path.c_str(),dir_path.length());
    //write(STDOUT_FILENO,"\n",2);
    while (dirent!=NULL){
        string dir_name=dirent->d_name;
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0){
            dirent=readdir(d);
            continue;
        }
        if(strcmp(dirent->d_name,file_name)==0){
            closedir(d);
            return true;
        }
        struct stat inode;
        string path=dir_path+"/"+string(dirent->d_name);
        lstat(path.c_str(),&inode);
        if((S_IFDIR&inode.st_mode)==S_IFDIR){
            if(recursive_search(path,file_name)){
                closedir(d);
                return true;
            }
        }
        dirent=readdir(d);
    }
    closedir(d);
    return false;
}

bool search(char* file_name){
    DIR *d=opendir(get_current_dir_name());
    struct dirent* dirent=readdir(d);
    while(dirent!=NULL){
        if(strcmp(dirent->d_name,".")==0 || strcmp(dirent->d_name,"..")==0){
            dirent=readdir(d);
            continue;
        }
        if(strcmp(dirent->d_name,file_name)==0){
            closedir(d);
            return true;
        }
        struct stat inode;
        lstat(dirent->d_name,&inode);
        if((S_IFDIR&inode.st_mode)==S_IFDIR){
            string path=string(get_current_dir_name())+"/"+string(dirent->d_name);
            //write(STDOUT_FILENO,path.c_str(),path.length());
            if(recursive_search(path,file_name)){
                closedir(d);
                return true;
            }
        }
        dirent=readdir(d);
    }
    closedir(d);
    return false;
}

//returns 1 on wrong command,2 on search true,3 on search false, 0 otherwise
int executeCommand(string command){
    int len=command.length();
    vector<char*> commandSplit;
    char temp[len];
    strcpy(temp,command.c_str());
    char *token=strtok(temp," ");
    while (token!=NULL){
        commandSplit.push_back(token);
        token=strtok(NULL," ");
    }
    if(strcmp(commandSplit[0],"copy")==0){
        if(commandSplit.size()<3) return 1;
        copy_files(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"move")==0){
        if(commandSplit.size()<3) return 1;
        move_files(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"rename")==0){
        if(commandSplit.size()!=3) return 1;
        rename_file(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"create_file")==0){
        if(commandSplit.size()!=3) return 1;
        create_file(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"create_dir")==0){
        if(commandSplit.size()!=3) return 1;
        create_dir(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"delete_file")==0){
        if(commandSplit.size()!=2) return 1;
        delete_file(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"delete_dir")==0){
        if(commandSplit.size()!=2) return 1;
        delete_dir(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"goto")==0){
        if(commandSplit.size()!=2) return 1;
        go_to(commandSplit);
        return 0;
    }
    if(strcmp(commandSplit[0],"search")==0){
        if(commandSplit.size()!=2) return 1;
        if(search(commandSplit[1])){
            return 2;
        }
        return 3;
    }
    return 1;
}

void commandMode(){
    char c[3]={0};
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&window);
    string to_bottom="\033["+to_string(window.ws_row)+";0H";
    write(STDOUT_FILENO,to_bottom.c_str(),8);
    //write(STDOUT_FILENO,"\033[K",3);
    write(STDOUT_FILENO,":",1);
    string command;
    while(1){
        memset(c,0,3);
        read(STDIN_FILENO,c,3);
        if(c[0]=='\033'){
            if(c[2]==0){
                to_bottom="\033["+to_string(window.ws_row)+";0H";
                write(STDOUT_FILENO,to_bottom.c_str(),8);
                write(STDOUT_FILENO,"\033[2K",5);
                to_bottom="\033["+to_string(pointer+1)+";0H";
                write(STDOUT_FILENO,to_bottom.c_str(),8);
                break;
            }
            else{
                continue;
            }
        }
        if(c[0]==127){
            if(command.length()>0){
                command.pop_back();
                string to_bottom="\033["+to_string(window.ws_row)+";0H";
                write(STDOUT_FILENO,to_bottom.c_str(),8);
                write(STDOUT_FILENO,"\033[2K",8);
                write(STDOUT_FILENO,":",1);
                write(STDOUT_FILENO,command.c_str(),command.length());
            }
            continue;
        }
        if(c[0]!=10){
            write(STDOUT_FILENO,c,1);
            command.push_back(c[0]);
        }
        if(c[0]==10){
            string to_bottom="\033["+to_string(window.ws_row)+";0H";
            write(STDOUT_FILENO,to_bottom.c_str(),8);
            write(STDOUT_FILENO,"\033[K",3);
            write(STDOUT_FILENO,":",1);
            if(!command.empty()){
                int status=executeCommand(command);
                /*string to_bottom="\033["+to_string(window.ws_row)+";0H";
                write(STDOUT_FILENO,to_bottom.c_str(),8);
                write(STDOUT_FILENO,"\033[K",3);
                write(STDOUT_FILENO,":",1);*/
                command.clear();
                if(status!=0){
                    if(status==1){
                        write(STDOUT_FILENO,"Wrong command",13);
                        //memset(c,0,3);
                    }
                    if(status==2){
                        write(STDOUT_FILENO,"File Found",10);
                    }
                    if(status==3){
                        write(STDOUT_FILENO,"File Not Found",14);
                    }
                    write(STDOUT_FILENO,",Enter to continue",18);
                }
            }
        }
    }

}
