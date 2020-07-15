/*
 * @Copyright: Copyright (c) 2020 苇名一心 All Rights Reserved.
 * @Description: 列表Linux文件，在Ubuntu 18.04上进行的编程
 * @Version: 1.0
 * @Author: 苇名一心
 * @Date: 2020-04-09 17:57:29
 * @LastEditors: 苇名一心
 * @LastEditTime: 2020-04-13 14:08:50
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

//颜色
#define NONE "\e[0m"      //复原
#define RED "\e[0;31m"    //ERROR
#define YELLOW "\e[0;33m" //目录
#define CYAN "\e[0;36m"   //除普通文件和目录文件外的文件
#define OPTION_NUM 7      //选项种类数目（除'--'以外）

//****************选项枚举****************
enum OPT
{
    _r,    //-r 递归方式列出子目录
    _a,    //-a 列出文件名第一个字符为圆点的普通文件
    _d,    //-d 展示文件的详细信息（默认只展示文件路径）
    _l,    //-l 后跟一整数，限定文件大小的最小值（字节）
    _h,    //-h 后跟一整数，限定文件大小的最大值（字节）
    _m,    //-m 后跟一整数n，限定文件的最近修改时间必须在n天内
    _help, //-? 显示帮助
} opt;
//-- 显式地终止命令选项分析
//**************************************

char split[] = "****************************************\n"; //分割线

char version[] = "1.0";     //版本号
char **path;                //路径
int path_num = 0;           //用于记录路径的数目
int opts[OPTION_NUM] = {0}; //用来存储选项是否开启，数组的每个位置对应相应选项，值为0表示未开启，为1表示开启
long max_size = 0;          //最大文件尺寸
long min_size = 0;          //最小文件尺寸
time_t limit_day = 0;       //最早修改在几天前
time_t current_time = 0;    //当前系统时间

void free_path();                                 //释放path分配的空间
void help();                                      //打印帮助信息
void analyse_input(int argc, char const *argv[]); //分析输入的命令的函数
void show_analyse_result();                       //输出命令解析的结果
void run(char *path);                             //对一个路径按照选项运行命令
void show_mode(mode_t mode);                      //输出文件的权限及文件类型（用于-d选项中）
void show(struct stat st, char *path);            //展示文件信息

int main(int argc, char const *argv[])
{
    time(&current_time);       //获取指令执行时的时间
    analyse_input(argc, argv); //分析用户的输入
    //用户加了帮助选项，输出帮助信息后退出
    if (opts[_help])
    {
        help();
        exit(EXIT_SUCCESS);
    }
    //用户没有帮助选项，输出命令解析的结果并运行该命令
    else
    {
        show_analyse_result();
        int i = 0;
        for (i = 0; i < path_num; i++)
        {
            run(path[i]);
        }
    }
    free_path();
    return 0;
}

/**
 * @name: free_path
 * @author: 苇名一心
 * @msg: 释放path分配的空间
 * @param: 无
 * @return: 无
 */
void free_path()
{
    int i = 0;
    for (i = 0; i < path_num; i++)
        free(path[i]);
    free(path);
}

/**
 * @name: help
 * @author: 苇名一心
 * @msg: 打印帮助信息
 * @param: 无
 * @return: 无
 */
void help()
{
    printf("%18sHELP%18s\n%s", "", "", split);
    printf("List %s by 苇名一心 Apr 10 2020\n", version);
    printf("Usage: ./list [OPTION]... [PATH]...\n");
    printf("List informaton about the FILEs in these PATHs(the current directory by default)\nOptions:\n");
    printf("\t-r\t\tList subdirectories recursively\n");
    printf("\t-a\t\tDo not hide entries starting with .\n");
    printf("\t-d\t\tShow detail information of file(only file path by default)\n");
    printf("\t-l <bytes>\tMinmum of file size\n");
    printf("\t-h <bytes>\tMaxmum of file size\n");
    printf("\t-m <days>\tLimit file last changed time\n");
    printf("\t-?\t\tDisplay this help and exit\n");
    printf("Color:\n");
    printf("\tWHITE: Regular file\n");
    printf(YELLOW "\tYELLOW: Directory file\n" NONE);
    printf(CYAN "\tCYAN: Other file\n" NONE);
    printf(RED "\tRED: Error\n" NONE);
    printf("%s", split);
}

/**
 * @name: analyse_input
 * @author: 苇名一心
 * @msg: 分析输入的命令的函数
 * @param: 同main函数
 * @return: 无
 */
