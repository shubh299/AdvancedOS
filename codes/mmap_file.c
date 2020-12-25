#include<stdio.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/types.h>
#include<fcntl.h>
#include<sys/stat.h>

/*char toupper(char c){
    if(c>=97 && c<=122)
        return c-22;
    return c;
}*/

int main(){
    long page_size=sysconf(_SC_PAGE_SIZE);
    printf("Page Size:%ld\n",page_size);
    int fd=open("./text.txt",O_RDWR,S_IRUSR|S_IWUSR);
    struct stat sb;
    if(fstat(fd,&sb)==-1){
        printf("Error1\n");
    }
    printf("File Size:%ld\n",sb.st_size);
    char *first=mmap(NULL,page_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    printf("Page Location:%p\n",first);
    for(int i=0;i<sb.st_size;i++){
        if(i%2!=0)
            first[i]=toupper(first[i]);
        //printf("%c",first[i]);
    }
    return 0;
}
