#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>


void* create_sharedmemory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;
  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;
  return mmap(NULL, size, protection, visibility, -1, 0);
}


char* SIG_NAME(int sig)
{
	switch (sig)
	{
	case 1:
		return "SIGHUP";
		break;
	case 2:
		return "SIGINT";
		break;
	case 3:
		return "SIGQUIT";
		break;
	case 4:
		return "SIGILL";
		break;
	case 5:
		return "SIGTRAP";
		break;
	case 6:
		return "SIGABRT";
		break;
	case 7:
		return "SIGBUS";
		break;
	case 8:
		return "SIGFPE";
		break;
	case 9:
		return "SIGKILL";
		break;
	case 11:
		return "SIGSEGV";
		break;
	case 13:
		return "SIGPIPE";
		break;
	case 14:
		return "SIGALRM";
		break;
	case 15:
		return "SIGTERM";
		break;
	case 19:
		return "SIGSTOP";
		break;
	default:
		return "NONE";
		break;
	}
	return "ERROR";
}

void *EO_string(void* ptr_void)
{
	char* ptr=ptr_void;
	while(*ptr!='\0')ptr++;
	return (void*)ptr;
}


int main(int argc,char *argv[]){
	pid_t pid=getpid();
	int status;
	
	char **ptrv=argv+1;
	char str[20]="";
	char log_msg[100];

	// create shared memory
	void* shmem = create_sharedmemory(2000);

	sprintf(log_msg, "---\nProcess tree: %d",getpid());
	memcpy(shmem,log_msg,sizeof(log_msg));
	

	while((*ptrv)!=NULL)
	{
		if((pid=fork())==0)// fork child process
		{
			sprintf(log_msg," -> %d",getpid());
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
			fflush(stdout);
			strcpy(str,*(ptrv++)); // child process should execute str
		}
		else break;
	}
	/* wait for child process terminates */
	if(pid!=0)
	{
		pid=waitpid(pid,&status,WUNTRACED);
		sprintf(log_msg,"Child process %d of parent process %d is ",pid,getpid());
		memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		if(WIFEXITED(status)){
			int sig=WEXITSTATUS(status);
			sprintf(log_msg,"terminated normally with EXIT STATUS = %d\n",sig);
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		}
		else if(WIFSIGNALED(status)){
			int sig=WTERMSIG(status);
			char* sigName=SIG_NAME(sig);
			sprintf(log_msg,"terminated by signal %d (%s)\n",sig,sigName);
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		}	
		else if(WIFSTOPPED(status)){
			int sig=WSTOPSIG(status);
			char* sigName=SIG_NAME(sig);
			sprintf(log_msg,"terminated by signal %d (%s)\n",sig,sigName);
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		}
		else if(WIFCONTINUED(status)){
			sprintf(log_msg,"continued\n");
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		}
		else{
			printf(log_msg,"OTHER EXCEPTIONS\n");
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		}
	}

	/* execute program (parent process do not) */
	if(strlen(str)>0)
	{
		if(ptrv-argv==argc)
		{
			sprintf(log_msg,"\n");
			memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		}
		char *para[2];
		para[0]=str;
		para[1]=NULL;
		execve(para[0],para,NULL);
	}
	else
	{
		sprintf(log_msg,"Myfork process (%d) terminated normally.\n",getpid());
		memcpy(EO_string(shmem),log_msg,sizeof(log_msg));
		printf("\n\n%s",(char*)shmem);
	}
	return 0;
}
