#include "syscall.h"
#include "interrupt.h"
#include "stdlib.h"
#include "asm_utils.h"
#include "os_modules.h"
#include "stdio.h"

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

int write(const char *str, int color) {
    return asm_system_call(1, (int)str, color);
}

int syscall_write(const char *str, int color) {
    return stdio.print(str, color);
}

int fork() {
    return asm_system_call(2);
}

int syscall_fork() {
    return programManager.fork();
}

void exit(int ret) {
    asm_system_call(3, ret);
}

void syscall_exit(int ret) {
    programManager.exit(ret);
}

int wait(int *retval) {
    return asm_system_call(4, (int)retval);
}

int syscall_wait(int *retval) {
    return programManager.wait(retval);
}

void *malloc(int size)
{
    asm_system_call(5, size);
}

void *syscall_malloc(int size)
{
    PCB *pcb = programManager.running;
    if (pcb->pageDirectoryAddress)
    {
        // 每一个进程有自己的ByteMemoryManager
        return pcb->byteMemoryManager.allocate(size);
    }
    else
    {
        // 所有内核线程共享一个ByteMemoryManager
        return kernelByteMemoryManager.allocate(size);
    }
}

void free(void *address)
{
    asm_system_call(6, (int)address);
}

void syscall_free(void *address)
{
    PCB *pcb = programManager.running;
    enum AddressPoolType type = pcb->pageDirectoryAddress == 0 ? AddressPoolType::KERNEL : AddressPoolType::USER;
    if (pcb->pageDirectoryAddress)
    {
        pcb->byteMemoryManager.release(type,address);
    }
    else
    {
        kernelByteMemoryManager.release(type,address);
    }
}