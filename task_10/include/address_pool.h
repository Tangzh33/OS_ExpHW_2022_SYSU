#ifndef ADDRESS_POOL_H
#define ADDRESS_POOL_H

#include "bitmap.h"
#include "os_type.h"
const int MAX_LENGTH = 247;

class AddressPool
{
public:
    BitMap resources;
    int startAddress;
    // 每个地址池维护一个计数器，用于计数LRU
    int lruCNT[MAX_LENGTH];
    // 用于记录对应页的索引
    int lruINDEX[MAX_LENGTH];
    // 局部时钟，用于全局计数维护时间戳
    int localClock;
public:
    AddressPool();
    // 初始化地址池
    void initialize(char *bitmap, const int length, const int startAddress);
    // 从地址池中分配count个连续页，成功则返回第一个页的地址，失败则返回-1
    int allocate(const int count);
    // 释放若干页的空间
    void release(const int address, const int amount);
    // 更新LRU数组
    void updateLRU();
    int findSwapOut();
};

#endif