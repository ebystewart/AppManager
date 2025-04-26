#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if(argc == 2U)
    {
        if(strcmp(argv[1], "-help") == 0U){
            printf("Help!!!\n");
        }
    }
    //while(1);

    return 0;
}