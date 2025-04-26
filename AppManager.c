#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <spawn.h>
#include "AppManager.h"
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>


#define BIN_FILE_NAME_MAXLEN  50U
#define BIN_PATH_MAXLEN      150U
#define BIN_ARGS_MAXLEN       50U
#define MAX_ARGS               3U

#define min(a,b)  (a < b ? a : b)
      

typedef struct{
    char binName[BIN_FILE_NAME_MAXLEN];
    char binPath[BIN_PATH_MAXLEN];
    char arguments[MAX_ARGS][BIN_ARGS_MAXLEN];
    char environ[1][10];
    uint32_t uid;
    uint32_t gid;
    uint32_t pid;
}AppAttributes_t;

static bool_t parse_app(FILE *fd);
static void run_app(AppAttributes_t *appAttr);
static void print_options(void);
static void parse_attributes(const char *sptr, unsigned int str_len);

int main(int argc, char **argv)
{
    FILE *fd;
    char *filePath = NULL;
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
        parse_app(fd);
        fclose (fd);
    }
    /* This is to support one entry one exit */   
    end:
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
    sptr = calloc(1, 200);
    if(fd != NULL){
        while(fgets(tmpBuf, 1024, fd)){
            printf("%s\n", tmpBuf);
            if(tmpBuf[0] == '$')
            {
                sof_marker = true;
            }
            if((sof_marker == true) && (tmpBuf[0] == '#')){
                tmpBuf++;
                strncpy(sptr, tmpBuf, strlen(tmpBuf));
                printf("op: %s\n", sptr);
                parse_attributes(sptr, strlen(sptr));
            }
        }
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
    exit(0);
}

static void run_app(AppAttributes_t *appAttr)
{
    pid_t pid;
    int status;
    unsigned int idx;
    char *binFile;

    // need to check why spawnp fialed when strcpy() is used to copy files to argv
    const char *argv[MAX_ARGS];
    //strcpy(argv[0], appAttr->binName); This step causes segmentation fault. Need to explore and understand. is it stack out of bounds?
    for(idx = 0U; idx < (MAX_ARGS-1); idx++)
    {
        argv[idx] = appAttr->arguments[idx];
    }
    argv[MAX_ARGS - 1] = NULL;

    printf("Arg1: %s\n", argv[0]);
    printf("Arg2: %s\n", argv[1]);
    printf("Arg3: %s\n", argv[2]);

    binFile = strcat(appAttr->binPath, appAttr->binName);
    printf("Full path is : %s\n", appAttr->binPath);

    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSCHEDULER | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETPGROUP);
    //posix_spawnattr_setschedpolicy(&attr,);
    //posix_spawnattr_setpgroup(&attr, );
    //posix_spawnattr_set

    status = posix_spawnp(&pid, appAttr->binPath, NULL, &attr, argv, appAttr->environ);
    if (status == 0)
    {
        printf("Child PID : %d\n", pid);
        do
        {
            if(waitpid(pid, &status, 0) != -1){
                printf("Child Status %d\n", WEXITSTATUS(status));
            }
            else{
                perror("waitpid failed\n");
                exit(1);
            }

        } while (!WIFEXITED(status) && !WEXITSTATUS(status));
    }
    else
    {
        printf("spawn failed: %s\n", strerror(status));
    }
}