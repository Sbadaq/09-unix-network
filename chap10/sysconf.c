#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    long nsems_max = sysconf(_SC_SEM_NSEMS_MAX);
    long value_max = sysconf(_SC_SEM_VALUE_MAX);
    //SEM_NSEMS_MAX = -1, SEM_VALUE_MAX = 2147483647
    if (nsems_max == -1 || value_max == -1) {
        perror("sysconf error");
        return 1;
    }

    printf("SEM_NSEMS_MAX = %ld, SEM_VALUE_MAX = %ld\n", nsems_max, value_max);
    return 0;
}