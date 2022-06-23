#ifndef BYTEMEMORY_H
#define BYTEMEMORY_H

#include "memory.h"
#include "sync.h"

enum ArenaType
{
    ARENA_16,
    ARENA_32,
    ARENA_64,
    ARENA_128,
    ARENA_256,
    ARENA_512,
    ARENA_1024,
    ARENA_MORE
};
struct Arena
{
    ArenaType type; // Arena的类型
    int counter;  // 如果是ARENA_MORE，则counter表明页的数量，
                    // 否则counter表明该页中的可分配arena的数量
};
struct MemoryBlockListItem
{
    MemoryBlockListItem *previous, *next;
};
// MemoryManager是在内核态调用的内存管理对象
class ByteMemoryManager
{

private:
    // 16, 32, 64, 128, 256, 512, 1024
    static const int MEM_BLOCK_TYPES = 7;       // 内存块的类型数目
    static const int minSize = 16;              // 内存块的最小大小
    int arenaSize[MEM_BLOCK_TYPES];             // 每种类型对应的内存块大小
    MemoryBlockListItem *arenas[MEM_BLOCK_TYPES]; // 每种类型的arena内存块的指针
    SpinLock locks[MEM_BLOCK_TYPES+1];        //给每种类型的arena内存块设置对应的锁

public:
    ByteMemoryManager();
    void initialize();
    void *allocate(int size);  // 分配一块地址
    void release(enum AddressPoolType type,void *address); // 释放一块地址

private:
    bool getNewArena(AddressPoolType type, int index);
};


#endif