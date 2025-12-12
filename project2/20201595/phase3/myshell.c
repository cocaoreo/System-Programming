/* $begin shellmain */
#include "csapp.h"
#include<errno.h>
#define MAXARGS   128
#define MAXJOB 200

typedef struct job {// 현재 위치(bg, fg), 현재 상태(R, S) 추가 필요 
    int pid;
    int listnumb;
    int bg; //1: bg, 2: fg
    int state; //1: run, 2: suspend
    char cmd[MAXARGS];
}Job;

Job list[MAXJOB];
int idx = 1;
int jcnt = 0;

void initjob();
void addjob(int pid, char *cmd, int bg);
void deletejob(int pid);
void suspendjob(int pid);
void printjob();

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
int pipeparse(char *buf, char **cndline);

void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);


volatile sig_atomic_t checkpid;

int main() 
{
    char cmdline[MAXLINE]; /* Command line */

    initjob();
    Signal(SIGCHLD, sigchld_handler);
    Signal(SIGINT, sigint_handler);
    Signal(SIGTSTP, sigtstp_handler);

    while (1) {
	/* Read */
	printf("CSE4100-SP-P2> ");                   
	fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    } 
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    char *cmd[MAXARGS];
    int cmdcnt;
    int fd[100][2];
    int pipeflag=0;
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    sigset_t mask, prev, mask_all, prev_all;

    Sigfillset(&mask_all);
    Sigemptyset(&mask);
    Sigemptyset(&prev);
    Sigaddset(&mask, SIGCHLD);
    Sigaddset(&mask, SIGINT);
    Sigaddset(&mask, SIGTSTP);
    strcpy(buf, cmdline);
    cmdcnt = pipeparse(buf,cmd);
    if(cmdcnt>1)
        pipeflag = 1;

    for(int i=0;i<cmdcnt;i++){

    bg = parseline(cmd[i], argv);
    if (argv[0] == NULL)  
	return;   /* Ignore empty lines */
    if (!builtin_command(argv)) { //quit -> exit(0), & -> ignore, other -> run
        if(pipeflag==1)
            pipe(fd[i]);
        Sigprocmask(SIG_BLOCK, &mask, &prev);

        if (((pid = Fork())== 0)){
            if (!bg) Sigprocmask(SIG_UNBLOCK, &mask, NULL);
            if(pipeflag == 1){	
				if(i!=0){
                    close(fd[i-1][1]);
					dup2(fd[i-1][0],0);
					close(fd[i-1][0]);
				}
                
                if(i!=cmdcnt-1){
                    close(fd[i][0]);
					dup2(fd[i][1], 1);
					close(fd[i][1]);
				}
			}

            if (execve(argv[0], argv, environ) < 0) {	//ex) /bin/ls ls -al &
            printf("%s: Command not found.\n", argv[0]);
            exit(0);
            }
        }


        Sigprocmask(SIG_UNBLOCK, &mask, NULL);
	    /* Parent waits for foreground job to terminate */
	    if (!bg){
            
            if(pipeflag==1)
                close(fd[i][1]);

            Sigprocmask(SIG_BLOCK, &mask_all, &prev);
            addjob(pid, cmd[i], 2);
            Sigprocmask(SIG_SETMASK, &prev, NULL);
        
            checkpid = 0;
            int flag = 0;
            while(!checkpid){
                Sigsuspend(&prev);
                for(int i=0;i<jcnt;i++){
                    if(list[i].bg ==2){
                        flag = 1;
                    }
                }
                if(!flag) break;
            }
            //printf("%s\n", cmd[i]);
            Sigprocmask(SIG_SETMASK, &prev, NULL);
	    }
	    else{//when there is background process!
	        Sigprocmask(SIG_BLOCK, &mask_all, NULL);
            addjob(pid, cmd[i], 1);
            Sigprocmask(SIG_SETMASK, &prev, NULL);
        
            printf("[%d] %s", pid, cmdline);
        }
    }
    }
    return ;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{   
    if (!strcmp(argv[0], "ls")){
        argv[0]= "/bin/ls";
    }

    if (!strcmp(argv[0], "mkdir")){
        argv[0]= "/bin/mkdir";
    }

    if (!strcmp(argv[0], "rmdir")){
        argv[0]= "/bin/rmdir";
    }

    if (!strcmp(argv[0], "touch")){
        argv[0]= "/bin/touch";
    }

    if (!strcmp(argv[0], "cat")){
        argv[0]= "/bin/cat";
    }

    if (!strcmp(argv[0], "echo")){
        argv[0]= "/bin/echo";
    }

    if (!strcmp(argv[0], "cd")){

        int a = chdir(argv[1]);
        if (a!=0){
            perror("cd error");
        }

        return 1;
    }

    if (!strcmp(argv[0], "grep")){
        argv[0]= "/bin/grep";
    }

    if (!strcmp(argv[0], "less")){
        argv[0]= "/bin/less";
    }

    if (!strcmp(argv[0], "sort")){
        argv[0]= "/bin/sort";
    }

    if (!strcmp(argv[0], "jobs")){
        printjob();
        return 1;
    }

    if (!strcmp(argv[0], "kill")){
        char * temp ;
        sigset_t mask_all, prev;
        Sigemptyset(&mask_all);
        Sigfillset(&mask_all);

        temp = strtok(argv[1], "%%");
        int k = atoi(temp);

        for(int i=0;i<jcnt;i++){
            Sigprocmask(SIG_BLOCK, &mask_all, &prev);
            if(list[i].listnumb==k){
                Kill(list[i].pid, SIGKILL);
                deletejob(list[i].pid);
                Sigprocmask(SIG_SETMASK, &prev, NULL);
                return 1;
            }
        }
        Sigprocmask(SIG_SETMASK, &prev, NULL);
        printf("No Such Job\n");
        return 1;
    }

    if (!strcmp(argv[0], "fg")){
        char * temp ;
        temp = strtok(argv[1], "%%");
        int k = atoi(temp);
        sigset_t mask_all, prev;
        Sigemptyset(&mask_all);
        Sigfillset(&mask_all);
        for(int i=0;i<jcnt;i++){
            Sigprocmask(SIG_BLOCK, &mask_all, &prev);
            if(list[i].listnumb==k){
                Kill(list[i].pid, SIGCONT);
                printf("[%d] running %s\n", list[i].listnumb, list[i].cmd);
                list[i].state = 1;
                list[i].bg = 2;

                checkpid = 0;
                int flag = 0;
                while(!checkpid){
                    Sigsuspend(&prev);
                    for(int i=0;i<jcnt;i++){
                        if(list[i].bg ==2){
                            flag = 1;
                        }
                    }
                    if(!flag) break;
                }
                Sigprocmask(SIG_SETMASK, &prev, NULL);
                return 1;
            }
        }
        Sigprocmask(SIG_SETMASK, &prev, NULL);
        printf("No Such Job\n");
        return 1;
    }

    if (!strcmp(argv[0], "bg")){
        char * temp ;
        temp = strtok(argv[1], "%%");
        int k = atoi(temp);
        sigset_t mask_all, prev;
        Sigemptyset(&mask_all);
        Sigaddset(&mask_all, SIGCHLD);
        for(int i=0;i<jcnt;i++){
            Sigprocmask(SIG_BLOCK, &mask_all, &prev);
            if(list[i].listnumb==k){
                Kill(list[i].pid, SIGCONT);
                printf("[%d] running %s\n", list[i].listnumb, list[i].cmd);
                list[i].state = 1;
                list[i].bg = 1;

                Sigprocmask(SIG_SETMASK, &prev, NULL);
                return 1;
            }
        }
        Sigprocmask(SIG_SETMASK, &prev, NULL);
        printf("No Such Job\n");
        return 1;
    }

    if (!strcmp(argv[0], "ps")){
        argv[0]= "/bin/ps";
    }

    if (!strcmp(argv[0], "quit")) /* quit command */
	exit(0);  
    if (!strcmp(argv[0], "exit")) 
	exit(0); 
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;
    return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    while (delim = strchr(buf, '\"')){
        *delim = ' ';
    }

    while (delim = strchr(buf, '\'')){
        *delim = ' ';
    }

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
    //printf("%s\n", argv[argc-1]);
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    if((delim = strchr(argv[argc-1], '&')) != NULL){
        bg=1;
        *delim = '\0';
    }
    
    return bg;
}
/* $end parseline */

int pipeparse(char *buf, char **cmdline){
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, '|'))) {
	cmdline[argc] = (char*)malloc(sizeof(char)*MAXLINE);
    *delim = '\0';
    strcpy(cmdline[argc],buf);
    strcpy(cmdline[argc++]+strlen(buf), "\n\0");    
	buf = delim + 1;
    }
    cmdline[argc++] = buf;
    cmdline[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    return argc;
}

