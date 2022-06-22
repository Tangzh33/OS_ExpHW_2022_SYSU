#include "asm_utils.h"
#include "interrupt.h"
#include "stdio.h"
#include "program.h"
#include "thread.h"
#include "sync.h"
#include "memory.h"
#include "syscall.h"
#include "tss.h"
#include "disk.h"
#include "swap.h"
#include "alloc.h"

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
// Alloc 分配启动器
ByteMemoryManager byteMemoryManager;

int syscall_0(int first, int second, int third, int forth, int fifth)
{
    printf("systerm call 0: %d, %d, %d, %d, %d\n",
           first, second, third, forth, fifth);
    return first + second + third + forth + fifth;
}

void first_process()
{
    printf_warning("Testing my process\n");
    enum AddressPoolType type = AddressPoolType::USER;
    char* buffer[4];
    for (int i = 0; i < 4; i++)
    {
        buffer[i] = (char*)memoryManager.allocatePages(USER, 1);
    }
    // buffer[1][1] = 1;
    for (int i = 0; i < 47; i++)
    {
        buffer[0][i] = i;
        buffer[1][i] = i;
        buffer[2][i] = i;
    }
    
    printf("%x\n",*(int*)memoryManager.toPTE((int)&buffer[3][0]));
    printf("Reading the unallocated page: %d\n",buffer[3][0]);
    printf_error("Begin Copying Testing\n");
    int flag = 1;
    for (int i = 0; i < 47; i++)
    {
        if(buffer[0][i] != i)
        {
            flag = 0;
            printf_error("ERROR! Break...\n");
        }
    }
    if(flag)
        printf_warning("Congratulations! No Fault!\n");
    printf_warning("Finish testing...\n");

    // asm_halt();

    // exit(0);
    // int pid = fork();
    // int retval;

    // if (pid)
    // {
    //     pid = fork();
    //     if (pid)
    //     {
    //         while ((pid = wait(&retval)) != -1)
    //         {
    //             printf("wait for a child process, pid: %d, return value: %d\n", pid, retval);
    //         }

    //         printf("all child process exit, programs: %d\n", programManager.allPrograms.size());

    //         asm_halt();
    //     }
    //     else
    //     {
    //         uint32 tmp = 0xffffff;
    //         while (tmp)
    //             --tmp;
    //         printf("exit, pid: %d\n", programManager.running->pid);
    //         exit(123934);
    //     }
    // }
    // else
    // {
    //     uint32 tmp = 0xffffff;
    //     while (tmp)
    //         --tmp;
    //     printf("exit, pid: %d\n", programManager.running->pid);
    //     exit(-123);
    // }
}

void second_thread(void *arg)
{
    printf("thread exit\n");
    // exit(0);
}

