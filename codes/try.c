#include <unistd.h>
#include <sys/types.h>

int main(){
	printf("%d\n",getpid());
	int pid=vfork();
	if(pid==0){
		//char a[]=["ls"];
		printf("Child\n");
		execlp("ls","ls",NULL);
	}
	else{
		printf("Parent\n");
	}
}