void sigchld_handler(int sig){
    int olderrno = errno;
    sigset_t mask_all, prev_all;
    int status;

    Sigfillset(&mask_all);
    while((checkpid = waitpid(-1, &status, WNOHANG|WUNTRACED))>0){
        Sigprocmask(SIG_BLOCK, &mask_all,&prev_all);
        if(WIFEXITED(status)||WIFSIGNALED(status)){
            deletejob(checkpid);
        }

        if(WIFSTOPPED(status)){
            suspendjob(checkpid);
        }
        Sigprocmask(SIG_SETMASK,&prev_all,NULL);
    }
    errno = olderrno;
}

void sigint_handler(int sig){
    int olderrno = errno;
    sigset_t mask_all, prev_all;

    Sigfillset(&mask_all);
    Sigprocmask(SIG_BLOCK, &mask_all,&prev_all);

    for(int i=0;i<MAXJOB;i++){
        if(list[i].bg==2){
            Kill(list[i].pid, SIGINT);
            deletejob(list[i].pid);
        }
    }
    
    Sigprocmask(SIG_SETMASK,&prev_all,NULL);
    errno = olderrno;
}

void sigtstp_handler(int sig){
    int olderrno = errno;
    sigset_t mask_all, prev_all;

    Sigfillset(&mask_all);
    Sigprocmask(SIG_BLOCK, &mask_all,&prev_all);
    for(int i=0;i<MAXJOB;i++){
        if(list[i].bg==2){
            kill(list[i].pid,SIGTSTP);
            suspendjob(list[i].pid);
        }
    }
    Sigprocmask(SIG_SETMASK,&prev_all,NULL);
    errno = olderrno;
}

