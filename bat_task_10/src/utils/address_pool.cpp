#include "address_pool.h"
#include "os_constant.h"
#include "stdio.h"
#include "asm_utils.h"

AddressPool::AddressPool()
{
}

// 设置地址池BitMap
void AddressPool::initialize(char *bitmap, const int length, const int startAddress)
{
    resources.initialize(bitmap, length);
    this->startAddress = startAddress;
    // 清空时间戳
    for(int i = 0; i < MAX_LENGTH; ++i)
    {
        lruCNT[i] = 0;
        lruINDEX[i] = -1;
    }
    // 初始化时钟
    localClock = 0;
}

// 从地址池中分配count个连续页
int AddressPool::allocate(const int count)
{
    int start = resources.allocate(count);
    if(start == -1)
        return -1;
    if (start == 0)
        return startAddress;
    // 更新时钟
    for(int i = 0; i < count; ++i)
    {
        // if((unsigned int)((i + start) * PAGE_SIZE + startAddress) >= 0xffc00000 || (unsigned int)((i + start) * PAGE_SIZE + startAddress) <= 0x8048000)
        //     break;
        int j = 0;
        while (lruINDEX[j] != -1 && j < MAX_LENGTH)
        {
            ++j;
        }
        if(j == MAX_LENGTH)
        {
            printf_error("Exeed the memory queue. halt ...\n");
            asm_halt();
            return -1;
        }
        lruINDEX[j] = i + start;
        lruCNT[j] = localClock;
    }
    return start * PAGE_SIZE + startAddress;
}

// 释放若干页的空间
void AddressPool::release(const int address, const int amount)
{
    resources.release((address - startAddress) / PAGE_SIZE, amount);
    int start = (address - startAddress) / PAGE_SIZE;
    // TODO
    for(int i = 0; i < amount; ++i)
    {
        int j = 0;
        while (lruINDEX[j] != i + start && j < MAX_LENGTH)
        {
            ++j;
        }
        lruINDEX[j] = -1;
        lruCNT[j] = 0;
    }
}

void AddressPool::updateLRU()
{
    ++localClock;
    for (int i = 0; i < MAX_LENGTH; ++i)
    {
        if(lruINDEX[i] == -1)
        // 如果没有被分配，则跳过
            continue;
        int vaddr = startAddress + lruINDEX[i] * PAGE_SIZE;
        unsigned int* pte = (unsigned int*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (((vaddr & 0x003ff000) >> 12) * 4));
        if((*pte) & (1<<5))
        {
            // printf_error("Updating index %d\n", i);
            lruCNT[i] = localClock;
            // 写入位和脏位置零
            (*pte) = (*pte) & (~( 3 << 5));
        }
    }
}

int AddressPool::findSwapOut()
{
    int currentMin = localClock, index = 0;
    printf_error("lruCNT Array: ");
    for (int i = 0; i < MAX_LENGTH; ++i)
    {
        if(lruINDEX[i] == -1)
        // 如果没有被分配，则跳过
            continue;
        printf_warning("[%d, %d] ",lruINDEX[i], lruCNT[i]);
        if(lruCNT[i] < currentMin)
        {
            currentMin = lruCNT[i];
            index = lruINDEX[i];
        }
    }
    printf_error("\n[Swap out] index  = %d, vaddr = 0x%x\n",index, startAddress + index * PAGE_SIZE);
    return startAddress + index * PAGE_SIZE;

}