#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int main(int argc, char** argv)
{
    int fd;
    char buf[64] = "12345";

    fd = open("/dev/hello_misc", O_RDWR);
    if(fd < 0)
    {
        perror("open error\n");
        return fd;
    }

    // read(fd, buf, sizeof(buf));
    // printf("buf is %s\n", buf);

    write(fd, buf, sizeof(buf));
    close(fd);
    return 0;
}