void initjob(){
    for(int i=0;i<MAXJOB;i++){
        list[i].pid = 0;
        list[i].bg = 0;
        list[i].listnumb =0;
        list[i].state= 0;
        strcpy(list[i].cmd, "\0");
    }
}

void addjob(int pid, char *cmd, int bg){
    list[jcnt].pid = pid;
    list[jcnt].state = 1;
    list[jcnt].bg = bg;
    if(bg==1)
        list[jcnt].listnumb = idx++;
    strcpy(list[jcnt++].cmd, cmd);
}

void deletejob(int pid){
    for(int i=0;i<MAXJOB;i++){
        if(list[i].pid==pid){
            list[i].pid = 0;
            list[i].bg = 0;
            list[i].listnumb =0;
            list[i].state= 0;
            strcpy(list[i].cmd, "\0");
        }
    }
}

void suspendjob(int pid){
    for(int i=0;i<MAXJOB;i++){
        if(list[i].pid==pid){
            if(list[i].listnumb==0)
                list[i].listnumb = idx++;
            list[i].bg = 1;
            list[i].state = 2;
        }
    }
}

void printjob(){
    for(int i=0;i<MAXJOB; i++){
        if(list[i].bg==1){
            if(list[i].state == 1){
                printf("[%d] %s %s\n", list[i].listnumb, "running", list[i].cmd);
            }

            if (list[i].state == 2){
                printf("[%d] %s %s\n", list[i].listnumb, "suspend ", list[i].cmd);
            }
        }
    }
}