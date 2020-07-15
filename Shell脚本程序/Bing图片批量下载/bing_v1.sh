###
 # @Copyright: Copyright (c) 2020 苇名一心 All Rights Reserved.
 # @Description: 下载bing 图库中图片
 # @Version: v1.0
 # @Author: 苇名一心
 # @Date: 2020-05-01 08:54:37
 # @LastEditors: 苇名一心
 # @LastEditTime: 2020-05-05 21:55:22
 ###

# 错误代号:
# 1: 参数输入格式错误
# 2: 输入的页数非整数
# 3: 输入的页数范围不在[1,126]
# 4: 开始页数<结束页数
# 5: 获取下载页面失败
# 6: 获取下载图片失败

NONE="\e[0m"
RED="\e[0;31m"
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
    start_page=$1
    end_page=$2
else
    printf $RED'Error: Please input the right arguments\n'$NONE
    echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
    echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
    echo "   or: $0 (with no arguments)      download photos in pages from 1 to 126 by default"
    exit 1
fi
# 输入的页数非整数
if ! expr $start_page + 0 > /dev/null 2>&1 || ! expr $end_page + 0 > /dev/null 2>&1
then
    printf $RED'Error: Your input page is not an integer\n'$NONE
    echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
    echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
    echo "   or: $0 (with no arguments)      download photos in pages from 1 to 126 by default"
    exit 2
fi
# 输入的页数范围不在[1,126]
if [ $start_page -lt 1 -o $end_page -lt 1 -o $start_page -gt 126 -o $end_page -gt 126 ]
then
    printf $RED'Error: Your input page not in the range [1,126]\n'$NONE
    echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
    echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
    echo "   or: $0 (with no arguments)      download photos in pages from 1 to 126 by default"
    exit 3
fi
# 开始页数<结束页数
if [ $start_page -gt $end_page ]
then
    printf $RED'Error: Your input [start_page] > [end page]\n'$NONE
    echo "Usage: $0 [start page] [end page]  download photos in pages from [start page] to [end page]"
    echo "   or: $0 [end page]               download photos in pages from [1] to [end page]"
    echo "   or: $0 (with no arguments)      download photos in pages from 1 to 126 by default"
    exit 4
fi

echo "Your input page range is [$start_page,$end_page]"
# 判断并创建目录
if [ ! -d bing ]
then
    mkdir bing
    echo "Work directory bing is created"
fi
# 缓存目录这里使用当前进程ID号作为区分，防止受到影响，下面下载图片使用$$.jpg作为缓存也是同样的道理
if [ ! -d bing/html_$$ ]
then
    mkdir bing/html_$$
    echo "Cache directory bing/html_$$ is created"
fi
# 对页范围内的循环下载
for i in `seq $start_page $end_page`
do
    # 判断是否已经下载
    if [ ! -f bing/html_$$/$i.html ]
    then
        echo "Download page $i……"
        if wget -O bing/html_$$/$i.html "https://bing.ioliu.cn/?p=$i" > /dev/null 2>&1
        then
            echo "Page $i download successfully"
        else
            if [ -f bing/html_$$/$i.html ]
            then
                rm bing/html_$$/$i.html
            fi
            printf $RED"Error: Page $i download failed\n"$NONE
            exit 5
        fi
    else
        echo "Page $i already exists"
    fi
    # 获取该页所以图片信息
    name_list=`cat bing/html_$$/$i.html | sed 's/<[^<>]*>/\n/g' | awk '/©/ { print $0 }' | sed 's/(.*)//g'`
    date_list=`cat bing/html_$$/$i.html | sed 's/<[^<>]*>/\n/g' | awk '/[0-9][0-9]*-[0-9][0-9]*-[0-9][0-9]*/ { print $0 }'`
    url_list=`cat bing/html_$$/$i.html | sed 's/src="/\n/g' | awk '/^http:/ { print $1 }' | sed -e 's/"><a//g' -e 's/640x480/1920x1080/g'`
    # 对该页内图片循环下载
    for j in `seq 1 12`
    do
        # 获取图片信息
        name=`echo "$name_list" | awk "NR==$j"`
        date=`echo "$date_list" | awk "NR==$j"`
        url=`echo "$url_list" | awk "NR==$j"`
        file_name="$date $name.jpg"
        # 清理残留缓存
        if [ -f "bing/$$.jpg" ]
        then
            rm "bing/$$.jpg"
        fi
        # 判断是否已经下载
        if [ ! -f "bing/$file_name" ]
        then
            echo "Download photo $i-$j……"
            if wget -O bing/$$.jpg "$url" > /dev/null 2>&1
            then
                mv "bing/$$.jpg" "bing/$file_name"
                echo "Photo $i-$j download successfully"
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
if [ -d bing/html_$$ ]
then
    rm -r bing/html_$$
fi
echo "All done,Thanks for using!"