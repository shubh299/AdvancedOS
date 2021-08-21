#include<bits/stdc++.h>
#include<sys/types.h>

struct directoryDetails{
    char *name;
    long long int size;
    mode_t mode;
    time_t time;
    char *owner;
    char *group;
    void printPermission(mode_t st_mode){
    //User
        if(st_mode & 0400){
            printf("r");
        }
        else{
            printf("-");
        }
        if(st_mode & 0200){
            printf("w");
        }
        else{
            printf("-");
        }
        if(st_mode & 0100){
            printf("x");
        }
        else{
            printf("-");
        }
        //Group
        if(st_mode & 0040){
            printf("r");
        }
        else{
            printf("-");
        }
        if(st_mode & 0020){
            printf("w");
        }
        else{
            printf("-");
        }
        if(st_mode & 0010){
            printf("x");
        }
        else{
            printf("-");
        }
        //Others
        if(st_mode & 0004){
            printf("r");
        }
        else{
            printf("-");
        }
        if(st_mode & 0002){
            printf("w");
        }
        else{
            printf("-");
        }
        if(st_mode & 0001){
            printf("x");
        }
        else{
            printf("-");
        }    
    }

    void printTime(time_t t){
        tm *lt=localtime(&t);
        printf("%d/",lt->tm_mday);
        printf("%d/",lt->tm_mon);
        printf("%d ",1900+lt->tm_year);
        printf("%d:",lt->tm_hour);
        printf("%d:",lt->tm_min);
        printf("%d",lt->tm_sec);
    }

    void print(){
        printPermission(mode);
        printf(" %s",owner);
        printf(" %s",group);
        printf(" %lld\t",size);
        printTime(time);
        printf("\t%s",name);
    }
};

std::vector<struct directoryDetails> dirList;
int pointer;
char *home;
std::stack<char*> forward_stack;
std::stack<char*> back_stack;
struct winsize window;
int viewTop;
int viewEnd;

void clearForwardStack(){
    while(!forward_stack.empty()){
        forward_stack.pop();
    }
    
}
