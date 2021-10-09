#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

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
		return "?????";
		break;
	}
	return "?????";
}


int main(int argc, char *argv[]){

	pid_t pid;
	int status;
	printf("I am Parent process. my pid is %d\n",getpid());
	/* fork a child process */
	if((pid=fork())==0) // child process
	{
		printf("I am Child process.  my pid is %d\n",getpid());
		printf("Child process starts to execute test program:\n");
		/* execute test program */ 
		execve(argv[1],argv+1,NULL);
		exit(0);
	}

	/* wait for child process terminates */
	pid=waitpid(-1,&status,WUNTRACED);
	/* check child process' termination status */
	if(WIFEXITED(status)){
		int sig=WEXITSTATUS(status);
		printf("Parent process receives the SIGCHLD signal\n");
		printf("Normal termination with EXIT STATUS = %d\n",sig);
	}
	else if(WIFSIGNALED(status)){
		int sig=WTERMSIG(status);
		char* sigName=SIG_NAME(sig);
		printf("Parent process receives the SIGCHLD signal\n");
		printf("Child process get the %s signal\n",sigName);
		printf("CHILD EXECUTION FAILED: %d\n", sig);
	}	
	else if(WIFSTOPPED(status)){
		int sig=WSTOPSIG(status);
		char* sigName=SIG_NAME(sig);
		printf("Parent process receives the SIGCHLD signal\n");
		printf("Child process get the %s signal\n",sigName);
		printf("CHILD PROCESS STOPPED: %d\n", sig);
	}
	else if(WIFCONTINUED(status)){
		printf("Parent process receives the SIGCHLD signal\n");
		printf("CHILD PROCESS CONTINUED\n");
	}
	else{
		printf("OTHER EXCEPTIONS\n");
	}
	exit(0);
	return 0;
}
