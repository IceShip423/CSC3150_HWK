#include <linux/module.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <linux/jiffies.h>
#include <linux/kmod.h>
#include <linux/fs.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");


struct wait_opts {  enum pid_type wo_type;  	//It is defined in ‘/include/linux/pid.h’.
	int wo_flags; 				//Wait options. (0, WNOHANG, WEXITED, etc.)
	struct pid *wo_pid;  			//Kernel's internal notion of a process identifier. “Find_get_pid()”
	struct siginfo __user *wo_info; 	//Singal information.
	int __user *wo_stat; 			// Child process’s termination status
	struct rusage __user *wo_rusage; 	//Resource usage
	wait_queue_t child_wait; 		//Task wait queue
	int notask_error ;}; 

/* extern functions */

extern long _do_fork(unsigned long clone_flags,
	unsigned long stack_start,
	unsigned long stack_size,
	int __user *parent_tidptr,
	int __user *child_tidptr,
	unsigned long tls);

extern int do_execve(struct filename *filename,
	const char __user *const __user *__argv,
	const char __user *const __user *__envp);


extern long do_wait(struct wait_opts *wo);

extern struct filename *getname(const char __user * filename);


/* signal-name transform*/
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
	case 131:
		return "SIGQUIT";
		break;
	case 132:
		return "SIGILL";
		break;
	case 133:
		return "SIGTRAP";
		break;
	case 134:
		return "SIGABRT";
		break;
	case 135:
		return "SIGBUS";
		break;
	case 136:
		return "SIGFPE";
		break;
	case 9:
		return "SIGKILL";
		break;
	case 139:
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
	case 4991:
		return "SIGSTOP";
		break;
	default:
		return "normal";
		break;
	}
	return "ERROR";
}


/* implement wait function*/
void my_wait(pid_t pid, int* status)
{
	struct  wait_opts wo;
	struct pid *wo_pid=NULL;
	enum pid_type type;
	int aa;
	type=PIDTYPE_PID;
	wo_pid=find_get_pid(pid);

	wo.wo_type=type;
	wo.wo_pid=wo_pid;
	wo.wo_flags= WUNTRACED | WEXITED;
	wo.wo_info=NULL;
	wo.wo_stat=(int __user*)status;
	wo.wo_rusage=NULL;
 
	aa=do_wait(&wo);
	put_pid(wo_pid);// decrease the count and free memory

}

/* implement exec function*/
void my_exec(void)
{
	int result;
	/* executable path*/
	const char path[]= "/home/zz/Desktop/source/program2/normal";
	const char *const argv[]={path,NULL,NULL};
	const char *const envp[]={"HOME=/","PATH=/sbin:/user/sbin:/bin:/usr/bin",NULL};
	struct filename * my_filename;
	printk("[program2] : Child process pid = %d\n",(int)current->pid);
	printk("[program2] : Child process\n");
	my_filename=getname(path);
	result=do_execve(my_filename,argv,envp);

	/* if exec success*/
	if(!result)
		return;
	/* if exec failure*/
	do_exit(result);
}

/* implement fork function*/
int my_fork(void *argc){
	
	int pid;
	int status=0;
	int i;
	int sig;
	char* sigName;
	struct k_sigaction *k_action;
	printk("[program2] : module_init kthread start");
	/* set default sigaction for current process*/
	k_action = &current->sighand->action[0];
	for(i=0;i<_NSIG;i++){
		k_action->sa.sa_handler = SIG_DFL;
		k_action->sa.sa_flags = 0;
		k_action->sa.sa_restorer = NULL;
		sigemptyset(&k_action->sa.sa_mask);
		k_action++;
	}
	
	/* fork a process using do_fork */
	/* Print out the process id for both parent and child process*/
	printk("[program2] : Parent process pid = %d\n",(int)current->pid);
	if((pid=_do_fork(SIGCHLD,(unsigned long)&my_exec,0,NULL,NULL,0))==0) // child process
	{
		printk("[program2] : exec error");
		do_exit(0);
	}
	

	/* wait until child process terminates */
	my_wait(pid,&status);
	sig=status;
	sigName=SIG_NAME(sig);
	if(sigName[0]!='n')
		printk("[program2] : get %s signal\n",sigName);
	else
		printk("[program2] : normal termination\n");
	printk("[program2] : child process terminated");
	printk("[program2] : The return signal is %d",sig);
	do_exit(0);
}

static int __init program2_init(void){

	struct task_struct *task;
	printk("[program2] : Module_init\n");	
	/* create a kernel thread to run my_fork */
	printk("[program2] : Module_init create kthread start\n");
	task=kthread_create(&my_fork,NULL,"MyThread");
	if(!IS_ERR(task)){	
		wake_up_process(task);
	}
	return 0;
}

static void __exit program2_exit(void){
	printk("[program2] : Module_exit\n");
}

module_init(program2_init);
module_exit(program2_exit);

