#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;
// 内存管理器
MemoryManager memoryManager;

void test_thread(void *arg)
{
    // 第1个线程不可以返回
    // stdio.moveCursor(0);
    // for (int i = 0; i < 25 * 80; ++i)
    // {
    //     stdio.print(' ');
    // }
    // stdio.moveCursor(0);
    char *pArray[4];
    for (int i = 0; i < 4; i++)
    {
        pArray[i] = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
        printf_error("[Allocate]Test Thread: 0x%x \n", pArray[i]);
    }

    for (int i = 0; i < 4; i++)
    {
        memoryManager.releasePages(AddressPoolType::KERNEL, (int)pArray[i], 2);
        printf_error("Test Thread: 0x%x \n", pArray[i]);
    }
}

void multi_first_thread(void *arg)
{
    char *pArray[4];
    for (int i = 0; i < 2; i++)
    {
        pArray[i] = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
        printf_error("[Allocate]First Thread: 0x%x \n", pArray[i]);
    }
    printf_warning("First Thread has finished allocated\n");
}

void multi_second_thread(void *arg)
{
    char *pArray[4];
    for (int i = 0; i < 2; i++)
    {
        pArray[i] = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 2);
        printf_error("[Allocate]Second Thread: 0x%x \n", pArray[i]);
    }
    printf_warning("Second Thread has finished allocated\n");
}

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    // stdio.moveCursor(0);
    // for (int i = 0; i < 25 * 80; ++i)
    // {
    //     stdio.print(' ');
    // }
    // stdio.moveCursor(0);
    int pid[3];
    // pid[0] = programManager.executeThread(test_thread, nullptr, "test thread", 1);
    pid[1] = programManager.executeThread(multi_first_thread, nullptr, "multi_first thread", 1);
    pid[2] = programManager.executeThread(multi_second_thread, nullptr, "multi_second thread", 1);

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

    // 进程/线程管理器
    programManager.initialize();

    // 内存管理器
    memoryManager.openPageMechanism();
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
