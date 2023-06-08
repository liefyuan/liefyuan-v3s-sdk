#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    char buff[1024];
    int fd1, fd2;
    int ret;
    printf("Welcome to io application!\r\n");
    /* 打开源文件 src_file(只读方式) */
    fd1 = open("./src_file", O_RDONLY);
    if (-1 == fd1)
    {
        printf("open file src fail\r\n");
        return fd1;
    }
    else
    {
        printf("open file src_file ok!\r\n");
    }
        
    /* 打开目标文件 dest_file(只写方式) */
    fd2 = open("./dest_file", O_WRONLY);
    if (-1 == fd2) 
    {
        printf("open file dest fail\r\n");
        ret = fd2;
        goto out1;
    }
    else
    {
        printf("open file dest_file ok!\r\n");
    }
    
    /* 读取源文件 1KB 数据到 buff 中 */
    ret = read(fd1, buff, sizeof(buff));
    if (-1 == ret)
        goto out2;
    /* 将 buff 中的数据写入目标文件 */
    ret = write(fd2, buff, sizeof(buff));
    if (-1 == ret)
        goto out2;
    ret = 0;
    out2:
    /* 关闭目标文件 */
    close(fd2);
    out1:
    /* 关闭源文件 */
    close(fd1);
    return ret;
}

 