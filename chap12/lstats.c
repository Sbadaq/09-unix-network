//the following program calls lstat() and display s select fields in the returneed stat structure
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysmacros.h>
#include <linux/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    struct stat file_st;

    if(argc != 2)
    {
        fprintf(stderr, "Usage: %s <pathname>\n", argv[0]);
        return 1;
    }
    if(lstat(argv[1], &file_st) < 0)
    {
        perror("lstat");
        return 2;
    }

    printf("ID of containing device : [%x,%x]\n",major(file_st.st_dev), minor(file_st.st_dev));
    printf("File Type : ");
    switch(file_st.st_mode & S_IFMT)
    {
        case S_IFBLK:  printf("block device\n"); break;
        case S_IFCHR:  printf("character device\n"); break;
        case S_IFDIR:  printf("directory\n"); break;
        case S_IFIFO:  printf("FIFO/pipe\n"); break;
        case S_IFLNK:  printf("symbolic link\n"); break;
        case S_IFREG:  printf("regular file\n"); break;
        case S_IFSOCK: printf("socket\n"); break;
        default:       printf("unknown?\n"); break;
    }

    printf("I-node number : %ld\n",file_st.st_ino);
    printf("Mode : %lo (octal)\n",file_st.st_mode);
    printf("Number of links : %ld\n",file_st.st_nlink);
    printf("Ownership : UID=%ld   GID=%ld\n",(long)file_st.st_uid,(long)file_st.st_gid);
    printf("Preferred I/O block size : %ld bytes\n",file_st.st_blksize);
    printf("Blocks allocated : %ld\n",file_st.st_blocks);
    printf("Last file access : %s",ctime(&file_st.st_atime));
    printf("Last file modification : %s",ctime(&file_st.st_mtime));
    printf("Last status change : %s",ctime(&file_st.st_ctime));
    printf("File size : %ld bytes\n",file_st.st_size);

    printf("Size : %ld bytes\n",file_st.st_size);
    return 0;

}