#include <stdio.h>

typedef struct{
    int a;
    int b;
    char c;
    char d[32];
}MSG;

int main(int argc, char const *argv[])
{
    //使用fwrite向文件写入数据
    FILE *fp;
    if((fp = fopen("file.txt", "w")) == NULL)
    {
        perror("fail to fopen");
        return -1;
    }

    //写入字符串
    //char buf[] = "1234567890";
    //fwrite(buf, 2, 5, fp);
    
    //写入整数
    //int a = 97868;
    //fwrite(&a, 4, 1, fp);

    //写入数组
    //int a[4] = {100, 200, 300, 400};
    //fwrite(a, 4, 4, fp);

    //写入结构体
    MSG msg = {666, 888, 'w', "zhangsan"};
    fwrite(&msg, sizeof(msg), 1, fp);

    return 0;
}
