#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

Semaphore semaphore;

int cheese_burger;


/* Consumer and Producer's Problem*/
const int bufferSIZE = 5;
int bufferCNT = 0;

void delay(int time)
{
    while (--time)
    ;    
}

void producer(void *arg)
{
    while (1)
    {
        while(bufferCNT >= bufferSIZE);
        printf("[Producer]: Before producing... Current buffer size is %d.\n",bufferCNT);
        int tmp = bufferCNT + 1;
        delay(0x7ffffff);
        bufferCNT = tmp;
        printf("[Producer]: After producing... Current buffer size is %d.\n",bufferCNT);
    }
}

void consumer(void *arg)
{
    while (1)
    {
        while(bufferCNT <= 0);
        printf("[Consumer]: Before consuming... Current buffer size is %d.\n",bufferCNT);
        int tmp = bufferCNT - 1;
        delay(0x7ffffff);
        bufferCNT = tmp;
        printf("[Consumer]: After consuming... Current buffer size is %d.\n",bufferCNT);
    }

}

void first_thread(void *arg)
{
    // 第1个线程不可以返回
    stdio.moveCursor(0);
    for (int i = 0; i < 25 * 80; ++i)
    {
        stdio.print(' ');
    }
    stdio.moveCursor(0);

    cheese_burger = 0;
    semaphore.initialize(1);

    programManager.executeThread(producer, nullptr, "second thread", 1);
    programManager.executeThread(consumer, nullptr, "third thread", 1);

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
