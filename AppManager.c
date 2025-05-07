#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <spawn.h>
#include "AppManager.h"
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include "glueThread/glthread.h"
#include <mqueue.h>
#include <sys/stat.h>
#include <pthread.h>


#define BIN_FILE_NAME_MAXLEN  50U
#define BIN_PATH_MAXLEN      150U
#define BIN_ARGS_MAXLEN       50U
#define MAX_ARGS               3U

#define min(a,b)  (a < b ? a : b)

typedef enum {
    NEW = 0,
    RUNNING,
    TERMINATED,
    ZOMBIE
}proc_state_e;

typedef struct{
    char binName[BIN_FILE_NAME_MAXLEN];
    char binPath[BIN_PATH_MAXLEN];
    char arguments[MAX_ARGS][BIN_ARGS_MAXLEN];
    char environ[1][10];
    uint32_t uid;
    uint32_t gid;
    uint32_t pid;
    proc_state_e state;
    glthread_t attrGlue;
}AppAttributes_t;


GLTHREAD_TO_STRUCT(glue_dll_to_attr, AppAttributes_t, attrGlue);

static AppAttributes_t *head = NULL; // this is root and will not have any data
//static AppAttributes_t *curr; // This always point to the latest update
int fd_pipe;


static bool_t parse_app(FILE *fd);
static void run_app(AppAttributes_t *appAttr);
static void print_options(void);
static void parse_attributes(const char *sptr, unsigned int str_len);
void signal_handler(int signum)
{
    pid_t status;
    printf("Error: Signal handler called\n");
    if(signum == SIGINT || signum == SIGABRT || signum == SIGTERM){
        glthread_t *curr;
        AppAttributes_t *attr;
        printf("Error: Process termination requested\n");

        ITERATE_GLTHREAD_BEGIN(&head->attrGlue, curr){

            attr = glue_dll_to_attr(curr);
            printf("Info: Sending kill request to pid %d\n",attr->pid);
            /* When parent terminates, kill the child processes too, 
               so that the child processes do not become zombies */
            if(attr->state == RUNNING || attr == NULL){
                kill(attr->pid, SIGTERM);
                do
                {
                    if (waitpid(attr->pid, &status, WNOHANG) != -1)
                    {
                        printf("Child Status %d\n", WEXITSTATUS(status));
                    }
                    else
                    {
                        perror("waitpid failed\n");
                        kill(attr->pid, SIGKILL);
                        // exit(1);
                    }

                } while (!WIFEXITED(status) && !WEXITSTATUS(status));
                free(attr);
            }  
        }ITERATE_GLTHREAD_END(&head->attrGlue, curr);
        free(head);
    }
    exit(1);
}

/* Handle when a child process terminates */
void sigchld_handler(int sig)
{
    pid_t pid;
    int status;
    glthread_t *curr;
    AppAttributes_t *attr;
    while((pid = waitpid(-1, &status, WNOHANG)) <= 0U);
    ITERATE_GLTHREAD_BEGIN(&head->attrGlue, curr){
        attr = glue_dll_to_attr(curr);
        if(pid == attr->pid)
        {
            remove_glthread(curr);
            attr->state = TERMINATED;
            free(attr);
        }
    }ITERATE_GLTHREAD_END(&head->attrGlue, curr);
    printf("Error: child process with PID : %d terminated\n", pid);
}