void first_thread(void *arg)
{
    /*
    // Swap in testing
    // char buffer[SECTOR_SIZE] = "Hello Word TangZh";
    char *buffer = (char*)memoryManager.allocatePages(AddressPoolType::KERNEL,1);
    char *buffer_1 = (char*)memoryManager.allocatePages(AddressPoolType::KERNEL,1);
    int *pte = (int*)memoryManager.toPTE((int)buffer);
    int *pde = (int*)memoryManager.toPDE((int)buffer);
    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
        buffer[i] = i;
        buffer_1[i] = i;
    }
    // *(int*)0xc0100300 = 1;

    printf("Before Swapout: %d, PTE %x; PDE is %x\n", buffer[2],*pte,*pde);
    memoryManager.swapOut((uint32)buffer, 0);
    // buffer[0] = 'a';
    // buffer[1] = 'a';
    // memoryManager.releasePages(AddressPoolType::KERNEL,(int)buffer,1);
    printf("After Swapout before swapin: PTE %x; PDE is %x\n", *pte,*pde);
    printf("Trying to read: Addr 0x%x PTE 0x%x %d\n", buffer,*pte,buffer[10]);
    // printf("Trying to read: Addr 0x%x PTE 0x%x %d\n", *(int*)0xc0101000,*pte,buffer[0]);
    // memoryManager.swapIn((uint32)buffer, 0);
    printf("After Swapin: %d, PTE %x; PDE is %x\n", buffer[2],*pte,*pde);
    // asm_halt();
    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
        // printf("%d ",buffer[i]);
        if (buffer[i] != buffer_1[i])
        {
            printf_error("error!\n");
        }
        // printf("\n");
    }
    printf("Finish Testing\n");
    */
    /*
    // PageFault Testing
    printf_warning("Begin Page Fault Testing\n");
    char *memory_test = (char *)memoryManager.allocateVirtualPages(AddressPoolType::KERNEL, 1);
    printf("allocate vaddr is : 0x%x.\n", (uint32)memory_test);
    int* pte = (int *)memoryManager.toPTE((int)memory_test);
    printf("PTE Before %x\n", *pte); 
    int physicalPageAddress = memoryManager.allocatePhysicalPages(AddressPoolType::KERNEL, 1);
    memoryManager.connectPhysicalVirtualPage((int)memory_test, physicalPageAddress);
    printf("PTE After %x\n", *pte); 
    *pte = *pte & ~(1);
    printf("PTE After %x\n", *pte); 
    printf("Trying to read the memory: %c\n", memory_test[0]);
    // int a = memory_test[0];
    // memory_test[0] = 'a';
    */
    // /*
    // Start Userspace testing
    // printf("start process\n");
    // int * pte= (int *)memoryManager.toPTE(0xc0101000);
    // printf_warning("%x %x\n",pte,*pte);
    // programManager.executeProcess((const char *)first_process, 1);
    // printf_error("Kernel Process is halting...\n");

    
    // programManager.executeThread(second_thread, nullptr, "second", 1);
    // */

    /* disk测试代码
    char buffer[SECTOR_SIZE] = "Hello Word TangZh";
    Disk::write(1, buffer);
    Disk::read(1, buffer);

    for (int i = 0; i < 17; ++i)
    {
        printf("%c", buffer[i]);
        if (i % 16 == 15)
        {
            printf("\n");
        }
    }

    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
        buffer[i] = i;
    }

    Disk::write(2, buffer);

    char test_buffer[SECTOR_SIZE];
    Disk::read(2, test_buffer);
    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
        if (buffer[i] != test_buffer[i])
        {
            printf_error("error!\n");
        }
    }
    */
    //  Alloc Manager
    int* test = (int*)byteMemoryManager.allocate(1);
    test[0] = 1;
    byteMemoryManager.release((void*)test);
    test[0] = 1;
    asm_halt();
}

extern "C" void setup_kernel()
{

    // 中断管理器
    interruptManager.initialize();
    interruptManager.enableTimeInterrupt();
    interruptManager.setTimeInterrupt((void *)asm_time_interrupt_handler);
    // 设置页错误中断的中断描述符
    interruptManager.setInterruptDescriptor(14, (uint)asm_pageFault_interrupt_handler, 0);
    // 输出管理器
    stdio.initialize();

    // 进程/线程管理器
    programManager.initialize();

    // 初始化系统调用
    systemService.initialize();
    // 设置0号系统调用
    systemService.setSystemCall(0, (int)syscall_0);
    // 设置1号系统调用
    systemService.setSystemCall(1, (int)syscall_write);
    // 设置2号系统调用
    systemService.setSystemCall(2, (int)syscall_fork);
    // 设置3号系统调用
    systemService.setSystemCall(3, (int)syscall_exit);
    // 设置4号系统调用
    systemService.setSystemCall(4, (int)syscall_wait);

    // 内存管理器
    memoryManager.initialize();

    // alloc管理器初始化
    byteMemoryManager.initialize();

    // 创建第一个线程
    int pid = programManager.executeThread(first_thread, nullptr, "first thread", 1);
    if (pid == -1)
    {
        printf("can not execute thread\n");
        asm_halt();
    }

    ListItem *item = programManager.readyPrograms.front();
    PCB *firstThread = ListItem2PCB(item, tagInGeneralList);
    firstThread->status = ProgramStatus::RUNNING;
    programManager.readyPrograms.pop_front();
    programManager.running = firstThread;
    asm_switch_thread(0, firstThread);

    asm_halt();
}
