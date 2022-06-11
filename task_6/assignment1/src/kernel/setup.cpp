#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;


extern "C" void setup_kernel()
{
    // 中断处理部件
    interruptManager.initialize();
    // 屏幕IO处理部件
    stdio.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);
    int x = 10086;
    double f = 47.25;
    printf("0. %d\n", (char)f);
    //asm_enable_interrupt();
    printf("1. print percentage: %%\n"
           "2.print char \"N\": %c\n"
           "3.print string \"Hello World!\": %s\n"
           "4.print decimal: \"-1234\": %d\n"
           "5.print hexadecimal \"10086(D)\": %x\n"
           "6.print binary: \"10086(D)\": %b\n"
           "7.print octal: \"10086(D)\": %o\n"
           "8.print float_point_num: \"47.25\": %f\n",
           'N', "Hello World!", -1234, x,x,x,f);

    printf_log("\n[Log]Testing log info...\n");
    printf_warning("\n[Warning]Testing warning info...\n");
    printf_error("\n[Error]Testing error info...\n");
    //uint a = 1 / 0;
    asm_halt();
}