void analyse_input(int argc, char const *argv[])
{
    path = (char **)malloc(argc * sizeof(char *));
    int i = 1;
    int flag = 1; //记录选项分析是否终止，终止为0，未终止为1
    for (i = 1; i < argc; i++)
    {
        //以-开头的是选项
        if (flag && argv[i][0] == '-')
        {
            if (strlen(argv[i]) == 1)
            {
                printf(RED "ERROR: Input option wrong, please input the right option\n" NONE);
                printf("Here are some tips:\n");
                help();
                free_path();
                exit(EXIT_FAILURE);
            }
            else if (strlen(argv[i]) == 2)
            {
                switch (argv[i][1])
                {
                case 'r':
                    opts[_r] = 1;
                    break;

                case 'a':
                    opts[_a] = 1;
                    break;

                case 'd':
                    opts[_d] = 1;
                    break;

                case 'l':
                    opts[_l] = 1;
                    //判断-l选项后面有没有参数
                    if (i < argc - 1)
                    {
                        char *stop;
                        long int size = strtol(argv[++i], &stop, 10);
                        //stop长度为0代表进行了有效的转换
                        if (strlen(stop) == 0)
                        {
                            min_size = size;
                        }
                        else
                        {
                            printf(RED "ERROR: Input option -l wrong, there is no required min size, please input the right option\n" NONE);
                            printf("Here are some tips:\n");
                            help();
                            free_path();
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        printf(RED "ERROR: Input option -l wrong, there is no required min size, please input the right option\n" NONE);
                        printf("Here are some tips:\n");
                        help();
                        free_path();
                        exit(EXIT_FAILURE);
                    }
                    break;

                case 'h':
                    opts[_h] = 1;
                    //判断-h选项后面有没有参数
                    if (i < argc - 1)
                    {
                        char *stop;
                        long int size = strtol(argv[++i], &stop, 10);
                        //stop长度为0代表进行了有效的转换
                        if (strlen(stop) == 0)
                        {
                            max_size = size;
                        }
                        else
                        {
                            printf(RED "ERROR: Input option -h wrong, there is no required max size, please input the right option\n" NONE);
                            printf("Here are some tips:\n");
                            help();
                            free_path();
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        printf(RED "ERROR: Input option -h wrong, there is no required max size, please input the right option\n" NONE);
                        printf("Here are some tips:\n");
                        help();
                        free_path();
                        exit(EXIT_FAILURE);
                    }
                    break;

                case 'm':
                    opts[_m] = 1;
                    //判断-m选项后面有没有参数
                    if (i < argc - 1)
                    {
                        char *stop;
                        long int day = strtol(argv[++i], &stop, 10);
                        //stop长度为0代表进行了有效的转换
                        if (strlen(stop) == 0)
                        {
                            limit_day = day;
                        }
                        else
                        {
                            printf(RED "ERROR: Input option -m wrong, there is no required limit day, please input the right option\n" NONE);
                            printf("Here are some tips:\n");
                            help();
                            free_path();
                            exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        printf(RED "ERROR: Input option -m wrong, there is no required limit day, please input the right option\n" NONE);
                        printf("Here are some tips:\n");
                        help();
                        free_path();
                        exit(EXIT_FAILURE);
                    }
                    break;

                case '?':
                    opts[_help] = 1;
                    break;

                case '-':
                    flag = 0;
                    break;

                default:
                    printf(RED "ERROR: Input option wrong, please input the right option\n" NONE);
                    printf("Here are some tips:\n");
                    help();
                    free_path();
                    exit(EXIT_FAILURE);
                    break;
                }
            }
            else
            {
                printf(RED "ERROR: Input option wrong, please input the right option\n" NONE);
                printf("Here are some tips:\n");
                help();
                free_path();
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            //不是-开头的认为是路径或者--后面的全部认为是路径
            path[path_num] = (char *)malloc(strlen(argv[i]) * sizeof(char));
            strcpy(path[path_num++], argv[i]);
        }
    }
    if (opts[_l] && opts[_h])
    {
        if (min_size > max_size)
        {
            printf(RED "ERROR: Min size of file is less than Max size of file, please input the right option\n" NONE);
            printf("Here are some tips:\n");
            help();
            free_path();
            exit(EXIT_FAILURE);
        }
    }
    //没有输入路径，默认为当前工作目录
    if (path_num == 0)
    {
        path[path_num] = (char *)malloc(sizeof(char) * 255);
        if (!getcwd(path[path_num++], sizeof(char) * 255))
        {
            printf(RED "ERROR: Get current work path wrong\n" NONE);
            free_path();
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * @name: show_analyse_result
 * @author: 苇名一心
 * @msg: 输出命令解析的结果
 * @param: 无
 * @return: 无
 */
void show_analyse_result()
{
    printf("Input Analyse Results are:\n");
    int i = 0;
    for (i = 0; i < OPTION_NUM; i++)
    {
        switch (i)
        {
        case _r:
            if (opts[i])
            {
                printf("-r: True\n");
            }
            else
            {
                printf("-r: False\n");
            }
            break;

        case _a:
            if (opts[i])
            {
                printf("-a: True\n");
            }
            else
            {
                printf("-a: False\n");
            }
            break;

        case _d:
            if (opts[i])
            {
                printf("-d: True\n");
            }
            else
            {
                printf("-d: False\n");
            }
            break;

        case _l:
            if (opts[i])
            {
                printf("-l: True\tMin Size:%ld\n", min_size);
            }
            else
            {
                printf("-l: False\n");
            }
            break;

        case _h:
            if (opts[i])
            {
                printf("-h: True\tMax Size:%ld\n", max_size);
            }
            else
            {
                printf("-h: False\n");
            }
            break;

        case _m:
            if (opts[i])
            {
                printf("-m: True\tLimit Day:%ld\n", limit_day);
            }
            else
            {
                printf("-m: False\n");
            }
            break;

        case _help:
            if (opts[i])
            {
                printf("-?: True\n");
            }
            else
            {
                printf("-?: False\n");
            }
            break;

        default:
            printf(RED "ERROR: Unknown error, please consult me\n" NONE);
            break;
        }
    }
    printf("%s", split);
    printf("Path Count is: %d\nPath:\n", path_num);
    for (i = 0; i < path_num; i++)
    {
        printf("%s\n", path[i]);
    }
    printf("%s", split);
}

/**
 * @name: run
 * @msg: 对一个路径按照选项运行命令
 * @param： path为这个路径
 * @return: 无
 */
void run(char *path)
{
    struct stat st;         //stat结构体
    DIR *dir;               //目录句柄
    struct dirent *entry;   //dirent结构体
    char current_path[255]; //当前的路径，最长255
    //note:一定要用lstat，否则可能会出现有些文件打不开的情况，下面同理
    int ret = lstat(path, &st); //取出当前路径的i节点信息
    //成功获取stat
    if (ret == 0)
    {
        //如果是目录
        if (S_ISDIR(st.st_mode))
        {
            //打开目录
            dir = opendir(path);
            //打开失败
            if (dir == NULL)
            {
                printf(RED "ERROR[%d] open dir \"%s\": %s\n" NONE, errno, path, strerror(errno));
            }
            //打开成功
            else
            {
                entry = readdir(dir);
                //只要没有读到目录尾就一直读
                while (entry != NULL)
                {
                    //去掉目录中的.和..
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                    {
                        //构造当前目录下各文件路径
                        strcpy(current_path, path);
                        //忽略路径最后一个字符为‘/’
                        if (current_path[strlen(current_path) - 1] != '/')
                        {
                            strcat(current_path, "/");
                        }
                        strcat(current_path, entry->d_name);
                        //获取当前目录下面各文件的stat
                        ret = lstat(current_path, &st);
                        //成功获取stat
                        if (ret == 0)
                        {
                            //输出
                            //是否显示.开头的文件(包括目录文件)
                            if (opts[_a] || entry->d_name[0] != '.')
                            {
                                show(st, current_path);
                                //如果还是目录并且指定了选项-r，则进行递归
                                //也就是说current_path一定是目录
                                if (S_ISDIR(st.st_mode) && opts[_r])
                                {
                                    run(current_path);
                                }
                            }
                        }
                        //失败输出错误信息
                        else
                        {
                            printf(RED "ERROR[%d] read file \"%s\": %s\n" NONE, errno, current_path, strerror(errno));
                        }
                    }
                    //继续读下一个
                    entry = readdir(dir);
                }
                closedir(dir);
            }
        }
        //如果是文件，直接打印
        else
        {
            show(st, path);
        }
    }
    //失败输出错误信息
    else
    {
        printf(RED "ERROR[%d] read file \"%s\": %s\n" NONE, errno, path, strerror(errno));
    }
}

/**
 * @name: show_mode
 * @author: 苇名一心
 * @msg: //输出文件的权限及文件类型（用于-d选项中）
 * @param：mode_t mode，文件的mode 
 * @return: 无
 */
void show_mode(mode_t mode)
{
    int pos = 0;
    char str[] = "----------";
    if (S_ISREG(mode))
        str[pos] = '-'; //普通文件
    else if (S_ISBLK(mode))
        str[pos] = 'b'; //块设备文件
    else if (S_ISDIR(mode))
        str[pos] = 'd'; //目录文件
    else if (S_ISCHR(mode))
        str[pos] = 'c'; //字符设备文件
    else if (S_ISLNK(mode))
        str[pos] = 'l'; //符号连接文件
    else if (S_ISFIFO(mode))
        str[pos] = 'p'; //命名管道文件
    //按位取与，获得文件所有者权限
    pos++;
    if (S_IRUSR & mode)
        str[pos] = 'r';
    pos++;
    if (S_IWUSR & mode)
        str[pos] = 'w';
    pos++;
    if (S_IXUSR & mode)
        str[pos] = 'x';
    //按位取与，获得同组用户权限
    pos++;
    if (S_IRGRP & mode)
        str[pos] = 'r';
    pos++;
    if (S_IWGRP & mode)
        str[pos] = 'w';
    pos++;
    if (S_IXGRP & mode)
        str[pos] = 'x';
    //按位取与，获得其他用户权限
    pos++;
    if (S_IROTH & mode)
        str[pos] = 'r';
    pos++;
    if (S_IWOTH & mode)
        str[pos] = 'w';
    pos++;
    if (S_IXOTH & mode)
        str[pos] = 'x';
    printf("%s", str);
}

/**
 * @name: show
 * @author: 苇名一心
 * @msg: 展示文件信息
 * @param: st为文件stat结构体，path为文件路径 
 * @return: 无
 */
void show(struct stat st, char *path)
{
    //是否对文件大小最小值有要求
    //如果opts[_l]为1，说明有要求，!opts[_l]=0
    //0||st.st_size > min_size=st.st_size > min_size
    //因此刚好可以限制文件大小
    //如果opts[_l]为0，说明没有要求，!opts[_l]=1
    //1||st.st_size > min_size=1
    //因此限制无效，直接通过，没有限制文件大小
    //以下其他命令同理
    if (!opts[_l] || st.st_size >= min_size)
    {
        //是否对文件大小最大值有要求
        if (!opts[_h] || st.st_size <= max_size)
        {
            //是否对文件修改时间有要求
            //st_mtime域代表了文件的最近改变时间
            //((current_time - st.st_mtime) / (24 * 60 * 60))可以得到天数
            if (!opts[_m] || ((current_time - st.st_mtime) / (24 * 60 * 60)) < limit_day)
            {
                if (opts[_d])
                {
                    show_mode(st.st_mode);
                    struct passwd *user;
                    user = getpwuid(st.st_uid);
                    struct group *group;
                    group = getgrgid(st.st_gid);
                    char *time_str;
                    time_str = ctime(&st.st_ctime);
                    //将日期最后的'\n'去掉
                    time_str[strlen(time_str) - 1] = '\0';
                    if (S_ISDIR(st.st_mode))
                    {
                        printf("%5lu  %-10s%-10s%-15ld%s  " YELLOW "%s\n" NONE,
                               st.st_nlink, user->pw_name, group->gr_name, st.st_size, time_str, path);
                    }
                    else if (S_ISREG(st.st_mode))
                    {
                        printf("%5lu  %-10s%-10s%-15ld%s  %s\n",
                               st.st_nlink, user->pw_name, group->gr_name, st.st_size, time_str, path);
                    }
                    else
                    {
                        printf("%5lu  %-10s%-10s%-15ld%s  " CYAN "%s\n" NONE,
                               st.st_nlink, user->pw_name, group->gr_name, st.st_size, time_str, path);
                    }
                }
                else
                {
                    if (S_ISDIR(st.st_mode))
                    {
                        printf(YELLOW "%s\n" NONE, path);
                    }
                    else if (S_ISREG(st.st_mode))
                    {
                        printf("%s\n", path);
                    }
                    else
                    {
                        printf(CYAN "%s\n" NONE, path);
                    }
                }
            }
        }
    }
}