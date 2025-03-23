#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ipc.h>


int main(int argc, char **argv)
{
    struct stat st;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(1);
    }
    stat(argv[1], &st) ;
    
    printf("st_dev : %lx, st_info : %lx, key: %x \n",(u_long)st.st_dev,(u_long)st.st_ino,ftok(argv[1],0x57));

    exit(0);
}