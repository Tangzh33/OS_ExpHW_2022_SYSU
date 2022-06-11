#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"

const int DELAY_TOTAL = 1000000000;
const int DELAY_CTRL = 10000000;

// 屏幕IO处理器
STDIO stdio;
// 中断管理器
InterruptManager interruptManager;
// 程序管理器
ProgramManager programManager;

void high_priority_thread(void *arg) {
    // printf("pid %d ppid %d name \"%s\": Hello World!\n", programManager.running->pid,programManager.running->ppid, programManager.running->name);
    for (int i = 0; i < DELAY_TOTAL; i++)
        if(i % DELAY_CTRL == 0)
            programManager.printInfo();
            // printf("[HIGHEST PRIORITY THREAD]time: %d piority: %d\n",programManager.running -> ticksPassedBy,programManager.running -> priority);
    program_exit();
}


void third_thread(void *arg) {
    // printf("pid %d ppid %d name \"%s\": Hello World!\n", programManager.running->pid,programManager.running->ppid, programManager.running->name);
    for (int i = 0; i < DELAY_TOTAL; i++)
    {
        if(i % DELAY_CTRL == 0)
            programManager.printInfo();
            // printf("Thread3 time: %d piority: %d\n",programManager.running -> ticksPassedBy,programManager.running -> priority);
        if (programManager.running->pid == 2 && i == 3 * 25000000)
        {
            programManager.executeThread(high_priority_thread, nullptr, "high priority thread");
        }
    }
    while(1) {

    }
}
void second_thread(void *arg) {
    // printf("pid %d ppid %d name \"%s\": Hello World!\n", programManager.running->pid,programManager.running->ppid, programManager.running->name);
    if (programManager.running->pid == 1)
    {
        programManager.executeThread(third_thread, nullptr, "third thread");
    }
    for (int i = 0; i < DELAY_TOTAL; i++)
        if(i % DELAY_CTRL == 0)
            programManager.printInfo();
            // printf("Thread2 time: %d piority: %d\n",programManager.running -> ticksPassedBy,programManager.running -> priority);
    program_exit();

}
void first_thread(void *arg)
{
    // 第1个线程不可以返回
    // printf("pid %d ppid %d name \"%s\": Hello World!\n", programManager.running->pid,programManager.running->ppid, programManager.running->name);
    programManager.printInfo("HelloWorld");
    if (!programManager.running->pid)
    {
        programManager.executeThread(second_thread, nullptr, "second thread");
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
    int pid = programManager.executeThread(first_thread, nullptr, "first thread");
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms[1].front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = RUNNING;
    --programManager.readyProgramsNum;
    programManager.readyPrograms[1].pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
