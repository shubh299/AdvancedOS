#include<signal.h>
#include "headers.h"

void handle_size_change(int sig){
    if(sig==SIGWINCH){
        updateView(0);
    }
}

int main(int argc,char *argv[]){
    //char *dirName;
    signal(SIGWINCH,handle_size_change);
    home=get_current_dir_name();
    listDir(home);
    switchModes();
}