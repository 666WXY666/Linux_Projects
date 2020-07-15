/*
 * @Copyright: Copyright (c) 2020 苇名一心 All Rights Reserved.
 * @Description: grep -v usr < /etc/passwd | wc –l > r.txt; cat r.txt
 * @Version: 1.0
 * @Author: 苇名一心
 * @Date: 2020-05-21 09:44:26
 * @LastEditors: 苇名一心
 * @LastEditTime: 2020-05-21 21:23:57
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    int sv;
    int fd[2];

    pipe(fd);
    if (fork() == 0)
    {
        // 输入重定向
        int fd0 = -1;
        fd0 = open("/etc/passwd", O_RDONLY);
        if (fd0 != -1)
        {
            dup2(fd0, 0);
            close(fd0);
        }
        // 管道输入端
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]);
        // 执行管道前的命令
        execlp("grep", "grep", "-v", "usr", 0);
        exit(1);
    }
    else
    {
        if (fork() == 0)
        {
            // 输出重定向
            int fd1 = -1;
            fd1 = open("r.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
            if (fd1 != -1)
            {
                dup2(fd1, 1);
                close(fd1);
            }
            // 管道输出端
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]);
            // 执行管道后的命令
            execlp("wc", "wc", "-l", 0);
            exit(1);
        }
    }

    close(fd[0]);
    close(fd[1]);
    wait(&sv);
    wait(&sv);
    // 执行第二个命令
    execlp("cat", "cat", "r.txt", 0);
    return 0;
}
