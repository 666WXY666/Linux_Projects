/*
 * @Copyright: Copyright (c) 2020 苇名一心 All Rights Reserved.
 * @Description: Simple Shell
 * @Version: 1.0
 * @Author: 苇名一心
 * @Date: 2020-05-21 09:44:26
 * @LastEditors: 苇名一心
 * @LastEditTime: 2020-05-21 21:56:34
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/wait.h>

// 颜色
#define NONE "\e[0m"   // 复原
#define RED "\e[0;31m" // ERROR

char version[] = "1.0"; // 版本号
int pipe_flag = 0;      // 是否使用管道
static jmp_buf env;
int sv;

// CTRL-C回调函数
void sig_handler(int sig)
{
    if (pipe_flag)
    {
        wait(&sv);
        wait(&sv);
    }
    else
    {
        wait(&sv);
    }
    printf("\n");
    siglongjmp(env, 1);
}

// 打印帮助
void help()
{
    printf("**********************************************************\n");
    printf("%27sHELP%27s\n**********************************************************\n", "", "");
    printf("Simple Shell %s by 苇名一心 May 21 2020\n", version);
    printf("Usage: [COMMAND] [OPTION]... (< [INFILE]) (| [COMMAND] [OPTION]... (> [OUTFILE])) (;...;...)\n");
    printf("Simple shell with redirect and pipe(Can also run multiple commands)\nSpecial commands:\n");
    printf("\thelp\tDisplay this help and continue\n");
    printf("\texit\tExit simple shell\n");
    printf("**********************************************************\n");
}

int main(int argc, char const *argv[])
{
    char buffer[256], *argv1[256], **p, *argv2[256], *buf, *end, *cmd2, *in, *out;
    int fd[2];
    // CTRL-C信号自行处理
    signal(SIGINT, sig_handler);

    for (;;)
    {
        sigsetjmp(env, 1);
        // 打印提示符
        printf("=> ");
        // 获取命令
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            exit(0);
        //没有输入任何命令
        if (strlen(buffer) == 1)
        {
            continue;
        }
        // 命令最后加;用于判断
        if (buffer[strlen(buffer) - 2] != ';')
        {
            buffer[strlen(buffer) - 1] = ';';
        }
        // 获取每一条;分割的命令，依次执行
        for (buf = buffer; end = strstr(buf, ";"); buf = end)
        {
            *end++ = '\0';
            pipe_flag = 0;
            // 获取管道命令，输入重定向文件和输出重定向文件
            // note:必须先获取管道命令，in和out在进行分词操作，因为分词会修改buf，导致获取管道命令，in和out出错
            cmd2 = strstr(buf, "|");
            in = strstr(buf, "<");
            out = strstr(buf, ">");
            if (in != NULL)
            {
                *in++ = '\0';
                in = strtok(in, " \t\n");
            }
            if (out != NULL)
            {
                *out++ = '\0';
                out = strtok(out, " \t\n");
            }
            if (cmd2 != NULL)
            {
                pipe_flag = 1;
                *cmd2++ = '\0';
                for (p = &argv2[0], *p = strtok(cmd2, " \t\n"); *p != NULL; *++p = strtok(NULL, " \t\n"))
                    ;
                if (argv2[0] == NULL)
                {
                    fprintf(stderr, RED "** No command 2 input!\n" NONE);
                    continue;
                }
            }
            for (p = &argv1[0], *p = strtok(buf, " \t\n"); *p != NULL; *++p = strtok(NULL, " \t\n"))
                ;
            // 没有命令输入
            if (argv1[0] == NULL)
            {
                fprintf(stderr, RED "** No command 1 input!\n" NONE);
                continue;
            }
            // 输入了exit
            if (strcmp(argv1[0], "exit") == 0)
                exit(0);
            // 输入了help
            if (strcmp(argv1[0], "help") == 0)
            {
                help();
                continue;
            }
            // 使用了管道
            if (pipe_flag)
            {
                pipe(fd);
                if (fork() == 0)
                {
                    // 输入重定向
                    int fd0 = -1;
                    if (in != NULL)
                        fd0 = open(in, O_RDONLY);
                    if (fd0 != -1)
                    {
                        dup2(fd0, 0);
                        close(fd0);
                    }
                    else if (in != NULL)
                    {
                        fprintf(stderr, RED "** No such file or directory: %s\n" NONE, in);
                        help();
                        exit(1);
                    }
                    // 管道输入端
                    dup2(fd[1], 1);
                    close(fd[1]);
                    close(fd[0]);
                    // 执行管道前的命令
                    execvp(argv1[0], argv1);
                    fprintf(stderr, RED "** Bad command 1: %m\n" NONE);
                    help();
                    exit(1);
                }
                else
                {
                    if (fork() == 0)
                    {
                        // 输出重定向
                        int fd1 = -1;
                        if (out != NULL)
                            fd1 = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0666);
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
                        execvp(argv2[0], argv2);
                        fprintf(stderr, RED "** Bad command 2: %m\n" NONE);
                        help();
                        exit(1);
                    }
                }

                close(fd[0]);
                close(fd[1]);
                wait(&sv);
                wait(&sv);
            }
            // 没有使用管道
            else
            {
                if (fork() == 0)
                {
                    int fd0 = -1, fd1 = -1;
                    // 输入重定向
                    if (in != NULL)
                        fd0 = open(in, O_RDONLY);
                    if (fd0 != -1)
                    {
                        dup2(fd0, 0);
                        close(fd0);
                    }
                    else if (in != NULL)
                    {
                        fprintf(stderr, RED "** No such file or directory: %s\n" NONE, in);
                        help();
                        exit(1);
                    }
                    // 输出重定向
                    if (out != NULL)
                        fd1 = open(out, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                    if (fd1 != -1)
                    {
                        dup2(fd1, 1);
                        close(fd1);
                    }
                    // 执行命令
                    execvp(argv1[0], argv1);
                    fprintf(stderr, RED "** Bad command 1: %m\n" NONE);
                    help();
                    exit(1);
                }

                wait(&sv);
            }
        }
    }

    return 0;
}
