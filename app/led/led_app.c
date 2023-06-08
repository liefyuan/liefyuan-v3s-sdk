#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    int fd;
    char* filename=NULL;
    int val;
    filename = argv[1];
    fd = open(filename, O_RDWR);//打开 dev/设备文件
    if (fd < 0)//小于 0 说明没有成功
    {
        printf("error, can't open %s\n", filename);
        return 0;
    }
    if(argc !=3)
    {
        printf("usage: ./led_dev.exe [device] [on/off]\n"); //打印用法
    }
    if(!strcmp(argv[2], "on")) //如果输入等于 on，则 LED 亮
        val = 0;
    else if(!strcmp(argv[2], "off")) //如果输入等于 off，则 LED 灭
        val = 1;
    else
        goto error;

    write(fd, &val, 4);//操作 LED
    close(fd);
    return 0;
    error:
    printf("usage: ./led_dev.exe [device] [on/off]\n"); //打印用法
    close(fd);
    return -1;
}