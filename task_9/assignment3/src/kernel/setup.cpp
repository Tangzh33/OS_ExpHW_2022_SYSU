#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"
#include "syscall.h"
#include "tss.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;
// 系统调用
SystemService systemService;
// Task State Segment
TSS tss;

void first_process()
{
    int pid = fork();
    int retval;

    if (pid)
    {
        printf("this is first process, son is %d\n", pid);
        asm_halt();
    }
    else
    {
        pid = fork();
        if (pid)
        {
            printf("this is second process, son is %d\n", pid);
            uint32 tmp = 0xffffff;
            uint32 otmp = 0xf;
            while (otmp)
            {
                while (tmp)
                {
                    tmp--;
                }
                tmp = 0xffffff;
                otmp--;
            }
            tmp = pid;
            if ((pid = wait(&retval)) != -1)
            {
                printf("wait for a child process, pid: %d, return value: %d\n",
                       pid, retval);
            }
            else
            {
                printf("[User]OS has collected zoombie process[Pid: %d]\n", tmp);
            }
        }
        else
        {
            printf("this is third process\n");
            exit(123);
        }
    }
}

void second_thread(void *arg)
{
    printf("thread exit\n");
    // exit(0);
}

void first_thread(void *arg)
{

    printf("start process\n");
    programManager.executeProcess((const char *)first_process, 1);
    // programManager.executeThread(second_thread, nullptr, "second", 1);
    while (true)
    {
        uint32 tmp = 0xffffff;
        while (tmp)
            tmp--;
        processCollector();
    }
    asm_halt();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);

    // 输出管理器
    stdio.initialize();

    // 初始化系统调用
    systemService.initialize();
    // 设置0号系统调用
    systemService.setSystemCall(0, (int)syscall_0);
    systemService.setSystemCall(1, (int)syscall_1);
    systemService.setSystemCall(2, (int)syscall_2_out_port);
    systemService.setSystemCall(3, (int)syscall_3_in_port);
    systemService.setSystemCall(4, (int)syscall_4_screen_print);
    systemService.setSystemCall(5, (int)syscall_5_screen_input);
    systemService.setSystemCall(6, (int)syscall_6_fork);
    systemService.setSystemCall(7, (int)syscall_7_exit);
    systemService.setSystemCall(8, (int)syscall_8_wait);
    systemService.setSystemCall(9, (int)syscall_9_processCollector);

    // 进程/线程管理器
    programManager.initialize();

    // 内存管理器
    memoryManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
