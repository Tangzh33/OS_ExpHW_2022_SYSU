#ifndef SYSCALL_H
#define SYSCALL_H

#include "os_constant.h"
#include "os_type.h"
class SystemService
{
public:
    SystemService();
    void initialize();
    // 设置系统调用，index=系统调用号，function=处理第index个系统调用函数的地址
    bool setSystemCall(int index, int function);
};

// 第0个系统调用
int syscall_0(int first=0, int second=0, int third=0, int forth=0, int fifth=0);
// 第1个系统调用
int syscall_1(int a1=0, int a2=0, int a3=0, int a4=0, int a5=0);
// 第2个系统调用
int syscall_2_out_port(int port=0, int value=0, int p1=0, int p2=0, int p3=0);
// 第3个系统调用
int syscall_3_in_port(int port=0, uint8 * value=0, int p1=0, int p2=0, int p3=0);
// 第4个系统调用
int syscall_4_screen_print(char * screen=0, int pos=0, int c=0,int color=0, int p1=0);
// 第5个系统调用
int syscall_5_screen_input(char * screen=0, int pos=0, int * c=0, int * color=0, int p1=0);
// 第6个系统调用, fork
int fork();
int syscall_6_fork();
// 第7个系统调用, exit
void exit(int ret);
void syscall_7_exit(int ret);
// 第8个系统调用, wait
int wait(int *retval);
int syscall_8_wait(int *retval);
// 第9个系统调用, processCollector
int processCollector();
int syscall_9_processCollector();
#endif