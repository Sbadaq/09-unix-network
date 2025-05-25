
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>


#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc,char** argv) {
    int fd;
    char *addr;
    off_t offset,pa_offset;
    size_t length;
    ssize_t s;
    struct stat sb;

    if(argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <pathname> <length>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = fopen(argv[1], "r");
    if(fd == -1)
        handle_error("open");

    //get the file stats structure
    if(fstat(fd,&sb) == -1)
        handle_error("fstat");
    
    offset = atoi(argv[2]);
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    //计算页面对齐的偏移量（pa_offset），因为mmap要求偏移量必须是页面大小的整数倍。这是通过清除偏移量的低几位来实现的，其中页面大小通过sysconf(_SC_PAGE_SIZE)获取
    /* must be page align*/
    /*
        pa_offset = 0x1234 & ~(0x1000 - 1)
           = 0x1234 & ~0x0FFF
           = 0x1234 & 0xF000
           = 0x1000

           */
    /*
    struct stat {
        dev_t     st_dev;     // ID of device containing file
        ino_t     st_ino;     // Inode number
        mode_t    st_mode;    // File type and mode
        nlink_t   st_nlink;   // Number of hard links
        uid_t     st_uid;     // User ID of owner
        gid_t     st_gid;     // Group ID of owner
        dev_t     st_rdev;    // Device ID (if special file)
        off_t     st_size;    // Total size, in bytes
        blksize_t st_blksize; // Block size for filesystem I/O
        blkcnt_t  st_blocks;  // Number of 512B blocks allocated

        struct timespec st_atim; // Last access time
        struct timespec st_mtim; // Last modification time
        struct timespec st_ctim; // Last status change time
    };

    */      
    if(offset >= sb.st_size)
    {
        fprintf(stderr, "offset is beyond the end of file\n");
        exit(EXIT_FAILURE);
    }

    if(argc == 4)
    {
        length = atoi(argv[3]);
        if((offset + length) > sb.st_size)
            length = sb.st_size - offset;
    }else{
        length = sb.st_size - offset;
    }

    addr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, pa_offset);
    if(addr == MAP_FAILED)
    {
        handle_error("mmap");
    }

    s = write(STDOUT_FILENO, addr + (offset - pa_offset), length);
    if (s != length)
        if(s == -1)
            handle_error("write");
    munmap(addr, length + offset - pa_offset);

    close(fd);

    return 0;
}





































































