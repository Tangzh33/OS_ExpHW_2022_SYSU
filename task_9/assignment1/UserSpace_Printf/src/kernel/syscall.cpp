#include "syscall.h"
#include "interrupt.h"
#include "stdlib.h"
#include "asm_utils.h"
#include "os_modules.h"
#include "os_type.h"

int system_call_table[MAX_SYSTEM_CALL];

SystemService::SystemService() {
    initialize();
}

void SystemService::initialize()
{
    memset((char *)system_call_table, 0, sizeof(int) * MAX_SYSTEM_CALL);
    // 代码段选择子默认是DPL=0的平坦模式代码段选择子，DPL=3，否则用户态程序无法使用该中断描述符
    interruptManager.setInterruptDescriptor(0x80, (uint32)asm_system_call_handler, 3);
}

bool SystemService::setSystemCall(int index, int function)
{
    system_call_table[index] = function;
    return true;
}

int syscall_0(int first, int second, int third, int forth, int fifth)
{
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

void graphicMemoryOP(uint pos, uint8 content, uint8 color, int mod)
{
    asm_system_call(47, (int)pos, (int)content,(int)color,mod);
}
void syscall_graphicMemoryOP(uint pos, uint8 content, uint8 color, int mod)
{
    if(mod == 0)
    {
        stdio.screen[2 * pos] = content;
        stdio.screen[2 * pos + 1] = color;
    }
    else if (mod == 1)
    {
        stdio.screen[2 * (pos - 80)] = stdio.screen[2 * pos];
        stdio.screen[2 * (pos - 80) + 1] = stdio.screen[2 * pos + 1];
    }
}   
// 第48个系统调用：端口操作
int portOP(int mod, uint16 port, uint8 content, uint8* outAddr)
{
    int retVal = asm_system_call(48,mod,port,content);
    if(mod == 1)
        *(outAddr) = retVal;        
    return 0;
}
int syscall_portOP(int mod, uint16 port, uint8 content)
{
    uint8 temp = 0;
    if(mod == 0)
    {
        asm_out_port(port, content);
    }
    else if(mod == 1)
    {
        asm_in_port(port, &temp);
    }
    return temp;
}
