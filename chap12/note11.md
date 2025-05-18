# 第十二章 共享内存区

共享内存区 是据有IPC中最快的。然而这种共享内存需要同步。
互斥锁、条件变量、读写锁、记录锁、信号量；

nmap 把一个文件或posix共享内存区映射到进程的地址空间。使用该函数有3个目的：
1. 使用普通文件眉眼高低提供内存映射 io
2. 使用特殊文件以提供匿名内存映射 
3. 使用shm_open以提供无亲缘关系的posix共享内存区。

```c
void *mmap(void addr[.length], size_t length, int prot, int flags,
                  int fd, off_t offset);
       int munmap(void addr[.length], size_t length);
```
mmap()  creates  a  new  mapping  in  the  virtual address space of the calling process.  The starting address for the new mapping is specified in addr.  The length argument specifies the length of the mapping (which must be
       greater than 0).

       If addr is NULL, then the kernel chooses the (page-aligned) address at which to create the mapping; this is the most portable method of creating a new mapping.  If addr is not NULL, then the kernel takes it as a  hint  about
       where  to place the mapping; on Linux, the kernel will pick a nearby page boundary (but always above or equal to the value specified by /proc/sys/vm/mmap_min_addr) and attempt to create the mapping there.  If another mapping
       already exists there, the kernel picks a new address that may or may not depend on the hint.  The address of the new mapping is returned as the result of the call.

       The contents of a file mapping (as opposed to an anonymous mapping; see MAP_ANONYMOUS below), are initialized using length bytes starting at offset offset in the file (or other object) referred to by the file descriptor  fd.
       offset must be a multiple of the page size as returned by sysconf(_SC_PAGE_SIZE).

       After the mmap() call has returned, the file descriptor, fd, can be closed immediately without invalidating the mapping.

       The prot argument describes the desired memory protection of the mapping (and must not conflict with the open mode of the file).  It is either PROT_NONE or the bitwise OR of one or more of the following flags: PROT_READ, PROT_WRITE, PROT_EXEC, PROT_GROWSDOWN, PROT_GROWSUP.

       