#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

/**
 * file name：keyirqApp
 * date: 2022-10-24  23:18
 * version：1.0
 * author:liefyuan
 * describe：按键中断测试APP
 * 执行命令：./irqApp 读取按键值 
 */


/* 主程序 */
int main(int argc, char *argv[])
{
    char *filename;  // 可执行文件名
    int fd,ret;  //  fd: 文件句柄 ret:函数操作返回值
    unsigned char  keyvalue; // 按键值 


    /* 先判断输入的参数 */
    if(argc !=  2){  // 本身文件名带1个 执行文件1个  
       printf("parameter error!\r\n");
       return -1;
    }

    /* 分析参数 ，提取有用的信息 */
    filename = argv[1];  // 可执行文件名 
    
    /* 打开key文件 */
    fd = open(filename, O_RDWR);  // 可读可写 
    if(fd < 0){
        printf("can't open file:%s\r\n",filename);
        return -1;
    }

    /* 循环读取按键值 */
    while(1){
        ret = read(fd, &keyvalue, sizeof(keyvalue));
        if(ret  >= 0 ){   /* 数据有效   */
            if(keyvalue)  /* 读取到数据 */
                printf("key0 press, value = %#X\r\n", keyvalue);  // 打印出按键值 
        }else{  // 数据无效 

        }
    }

    /* 关闭文件 */
    ret = close(fd);
    if(ret < 0){
        printf("can't close file %s \r\n", filename);
        return -1;
    }

    return 0;
}


