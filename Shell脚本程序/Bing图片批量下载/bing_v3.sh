###
 # @Copyright: Copyright (c) 2020 苇名一心 All Rights Reserved.
 # @Description: 下载bing 图库中图片
 # @Version: v3.0
 # @Author: 苇名一心
 # @Date: 2020-05-01 08:54:37
 # @LastEditors: 苇名一心
 # @LastEditTime: 2020-05-05 21:54:18
 ###

# 错误代号:
# 1: 参数输入格式错误
# 2: 输入的页数非整数
# 3: 输入的页数范围不在[1,126]
# 4: 开始页数<结束页数
# 5: 下载页面失败
# 6: 下载图片失败
# 7: 输入的随机次数非整数
# 8: 输入的随机次数<1
# 9: 下载json失败
# 10: 未知错误

NONE="\e[0m"
RED="\e[0;31m"
# rand=0，代表普通模式；rand>0代表随机模式，rand的数值代表随机的次数
rand=0
# 读取命令行参数，并进行错误处理
if [ $# = 0 ]
then
    start_page=1
    end_page=126
# 额外增加了一个，如果只输入一个参数，代表从第1页到输入的参数的页数
elif [ $# = 1 ]
then
    start_page=1
    end_page=$1
elif [ $# = 2 ]
then
    # 随机模式
    if [ $1 = 'rand' ]
    then
        # 判断第二个参数是否为整数
        if expr $2 + 0 > /dev/null 2>&1
        then
            # 输入的随机次数<1
            if [ $2 -lt 1 ]
            then
                printf $RED'Error: Your input [rand times] < 1\n'$NONE
                echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
                echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
                echo "   or: $0 (with no arguments)      download photos in pages from [1] to [126] by default"
                echo "   or: $0 rand [rand times]          rand download photos of [rand times]"
                exit 8
            else
                rand=$2
            fi
        # 输入的随机次数非整数
        else
            printf $RED'Error: Your input [rand times] is not an integer\n'$NONE
            echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
            echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
            echo "   or: $0 (with no arguments)      download photos in pages from [1] to [126] by default"
            echo "   or: $0 rand [rand times]          rand download photos of [rand times]"
            exit 7
        fi
    # 普通模式
    else
        start_page=$1
        end_page=$2
        # 输入的页数非整数
        if ! expr $start_page + 0 > /dev/null 2>&1 || ! expr $end_page + 0 > /dev/null 2>&1
        then
            printf $RED'Error: Your input page is not an integer\n'$NONE
            echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
            echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
            echo "   or: $0 (with no arguments)      download photos in pages from [1] to [126] by default"
            echo "   or: $0 rand [rand times]        rand download photos of [rand times]"
            exit 2
        fi
        # 输入的页数范围不在[1,126]
        if [ $start_page -lt 1 -o $end_page -lt 1 -o $start_page -gt 126 -o $end_page -gt 126 ]
        then
            printf $RED'Error: Your input page not in the range [1,126]\n'$NONE
            echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
            echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
            echo "   or: $0 (with no arguments)      download photos in pages from [1] to [126] by default"
            echo "   or: $0 rand [rand times]        rand download photos of [rand times]"
            exit 3
        fi
        # 开始页数<结束页数
        if [ $start_page -gt $end_page ]
        then
            printf $RED'Error: Your input [start_page] > [end page]\n'$NONE
            echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
            echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
            echo "   or: $0 (with no arguments)      download photos in pages from [1] to [126] by default"
            echo "   or: $0 rand [rand times]        rand download photos of [rand times]"
            exit 4
        fi
    fi
# 参数输入格式错误
else
    printf $RED'Error: Please input the right arguments\n'$NONE
    echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
    echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
    echo "   or: $0 (with no arguments)      download photos in pages from [1] to [126] by default"
    echo "   or: $0 rand [rand times]        rand download photos of [rand times]"
    exit 1
fi

# 判断并创建工作目录
if [ ! -d bing ]
then
    mkdir bing
    echo "Work directory bing is created"
fi

# 普通模式
if [ $rand -eq 0 ]
then
    echo "##############Regular Mode##############"
    echo "Your input page range is [$start_page,$end_page]"
    # 对页范围内的循环下载
    for i in `seq $start_page $end_page`
    do
        # 缓存文件使用当前进程ID号$$.html命名，防止受到影响，下面下载图片使用$$.jpg作为缓存也是同样的道理
        echo "Download page $i……"
        if wget -O bing/$$.html "https://bing.ioliu.cn/?p=$i" > /dev/null 2>&1
        then
            echo "Page $i download successfully"
        else
            if [ -f bing/$$.html ]
            then
                rm bing/$$.html
            fi
            printf $RED"Error: Page $i download failed\n"$NONE
            exit 5
        fi
        # 获取该页所以图片信息
        name_list=`cat bing/$$.html | sed 's/<[^<>]*>/\n/g' | awk '/©/ { print $0 }' | sed 's/(.*)//g'`
        date_list=`cat bing/$$.html | sed 's/<[^<>]*>/\n/g' | awk '/[0-9][0-9]*-[0-9][0-9]*-[0-9][0-9]*/ { print $0 }'`
        url_list=`cat bing/$$.html | sed 's/src="/\n/g' | awk '/^http:/ { print $1 }' | sed -e 's/"><a//g' -e 's/640x480/1920x1080/g'`
        # 对该页内图片循环下载
        for j in `seq 1 12`
        do
            # 获取图片信息
            name=`echo "$name_list" | awk "NR==$j"`
            date=`echo "$date_list" | awk "NR==$j"`
            url=`echo "$url_list" | awk "NR==$j"`
            file_name="$date $name.jpg"
            # 判断是否已经下载
            if [ ! -f "bing/$file_name" ]
            then
                echo "Download photo $i-$j……"
                if wget -O bing/$$.jpg "$url" > /dev/null 2>&1
                then
                    same=0
                    # 先判断md5文件是否存在，如果不存在，说明bing空目录，直接改名
                    if [ -f "bing/md5.txt" ]
                    then
                        # 文件名不存在的，判断是否有文件大小相同的和md5相同的
                        cat bing/md5.txt | while read line
                        do
                            # 先比较文件大小是否相同，加快速度
                            if [ `ls -l "bing/$$.jpg" | awk '{ print $5 }'` = `echo "$line" | awk '{ print $1 }'` ]
                            then
                                # md5sum计算文件md5值，进行比较
                                if [ `md5sum "bing/$$.jpg" | awk '{ print $1 }'` = `echo "$line" | awk '{ print $2 }'` ]
                                then
                                    # 相同
                                    same=1
                                    break
                                fi
                            fi
                        done
                    fi
                    if [ $same = 1 ]
                    then
                        rm "bing/$$.jpg"
                        echo "Photo $i-$j already exists"
                    else
                        mv "bing/$$.jpg" "bing/$file_name"
                        echo "Photo $i-$j download successfully"
                        # 生成文件大小和MD5校验信息
                        echo "`ls -l "bing/$file_name" | awk '{ print $5 }'` "`md5sum "bing/$file_name"` >> bing/md5.txt
                    fi
                else
                    printf $RED"Error: Photo $i-$j download failed\n"$NONE
                    exit 6
                fi
            else
                echo "Photo $i-$j already exists"
            fi
        done
    done
    # 清理缓存
    echo "Cleaning……"
    rm bing/$$.html
# 随机模式
elif [ $rand -gt 0 ]
then
    echo "##############Random Mode##############"
    echo "Your input rand times is $rand"
    # 循环随机下载
    for k in `seq 1 $rand`
    do
        # 获取json
        if wget -O "bing/$$.json" "https://bing.ioliu.cn/v1/rand?type=json" > /dev/null 2>&1
        then
            # 获取图片信息
            name=`cat bing/$$.json | sed 's/.*"copyright":"\(.*\)(.*©.*)"},.*/\1/g'`
            date=`cat bing/$$.json | sed 's/.*"enddate":"\([0-9][0-9][0-9][0-9]\)\([0-9][0-9]\)\([0-9][0-9]\)",.*/\1-\2-\3/g'`
            url=`cat bing/$$.json | sed 's/.*"url":"\(http:.*imageslim\)",.*/\1/g'`
            file_name="$date $name.jpg"
            # 判断是否已经下载，先用文件名判断
            if [ ! -f "bing/$file_name" ]
            then
                echo "Download photo $k……"
                if wget -O "bing/$$.jpg" "$url" > /dev/null 2>&1
                then
                    same=0
                    # 先判断md5文件是否存在，如果不存在，说明bing空目录，直接改名
                    if [ -f "bing/md5.txt" ]
                    then
                        # 文件名不存在的，判断是否有文件大小相同的和md5相同的
                        cat bing/md5.txt | while read line
                        do
                            # 先比较文件大小是否相同，加快速度
                            if [ `ls -l "bing/$$.jpg" | awk '{ print $5 }'` = `echo "$line" | awk '{ print $1 }'` ]
                            then
                                # md5sum计算文件md5值，进行比较
                                if [ `md5sum "bing/$$.jpg" | awk '{ print $1 }'` = `echo "$line" | awk '{ print $2 }'` ]
                                then
                                    # 相同
                                    same=1
                                    break
                                fi
                            fi
                        done
                    fi
                    if [ $same = 1 ]
                    then
                        rm "bing/$$.jpg"
                        echo "Photo $k already exists"
                    else
                        mv "bing/$$.jpg" "bing/$file_name"
                        echo "Photo $k download successfully"
                        # 生成文件大小和MD5校验信息
                        echo "`ls -l "bing/$file_name" | awk '{ print $5 }'` "`md5sum "bing/$file_name"` >> bing/md5.txt
                    fi
                else
                    printf $RED"Error: Photo $k download failed\n"$NONE
                    exit 6
                fi
            else
                echo "Photo $k already exists"
            fi
        else
            if [ -f bing/$$.json ]
            then
                rm bing/$$.json
            fi
            printf $RED"Error: Json $k download failed\n"$NONE
            exit 9
        fi
    done
    # 清理缓存
    echo "Cleaning……"
    rm bing/$$.json
# 未知错误
else
    printf $RED"Error: Unkonwn error\n"$NONE
    exit 10
fi
echo "All done,Thanks for using!"