#include "interrupt.h"
#include "os_type.h"
#include "os_constant.h"
#include "asm_utils.h"
#include "stdio.h"
#include "os_modules.h"
#include "program.h"

int times = 0;

InterruptManager::InterruptManager()
{
    initialize();
}

void InterruptManager::initialize()
{
    // 初始化中断计数变量
    times = 0;

    // 初始化IDT
    IDT = (uint32 *)IDT_START_ADDRESS;
    asm_lidt(IDT_START_ADDRESS, 256 * 8 - 1);

    for (uint i = 0; i < 256; ++i)
    {
        setInterruptDescriptor(i, (uint32)asm_unhandled_interrupt, 0);
    }

    // 初始化8259A芯片
    initialize8259A();
}

void InterruptManager::setInterruptDescriptor(uint32 index, uint32 address, byte DPL)
{
    IDT[index * 2] = (CODE_SELECTOR << 16) | (address & 0xffff);
    IDT[index * 2 + 1] = (address & 0xffff0000) | (0x1 << 15) | (DPL << 13) | (0xe << 8);
}

void InterruptManager::initialize8259A()
{
    // ICW 1
    asm_out_port(0x20, 0x11);
    asm_out_port(0xa0, 0x11);
    // ICW 2
    IRQ0_8259A_MASTER = 0x20;
    IRQ0_8259A_SLAVE = 0x28;
    asm_out_port(0x21, IRQ0_8259A_MASTER);
    asm_out_port(0xa1, IRQ0_8259A_SLAVE);
    // ICW 3
    asm_out_port(0x21, 4);
    asm_out_port(0xa1, 2);
    // ICW 4
    asm_out_port(0x21, 1);
    asm_out_port(0xa1, 1);

    // OCW 1 屏蔽主片所有中断，但主片的IRQ2需要开启
    asm_out_port(0x21, 0xfb);
    // OCW 1 屏蔽从片所有中断
    asm_out_port(0xa1, 0xff);
}

void InterruptManager::enableTimeInterrupt()
{
    uint8 value;
    // 读入主片OCW
    asm_in_port(0x21, &value);
    // 开启主片时钟中断，置0开启
    value = value & 0xfe;
    asm_out_port(0x21, value);
}

void InterruptManager::disableTimeInterrupt()
{
    uint8 value;
    asm_in_port(0x21, &value);
    // 关闭时钟中断，置1关闭
    value = value | 0x01;
    asm_out_port(0x21, value);
}

void InterruptManager::setTimeInterrupt(void *handler)
{
    setInterruptDescriptor(IRQ0_8259A_MASTER, (uint32)handler, 0);
}

// 中断处理函数
extern "C" void c_time_interrupt_handler()
{
    PCB *cur = programManager.running;
    if(cur->pageDirectoryAddress == 0)
        memoryManager.kernelVirtual.updateLRU();
    else
        cur->userVirtual.updateLRU();
    if (cur->ticks)
    {
        --cur->ticks;
        ++cur->ticksPassedBy;
    }
    else
    {
        programManager.schedule();
    }
}

// 中断处理函数
extern "C" void c_pageFault_handler(uint32 pageFault_Code, uint32 pageFault_Addr, uint32 OSMod)
{
    // 已经在汇编中关中断了，所以此时不必重复设置
    pageFault_Addr = pageFault_Addr & 0xfffff000;
    printf_error("[Page Fault] is happening... Catch the fault page 0x%x\n", pageFault_Addr);
    printf_error("[Page Fault] Error code is 0x%x PTE is %x OS Mod is %d\n", pageFault_Code, *(int*)memoryManager.toPTE(pageFault_Addr), OSMod & 3);
    // printf_error("[Page Fault] OS Mod is %x\n", (int*)memoryManager.toPTE(pageFault_Addr));
    bool accPer_Flag = (pageFault_Code & 4) >> 2;
    bool wriPer_Flag = (pageFault_Code & 2) >> 1;
    bool noPhy_Flag = !(pageFault_Code & 1);
    bool inKer_Flag = ((OSMod & 3) == 0);
    bool inDis_Flag = ((*(int*)memoryManager.toPTE(pageFault_Addr)) & 2 ) == 2;
    enum AddressPoolType type = OSMod == 1 ? AddressPoolType::KERNEL : AddressPoolType:: USER;
    printf_warning("[Page Fault] Report:");
    if(inDis_Flag)
        printf_warning("  Page in disk");
    if(noPhy_Flag)
        printf_warning("  No Phy-Page connected");
    // 对于前两种，是违法操作，不必操作系统进行响应
    if(wriPer_Flag)
    {
        printf_warning("  Write on Read-Only pages.");
        // asm_halt();
    }
    if(accPer_Flag)
    {
        printf_warning("  Operate on Illeagal pages.");
        // asm_halt();
    }
    printf("\n");
    // asm_halt();
    if(inDis_Flag)
    {
        memoryManager.swapIn(pageFault_Addr, inKer_Flag);
        return;
    }

    int physicalPageAddress = memoryManager.allocatePhysicalPages(type, 1);
    // int physicalPageAddress = allocatePhysicalPages(AddressPoolType::USER, 1);
    if (physicalPageAddress == 0)
    {
        /*Find One Page and swapout*/
        // printf_error("Unhandled!!!\n");
        // asm_halt();
        int swapOutPage = 0;
        if(type == 1)
        {
            // in kernel
            swapOutPage = memoryManager.kernelVirtual.findSwapOut();
        }
        else
        {
            // in user space
            swapOutPage = programManager.running->userVirtual.findSwapOut();
        }
        
        memoryManager.swapOut(swapOutPage, type);
        physicalPageAddress = memoryManager.allocatePhysicalPages(type, 1);
        // physicalPageAddress = allocatePhysicalPages(AddressPoolType::USER, 1);
    }
    memoryManager.connectPhysicalVirtualPage((int)pageFault_Addr, physicalPageAddress);
    asm_update_tlb();
    // asm_halt();
    // return;
}

void InterruptManager::enableInterrupt()
{
    asm_enable_interrupt();
}

void InterruptManager::disableInterrupt()
{
    asm_disable_interrupt();
}

bool InterruptManager::getInterruptStatus()
{
    return asm_interrupt_status() ? true : false;
}

// 设置中断状态
void InterruptManager::setInterruptStatus(bool status)
{
    if (status)
    {
        enableInterrupt();
    }
    else
    {
        disableInterrupt();
    }
}
