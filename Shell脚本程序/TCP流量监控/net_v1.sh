###
 # @Copyright: Copyright (c) 2020 苇名一心 All Rights Reserved.
 # @Description: 生成TCP 活动状况报告
 # @Version: v1.0
 # @Author: 苇名一心
 # @Date: 2020-04-30 11:26:28
 # @LastEditors: 苇名一心
 # @LastEditTime: 2020-05-01 23:19:04
 ###

# 获取时间
date_time=`date '+%Y-%m-%d %H:%M'`
# 这一分钟开始时的TCP报文数
data1=`netstat --statistics | grep '[0-9][0-9]* segments [rs][e][cn]'`
# 发送
send1=`echo $data1 | awk '{print $4}'`
# 接收
recv1=`echo $data1 | awk '{print $1}'`
# 等60s
sleep 60
# 这一分钟结束时的TCP报文数
data_temp=`netstat --statistics | grep '[0-9][0-9]* segments [rs][e][cn]'`
# 发送
send_temp=`echo $data_temp | awk '{print $4}'`
# 接收
recv_temp=`echo $data_temp | awk '{print $1}'`
# 这一分钟内发送的TCP报文数
send=`expr $send_temp - $send1`
# 这一分钟内接收的TCP报文数
recv=`expr $recv_temp - $recv1`
# 这一分钟内收发TCP报文总数
sum_temp=`expr $send + $recv`
# 输出,第一次输出不加最后的符号
printf '%-16s%8s%8s%8s\n' "$date_time" "$send" "$recv" "$sum_temp"

while true
do
    # 获取时间
    date_time=`date '+%Y-%m-%d %H:%M'`
    # 等60s
    sleep 60
    # 一分钟后的TCP报文数
    data2=`netstat --statistics | grep '[0-9][0-9]* segments [rs][e][cn]'`
    # 发送
    send2=`echo $data2 | awk '{print $4}'`
    # 接收
    recv2=`echo $data2 | awk '{print $1}'`
    # 这一分钟内发送的TCP报文数
    send=`expr $send2 - $send_temp`
    # 这一分钟内接收的TCP报文数
    recv=`expr $recv2 - $recv_temp`
    # 这一分钟内收发TCP报文总数
    sum2=`expr $send + $recv`
    # 判断最后的符号
    # 后-前>10,为+
    if [ `expr $sum2 - $sum_temp` -gt 10 ]
    then
        sign='+'
    # 前-后>10,为-
    elif [ `expr $sum_temp - $sum2` -gt 10 ]
    then
        sign='-'
    # 其他情况,为空格
    else
        sign=' '
    fi
    # 输出
    printf '%-16s%8s%8s%8s%5s\n' "$date_time" "$send" "$recv" "$sum2" "$sign"
    # 保留上一次的数据,用于下一次比较
    send_temp=$send2
    recv_temp=$recv2
    sum_temp=$sum2
done