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

Semaphore chopstick[5];

void delay(int time = 0x6ffffff)
{
    while (--time)
    ;    
}
void think(int num)
{
    delay();
    printf("Philosopher %d is thinking...\n", num + 1);
}
void hungry(int num)
{
    printf("Philosopher %d is hungery...\n", num + 1);
}
void eat(int num)
{
    printf("Philosopher %d is eating...\n", num + 1);
}
void philosopher(void *arg)
{
    int num = *((int *)arg);
    int leftChopstick = num;
    int rightChopstick = (num + 1) % 5;
    while (1)
    {
        think(num);
        hungry(num);
        chopstick[leftChopstick].P();
        printf("Philospher %d gets the chopstick %d, still need one\n",num + 1, leftChopstick + 1);
        delay();
        chopstick[rightChopstick].P();
        printf("Philospher %d gets the chopstick %d, can eat\n",num + 1, rightChopstick + 1);
        eat(num);
        chopstick[rightChopstick].V();
        printf("Philospher %d puts the chopstick %d down\n",num + 1, rightChopstick + 1);
        chopstick[leftChopstick].V();
        printf("Philospher %d puts the chopstick %d down\n",num + 1, leftChopstick + 1);
        delay();
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

    int number[5] = {0};
    char* name = "Philosopher 1";
    for (int i = 0; i < 5; i++)
    {
        chopstick[i].initialize(1);
        number[i] = i;
        name[12] = i + '1';
        programManager.executeThread(philosopher, &number[i], name, 1);
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