static void thread_handler(void *arg)
{
    int fd2;
    unsigned char rxBuffer[10];
    unsigned int prio = 0U;
    struct sigevent *sigevent;
    //sigevent->sigev_notify = SIGEV_THREAD;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    fd_pipe = open("/home/eby/npipe", O_WRONLY);
    if (fd_pipe == -1)
    {
        printf("Error: Queue may not exist, create new\n");
        /* We use a named fifo because, we create the fifo much later than spanwing teh child process */
        if(mkfifo("/home/eby/npipe", S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH) == -1){
            perror("fifo creation failed");
            exit(-1);
        }
        /* open and close for initialization */
        fd_pipe = open("/home/eby/npipe", O_WRONLY);
        close(fd_pipe);
    }
    if(fd_pipe == -1){
        perror("fifo_open failed");
        exit(-1);
    }

    // another way is to use mq_notify()
    while(1)
    {
        fd2 = open("/home/eby/npipe", O_RDONLY);  
        if(read(fd2, rxBuffer, sizeof(rxBuffer)) != -1)
        {
            printf("Process termination request received for PID : %d\n", rxBuffer[0]);
            close(fd2);
        }
    }
}

int main(int argc, char **argv)
{
    FILE *fd;
    char *filePath = NULL;
    pthread_attr_t attr;
    pthread_t *pthread = calloc(1, sizeof(pthread_t));

    signal(SIGINT, signal_handler);
    signal(SIGCHLD, sigchld_handler);

    if((argv[2] != NULL) && (strcmp(argv[1], "-c") == 0U) && (argc == 3U))
    {
        filePath = (char *)malloc(100);
        filePath = argv[2];
        printf("The configuration file path is %s\n", filePath);
        fd = fopen(filePath, "r");
    }
    else if((strcmp(argv[1], "-h") == 0U) && (argc == 2))
    {
        print_options();
        goto end;
    }
    else 
    {
        printf("Error: Invalid arguments passed\n");
        print_options();
        goto end;
    }
    if(!fd){
        free(filePath);
        goto end;
    }
    else{
        head = (AppAttributes_t *)calloc(1, sizeof(AppAttributes_t));
        init_glthread(&head->attrGlue);
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        pthread_create(pthread, &attr, thread_handler, NULL);
        //curr = &head;
        parse_app(fd);
        fclose (fd);
        pthread_join(pthread, NULL);
    }
    /* This is to support one entry one exit */   
    end:
        //while(1); // should be replaced with pthread_join()
        pthread_attr_destroy(&attr);
        pthread_cancel(pthread);
        pthread_join(pthread, NULL);
        free(pthread);
        exit(0);   
    return 0;
}

static void print_options(void)
{
    printf("******************************************************************************\n");
    printf("*********************************** APP Manager ******************************\n");
    printf("******************************************************************************\n");
    printf("\t -c <configuration file> : configuartion file containing application\n");
    printf("\t -h                      : help \n");
    printf("******************************************************************************\n");
}

/*  This function gets the number of applications configured in the configuration file
    Each configuration should start with a '#' symbol.
    The Start and end of the file is marked by a '$' symbol, without which the configuration file is termed invalid. */
static unsigned int get_numAppEntries(FILE *fd)
{

}
static char *get_executableFile(FILE *fd, unsigned int appNum)
{

}

static char *get_executableArgs(FILE *fd, unsigned int appNum)
{

}

static char *get_executableUserId(FILE *fd, unsigned int appNum)
{

}

static char *get_executableGroupId(FILE *fd, unsigned int appNum)
{

}

