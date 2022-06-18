#include "address_pool.h"
#include "os_constant.h"
#include "stdio.h"

AddressPool::AddressPool()
{
}

// 设置地址池BitMap
void AddressPool::initialize(char *bitmap, const int length, const int startAddress)
{
    resources.initialize(bitmap, length);
    this->startAddress = startAddress;
    // 清空时间戳
    for(int i = 0; i < length; ++i)
        lruCNT[i] = 0;
    // 初始化时钟
}

// 从地址池中分配count个连续页
int AddressPool::allocate(const int count)
{
    int start = resources.allocate(count);
    if(start == -1)
        return -1;
    // 更新时钟
    for(int i = 0; i < count; ++i)
    {
        lruCNT[start + i] = localClock;
    }
    return start * PAGE_SIZE + startAddress;
}

// 释放若干页的空间
void AddressPool::release(const int address, const int amount)
{
    resources.release((address - startAddress) / PAGE_SIZE, amount);
}

void AddressPool::updateLRU()
{
    ++localClock;
    for (int i = 0; i < resources.length; ++i)
    {
        if(!resources.get(i))
        // 如果没有被访问，则跳过
            continue;
        int vaddr = startAddress + i * PAGE_SIZE;
        unsigned int* pte = (unsigned int*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (((vaddr & 0x003ff000) >> 12) * 4));
        if((*pte) & (1<<5))
        {
            printf_error("Updating index %d\n", i);
            lruCNT[i] = localClock;
            // 写入位和脏位置零
            (*pte) = (*pte) & (~ 3 << 5);
        }
    }
}

int AddressPool::swapOut()
{
    int currentMin = localClock, index = 0;
    printf_error("lruCNT Array: ");
    for (int i = 0; i < resources.length; ++i)
    {
        printf_warning("%d ",lruCNT[i]);
        if(!resources.get(i))
        // 如果没有被访问，则跳过
            continue;
        if(lruCNT[i] < currentMin)
        {
            currentMin = lruCNT[i];
            index = i;
        }
    }
    printf_error("\n[Swap out] index  = %d, vaddr = 0x%x\n",index, startAddress + index * PAGE_SIZE);
    return startAddress + index * PAGE_SIZE;

}