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
int syscall_0(int first, int second, int third, int forth, int fifth);
// 第47个系统调用：写显存
void graphicMemoryOP(uint pos, uint8 content, uint8 color, int mod);
void syscall_graphicMemoryOP(uint pos, uint8 content, uint8 color, int mod);
// 第48个系统调用：端口操作
int portOP(int mod, uint16 port, uint8 content, uint8* outAddr);
int syscall_portOP(int mod, uint16 port, uint8 content);

#endif