static bool_t parse_app(FILE *fd)
{
    char cptr;
    //char *fptr;
    const char *sptr;
    /* temporary allocate 1KB in heap to read and store the conf file */
    char *tmpBuf = (char *)calloc(1, 1024);
    #if 0
    /* we assume that the first character in the file is $ */
    cptr = fgetc(fd);
    if(cptr == '$')
    {
        while (cptr != EOF)
        {
            cptr = getc(fd);
            if(cptr == '\n')
            {

                cptr = fgetc(fd);
                if(cptr == '#')
                {
                    /* find the start of an app configuration */
                    sptr = fgets(tmpBuf, 100, fd);
                    printf("%s\n",tmpBuf);
                    parse_attributes(sptr, strlen(sptr));
                    //memset(sptr, '\0', 100);
                }
                printf("Parsing next line.....%c\n",cptr);
            }
        }
    }
    else{
        printf("Error: Config file marker not found! Invalid conf file\n");
        exit(1);
    }
    #endif
    #if 1
    bool sof_marker = false;
    unsigned int idx = 0U;
    unsigned int p_idx;
    // somehow char **dsptr = calloc(1, 4096) was causing a segmentation fault. need to find the reason
    char dsptr[4][1024];
    if(fd != NULL){
        while(fgets(tmpBuf, 1024, fd)){
            printf("%s\n", tmpBuf);
            if(tmpBuf[0] == '$')
            {
                sof_marker = true;
            }
            if((sof_marker == true) && (tmpBuf[0] == '#')){
                tmpBuf++;
                strcpy(dsptr[idx], tmpBuf);
                printf("op: %s\n", dsptr[idx]);
                idx++;
            } 
        }
    }
    printf("number of Apps configured is %d\n", idx);
    for(p_idx = 0U; p_idx < idx; p_idx++)
    {
        // This parses only once.Need to find the rason and fix
        parse_attributes(dsptr[p_idx], strlen(dsptr[p_idx]));
        printf("App started\n");
    }
    #endif

    /* release the heap */
    //free(tmpBuf);

}

static void parse_attributes(const char *sptr, unsigned int str_len)
{
    unsigned int idx = 1U;
    char *cptr;
    char *aptr = calloc(1, BIN_ARGS_MAXLEN);
    char *tmp = calloc(1, str_len);
    AppAttributes_t *attr = calloc(1, sizeof(AppAttributes_t));
    strncpy(tmp, sptr, str_len);
    cptr = strtok_r(tmp, " ", &tmp); // looks like memory will be auto-freed after all tokens are extracted
    printf("%s\n",cptr);
    if(strlen(cptr) <= BIN_FILE_NAME_MAXLEN)
    {
        strcpy(attr->binName, cptr);
        strcpy(attr->arguments[0], cptr);
    }
    else
        goto end;

    /* get the attaributes for the application */
    while(cptr = strtok_r(tmp, " ", &tmp)){
        printf("%s\n",cptr);
        if(*cptr == '-')
        {
            memcpy(aptr, cptr, min(BIN_ARGS_MAXLEN, str_len));
            //printf("- is reached\n");
            if(cptr[1] == 'd')
            {
                //printf("-d is reached\n");
                aptr = strtok_r(aptr+3, "\"", &aptr);        
                memcpy(attr->binPath, aptr, strlen(aptr));
                //need to count the argument to avoid duplication 
            }
            if(cptr[1] == 'a'){
                //printf("-a is reached\n");
                aptr = strtok_r(aptr+3, "\"", &aptr);
                // If -a if followed by arguments seperated by spaces, special handling is required
                //while(aptr = strtok_r(aptr, " ", &aptr)){
                    printf("Args: %s\n", aptr);
                    if(idx < (MAX_ARGS - 1U)){
                        memcpy(attr->arguments[idx], aptr, strlen(aptr));
                        printf("Args: parsing argument %d\n", idx);
                        idx++;
                    }
                    else{
                        printf("Error: More args than configured\n");
                        exit(1);
                    }
                //}
                *attr->arguments[MAX_ARGS - 1] = NULL;
            }
            if(cptr[1] == 'u'){
                //printf("-u is reached\n");
                attr->uid = atoi(cptr+2);
                //printf("UID: %d\n", attr->uid);
            }
            if(cptr[1] == 'g'){
                //printf("-g is reached\n");
                attr->gid = atoi(cptr+2);
                //printf("GID: %d\n", attr->gid);
            }
        }
    }
    glthread_add_next(&head->attrGlue, &attr->attrGlue);
    //curr = attr;
    printf("Bin Name: %s\n",attr->binName);
    printf("Bin Path: %s\n",attr->binPath);
    printf("Arguments: %s\n",attr->arguments[0]);
    printf("Arguments: %s\n",attr->arguments[1]);
    printf("Arguments: %s\n",*attr->arguments[2]);
    printf("UID: %d\n",attr->uid);
    printf("GID: %d\n",attr->gid);

    run_app(attr);
    
end:
    //free(aptr);
    free(cptr);
    //free(tmp);
    //exit(0);
}

