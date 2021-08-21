#include<stdio.h>
#include<unistd.h>
#include<sys/mman.h>
#include<sys/types.h>

int main(){
    long page_size=sysconf(_SC_PAGE_SIZE);
    printf("Page Size:%ld\n",page_size);
    u_int8_t *first=mmap(NULL,page_size,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
    printf("Page Location:%p\n",first);
    return 0;
}