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

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    // stdio.moveCursor(0);
    // for (int i = 0; i < 25 * 80; ++i)
    // {
    //     stdio.print(' ');
    // }
    // stdio.moveCursor(0);
    // char* pArray[10] = {0};
    printf_log("Begin Testing\n");
    int cnt = 0;
    char* pArray[10];
    for (int i = 0; i < 12; ++i)
    {
        if(i < 10)
            pArray[i] = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 1);
        else if(i == 10)
        {
        // 休眠一段时间，拉开时间差
            for (int i = 0; i < 0x7f7f7f7; i++);
            printf_error("Before writing: %x\n",*(int*)memoryManager.toPTE((int)pArray[0]));
            *(int *)pArray[0] = 0x7c00;
            *(int *)pArray[2] = 0x7c00;
            // pArray[2][1] = 1;
            printf_error("After writing: %x\n",*(int*)memoryManager.toPTE((int)pArray[0]));
            for (int i = 0; i < 0x7f7f7f7; i++);
        }
        else
            char *p1 = (char *)memoryManager.allocatePages(AddressPoolType::KERNEL, 3);
        // cnt += 4;
        // printf_warning("[0x%x]We have allocated %d MB, %d MB left\n",p1,cnt, memoryManager.totalMemory / 1024 / 1024 - cnt);
        // ++cnt;
        // if(p1 == 0)
        //     break;
        // printf_warning("[0x%x]We have allocated %d pages\n",p1, cnt * 3);
    }
    printf_log("Testing Finished\n");
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