static void run_app(AppAttributes_t *appAttr)
{
    pid_t pid;
    int status;
    unsigned int idx;
    char *binFile;
    int schedPolicy;
    struct sched_param param;
    char txBuffer[5] ;
    txBuffer[4] = '\0';

    // need to check why spawnp fialed when strcpy() is used to copy files to argv
    const char *argv[MAX_ARGS];
    //strcpy(argv[0], appAttr->binName); This step causes segmentation fault. Need to explore and understand. is it stack out of bounds?
    for(idx = 0U; idx < (MAX_ARGS-1); idx++)
    {
        argv[idx] = appAttr->arguments[idx];
    }
    argv[MAX_ARGS - 1] = NULL;

    /*
    printf("Arg1: %s\n", argv[0]);
    printf("Arg2: %s\n", argv[1]);
    printf("Arg3: %s\n", argv[2]);
    */

    binFile = strcat(appAttr->binPath, appAttr->binName);
    printf("Full path is : %s\n", appAttr->binPath);

    /* New process need not inherit the opened files of parent. This may also break the pipe between the
       child process and console */
    posix_spawn_file_actions_t file_actions;
    posix_spawn_file_actions_init(&file_actions);
    posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0); // Redirect stdin
    posix_spawn_file_actions_addopen(&file_actions, 1, "/dev/null", O_WRONLY, 0); // Redirect stdout
    posix_spawn_file_actions_addopen(&file_actions, 2, "/dev/null", O_WRONLY, 0); // Redirect stderr

    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    /* detach the new process from its parent's session. This is required to detach it from the shell which launched it */
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSCHEDULER | POSIX_SPAWN_SETSCHEDPARAM | \
                                    POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_RESETIDS);
    #ifdef __QNX__
    //https://www.qnx.com/developers/docs/6.5.0SP1.update/com.qnx.doc.neutrino_lib_ref/p/posix_spawnattr_setxflags.html
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSID | POSIX_SPAWN_SETCRED);
    posix_spawnattr_setsid(&attr);
    posix_spawnattr_setcred(&attr, appAttr->uid, appAttr->gid);
    #endif
    schedPolicy = SCHED_RR;
    param.sched_priority = 20;
    /* spawn fails if sched param and/or policy is changed (in Ubuntu test setup) */
    //posix_spawnattr_setschedpolicy(&attr, schedPolicy);
    //posix_spawnattr_setschedparam(&attr, &param);
    //posix_spawnattr_setpgroup(&attr, (pid_t)appAttr->gid);

    status = posix_spawnp(&pid, appAttr->binPath, &file_actions, &attr, argv, appAttr->environ);
    if (status == 0)
    {
        appAttr->pid = pid;
        appAttr->state = RUNNING;
        memcpy(txBuffer, (char *)&appAttr->pid, 4U);
        printf("Child PID : %d\n", appAttr->pid);
        fd_pipe = open("/home/eby/npipe", O_WRONLY);
        if (write(fd_pipe, txBuffer, 5U) != -1)
        {
            printf("Monitor notified of new App\n");
            close(fd_pipe);
        }
        /*do
        {
            if(waitpid(pid, &status, 0) != -1){
                printf("Child Status %d\n", WEXITSTATUS(status));
            }
            else{
                perror("waitpid failed\n");
                exit(1);
            }

        } while (!WIFEXITED(status) && !WEXITSTATUS(status));*/
        /* Detach the child process. This should have been done from the child process  */
        setsid();
        //setuid(appAttr->uid);
        //setgid(appAttr->gid);
    }
    else
    {
        printf("spawn failed: %s\n", strerror(status));
    }

    posix_spawn_file_actions_destroy(&file_actions);
    posix_spawnattr_destroy(&attr);
}