#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <spawn.h>
#include "AppManager.h"
#include <stdbool.h>
#include <string.h>


#define BIN_FILE_NAME_MAXLEN  50U
#define BIN_PATH_MAXLEN      150U
#define BIN_ARGS_MAXLEN      250U

#define min(a,b)  (a < b ? a : b)
      

typedef struct{
    char binName[BIN_FILE_NAME_MAXLEN];
    char binPath[BIN_PATH_MAXLEN];
    char arguments[BIN_ARGS_MAXLEN];
    uint32_t uid;
    uint32_t gid;
    uint32_t pid;
}AppAttributes_t;

static bool_t parse_app(FILE *fd);
static void run_app(AppAttributes_t *appAttr);
static void print_options(void);
static void parse_attributes(char *sptr, unsigned int str_len);

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
    char *sptr;
    /* temporary allocate 1KB in heap to read and store the conf file */
    char *tmpBuf = (char *)calloc(1, 100);

    cptr = fgetc(fd);
    if(cptr == '$')
    {
        while (cptr != EOF)
        {
            cptr = fgetc(fd);
            if(cptr == '#')
            {
                sptr = fgets(tmpBuf, 100, fd);
                printf("%s\n",tmpBuf);
                parse_attributes(sptr, strlen(sptr));
                memset(sptr, '\n', 100);
            }
        }
    }
    //fgets(tmpBuf, 1000, fd);
    //printf("%s\n",tmpBuf);

    //fptr = strchr(tmpBuf, '$');
    //printf("%s\n",fptr+1);

    /* release the heap */
    free(tmpBuf);

}

static void parse_attributes(char *sptr, unsigned int str_len)
{
    char *cptr;
    char *aptr = malloc(BIN_ARGS_MAXLEN);
    char *tmp = malloc(str_len);
    AppAttributes_t *attr = calloc(1, sizeof(AppAttributes_t));
    strncpy(tmp, sptr, str_len);
    cptr = strtok_r(tmp, " ", &tmp);
    //printf("%s\n",cptr);
    if(strlen(cptr) <= BIN_FILE_NAME_MAXLEN)
        strcpy(attr->binName, cptr);
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
            }
            if(cptr[1] == 'a'){
                //printf("-a is reached\n");
                aptr = strtok_r(aptr+3, "\"", &aptr);  
                memcpy(attr->arguments, aptr, strlen(aptr));
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
    printf("Arguments: %s\n",attr->arguments);
    printf("UID: %d\n",attr->uid);
    printf("GID: %d\n",attr->gid);
    run_app(attr);
    
end:
    free(aptr);
    free(cptr);
    free(tmp);
    exit(0);
}

static void run_app(AppAttributes_t *appAttr)
{
    pid_t pid;
    char *binFile;
    char * const* name = "App1";

    binFile = strcat(appAttr->binPath, appAttr->binName);
    printf("Full path is : %s\n", appAttr->binPath);

    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSCHEDULER | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETPGROUP);
    //posix_spawnattr_setschedpolicy(&attr,);
    //posix_spawnattr_setpgroup(&attr, );
    //posix_spawnattr_set

    if(posix_spawnp(&pid, appAttr->binPath, NULL, &attr, "Name", NULL))
    {
        perror("spawn failed");
        exit(0);
    }
    else
        printf("Process spawned with pid %d\n", pid);
}