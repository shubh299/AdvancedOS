//updates display on terminal
void updateView(int start){
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&window);
    int terminalLines=window.ws_row;
    if(dirList.size()<terminalLines){
        for(int i=0;i<dirList.size();i++){
            dirList[i].print();
            printf("\n");
        }
        viewTop=0;
        viewEnd=dirList.size()-1;
        pointer=dirList.size();
        //printf("%d",pointer);
    }
    else{
        for(int i=start;i<start+terminalLines;i++){
            dirList[i].print();
            printf("\n");
        }
        viewTop=start;
        viewEnd=start+terminalLines-1;
        pointer=start+terminalLines;
    }
}

//get list of content in directory and add to dirList vector
void listDir(char *dirName){
    printf("\033[H\033[J");
    chdir(dirName);
    DIR* dir=opendir(".");
    if(dir==0){
        exit(1);
    }
    struct stat inode;
    struct dirent *dirEntry;
    dirEntry=readdir(dir);
    pointer=0;
    dirList.clear();
    while(dirEntry!=0){
        struct directoryDetails temp;
        temp.name=dirEntry->d_name;
        lstat(dirEntry->d_name,&inode);
        temp.mode=inode.st_mode;
        temp.owner=getpwuid(inode.st_uid)->pw_name;
        temp.group=getgrgid(inode.st_gid)->gr_name;
        temp.size=inode.st_size;
        temp.time=inode.st_atim.tv_sec;
        dirList.push_back(temp);
        dirEntry=readdir(dir);
    }
    updateView(0);
    closedir(dir);
}

//handles up arrow key
void upArrow(char c[3]){
    if(pointer==viewTop && pointer>0){
        updateView(viewTop-1);
        pointer=viewTop;
        return;
    }
    pointer--;
    if(pointer<0)
        pointer=0;
    write(STDOUT_FILENO,c,3);
}

//handles down arrow key
void downArrow(char c[3]){
    if(pointer<window.ws_row-1){
        pointer++;
        write(STDOUT_FILENO,c,3);
    }
}

//handles right arrow key
void rightArrow(){
    if(!forward_stack.empty()){
        auto path=forward_stack.top();
        forward_stack.pop();
        back_stack.push(get_current_dir_name());
        listDir(path);
    }
}

//handles left arrow key
void leftArrow(){
    if(!back_stack.empty()){
        auto path=back_stack.top();
        back_stack.pop();
        forward_stack.push(get_current_dir_name());
        listDir(path);
    }
}

//handles enters key
void enterKey(){
    if(pointer<=dirList.size()){
        auto dirContent=dirList[pointer];
        if((S_IFDIR&dirContent.mode)==S_IFDIR){
            back_stack.push(get_current_dir_name());
            listDir(dirContent.name);
        }
        else{
            int pid=fork();
            if(pid==0){
                //char *path=get_current_dir_name();
                execl("/usr/bin/vi","vi",dirContent.name,NULL);
            }
            else{
                int *status;
                wait(status);
            }
        }
    }
}

//handles backspace
void backspaceKey(){
    char *cur_dir=get_current_dir_name();
    if(strcmp(cur_dir,home)!=0){
        back_stack.push(cur_dir);
        listDir("..");
    }
}

//handles key press in normal mode
void switchModes(){
    if (!isatty (STDIN_FILENO))
    {
        printf ("Not a terminal.\n");
        exit (EXIT_FAILURE);
    }
    struct termios canon_mode, noncanon_mode;
    tcgetattr(STDIN_FILENO, &canon_mode);
    tcgetattr(STDIN_FILENO, &noncanon_mode);
    noncanon_mode.c_lflag &= ~(ECHO | ICANON);
    //switch to non canonical
    tcsetattr(STDIN_FILENO, TCSANOW, &noncanon_mode);
    char c[3]={'\0'};
    while(1){

        //read key press from standard input
        read(STDIN_FILENO,c,3);
        if(c[0]=='\004'){
            break;
        }

        //handles arrow key
        if(c[0]==27){
            if(c[2]==65){
                upArrow(c);
            }
            if(c[2]==66){
                downArrow(c);
            }
            if(c[2]==67){
                rightArrow();
            }
            if(c[2]==68){
                leftArrow();
            }
        }
        
        //check enter
        if(c[0]==10){
            enterKey();
        }

        //check backspace
        if(c[0]==127){
            backspaceKey();
        }

        //check 'k' press
        if(c[0]=='k' || c[0]=='K'){
            if(viewTop>0){
                updateView(viewTop-1);
            }
        }

        //check 'l' press
        if(c[0]=='l' || c[0]=='L'){
            if(viewEnd<dirList.size()-1){
                updateView(viewTop+1);
            }
        }

        //check 'h' press
        if(c[0]=='h' || c[0]=='H'){
            back_stack.push(get_current_dir_name());
            clearForwardStack();
            listDir(home);
        }

        //check ':' press and switch to command mode
        if(c[0]==':'){
            commandMode();
            listDir(get_current_dir_name());
        }

        //reset input buffer
        memset(c, 0, 3 * sizeof(c[0]));
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &canon_mode);
}
