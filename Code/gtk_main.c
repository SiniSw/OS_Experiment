#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
int main(int argc,char *argv[]){
	pid_t p1,p2,p3;
	pid_t t1,t2,t3;
	int status;
	if((p1=fork())==0){
		execv("./gtk_time",argv);
	}
	else if((p2=fork())==0){
		execv("./gtk_num",argv);
	}
	else if((p3=fork())==0){
		execv("./gtk_add",argv);
	}
	else{
		t1=waitpid(p1,&status,0);
		t2=waitpid(p2,&status,0);
		t3=waitpid(p3,&status,0);
	}
}

