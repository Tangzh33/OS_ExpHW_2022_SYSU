#include "program.h"
#include "stdlib.h"
#include "interrupt.h"
#include "asm_utils.h"
#include "stdio.h"
#include "thread.h"
#include "os_modules.h"

const int PCB_SIZE = 4096;                   // PCB的大小，4KB。
char PCB_SET[PCB_SIZE * MAX_PROGRAM_AMOUNT]; // 存放PCB的数组，预留了MAX_PROGRAM_AMOUNT个PCB的大小空间。
bool PCB_SET_STATUS[MAX_PROGRAM_AMOUNT];     // PCB的分配状态，true表示已经分配，false表示未分配。

ProgramManager::ProgramManager()
{
    initialize();
}

void ProgramManager::initialize()
{
    allPrograms.initialize();
    for (int i = 0; i < 6; i++)
    {
        readyPrograms[i].initialize();
    }
    readyProgramsNum = 0;
    running = nullptr;

    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        PCB_SET_STATUS[i] = false;
    }
}

int ProgramManager::executeThread(ThreadFunction function, void *parameter, const char *name, int priority)
// 线程优先级默认为1
{
    // 关中断，防止创建线程的过程被打断
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    // 分配一页作为PCB
    PCB *thread = allocatePCB();

    if (!thread)
        return -1;

    // 初始化分配的页
    memset(thread, 0, PCB_SIZE);

    for (int i = 0; i < MAX_PROGRAM_NAME && name[i]; ++i)
    {
        thread->name[i] = name[i];
    }

    thread->status = ProgramStatus::READY;
    thread->priority = priority;
    thread->ticks = priority * 10;
    thread->ticksPassedBy = 0;
    thread->pid = ((int)thread - (int)PCB_SET) / PCB_SIZE;
    thread->ppid = running ? running->pid : 0;

    // 线程栈
    thread->stack = (int *)((int)thread + PCB_SIZE);
    thread->stack -= 7;
    // 设置四个寄存器的值
    thread->stack[0] = 0;
    thread->stack[1] = 0;
    thread->stack[2] = 0;
    thread->stack[3] = 0;
    thread->stack[4] = (int)function;
    thread->stack[5] = (int)program_exit;
    thread->stack[6] = (int)parameter;

    ++readyProgramsNum;
    allPrograms.push_back(&(thread->tagInAllList));
    readyPrograms[thread->priority].push_back(&(thread->tagInGeneralList));//往对应优先级队列中加入进程

    // 恢复中断
    interruptManager.setInterruptStatus(status);

    return thread->pid;
}

void ProgramManager::schedule()
{
    // 关中断，防止调度时被打断
    bool status = interruptManager.getInterruptStatus();
    interruptManager.disableInterrupt();

    if (readyProgramsNum == 0)
    {
        interruptManager.setInterruptStatus(status);
        return;
    }

    if (running->status == ProgramStatus::RUNNING)
    {
        running->status = ProgramStatus::READY;
        running->priority = running->priority > 4 ? 5 : running->priority + 1; //时间片耗完轮转时优先级下降
        running->ticks = running->priority * 10;
        readyPrograms[running->priority].push_back(&(running->tagInGeneralList));
        readyProgramsNum++;
    }
    else if (running->status == ProgramStatus::DEAD)
    {
        releasePCB(running);
    }


    ListItem *item = nullptr;
    for (int i = 0; i < 6; i++)
        if(readyPrograms[i].size())
        {
            item = readyPrograms[i].front();
            readyPrograms[i].pop_front();
            break;
        }
    PCB *next = ListItem2PCB(item, tagInGeneralList);
    PCB *cur = running;
    next->status = ProgramStatus::RUNNING;
    running = next;

    asm_switch_thread(cur, next);

    interruptManager.setInterruptStatus(status);
}

void program_exit()
{
    PCB *thread = programManager.running;
    programManager.printInfo("Exit");
    thread->status = ProgramStatus::DEAD;

    if (thread->pid)
    {
        programManager.schedule();
    }
    else
    {
        interruptManager.disableInterrupt();
        printf("halt\n");
        asm_halt();
    }
}

PCB *ProgramManager::allocatePCB()
{
    for (int i = 0; i < MAX_PROGRAM_AMOUNT; ++i)
    {
        if (!PCB_SET_STATUS[i])
        {
            PCB_SET_STATUS[i] = true;
            return (PCB *)((int)PCB_SET + PCB_SIZE * i);
        }
    }

    return nullptr;
}

void ProgramManager::releasePCB(PCB *program)
{
    int index = ((int)program - (int)PCB_SET) / PCB_SIZE;
    PCB_SET_STATUS[index] = false;
}

void ProgramManager::printInfo(char * Message)
{
    if(Message)
        printf_warning("[TangZh] Pid: %d PPid: %d Name: \"%s\" Piority: %d time: %dMessage: \"%s\"\n", running->pid, running->ppid, running->name, running->priority,running->ticksPassedBy,Message);
    else
        printf_warning("[TangZh] Pid: %d PPid: %d Name: \"%s\" Piority: %d time: %d\n", running->pid, running->ppid, running->name, running->priority,running->ticksPassedBy);

}