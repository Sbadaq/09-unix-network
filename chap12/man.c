
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

    if(fstat(fd,&sb) == -1)
        handle_error("fstat");
    
    offset = atoi(argv[2]);
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    /* must be page align*/

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





































































