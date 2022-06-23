#include "memory.h"
#include "bytememory.h"
#include"stdio.h"
#include "os_modules.h"
#include "os_constant.h"
#include "os_type.h"

ByteMemoryManager::ByteMemoryManager()
{
    initialize();
}

void ByteMemoryManager::initialize()
{
    int size = minSize;
    for (int i = 0; i < MEM_BLOCK_TYPES; ++i)
    {
        arenas[i] = nullptr;
        arenaSize[i] = size;
        size = size << 1;
        locks[i].initialize();
    }
     locks[MEM_BLOCK_TYPES].initialize();
}

void *ByteMemoryManager::allocate(int size)
{
    int index = 0;
    while (index < MEM_BLOCK_TYPES && arenaSize[index] < size)
        ++index;
    locks[index].lock();
    PCB *pcb = programManager.running;
    AddressPoolType poolType = (pcb->pageDirectoryAddress) ? AddressPoolType::USER : AddressPoolType::KERNEL;
    void *ans = nullptr;

    if (index == MEM_BLOCK_TYPES)
    {
        // 上取整
        int pageAmount = (size + sizeof(Arena) + PAGE_SIZE - 1) / PAGE_SIZE;

        ans = (void*)memoryManager.allocatePages(poolType, pageAmount);

        if (ans)
        {
            Arena *arena = (Arena *)ans;
            arena->type = ArenaType::ARENA_MORE;
            arena->counter = pageAmount;
        }
    }
    else
    {
            printf_warning("[ByteMemoryManager]allocating...\n");
        if (arenas[index] == nullptr)
        {
            if (!getNewArena(poolType, index))
            {
                locks[index].unlock();
                return nullptr;
            }
        }
        
        // 每次取出内存块链表中的第一个内存块
        ans = arenas[index];
        arenas[index] = ((MemoryBlockListItem *)ans)->next;

        if (arenas[index])
        {
            (arenas[index])->previous = nullptr;
        }

        Arena *arena = (Arena *)((int)ans & 0xfffff000);
        --(arena->counter);
        printf_warning("[ByteMemoryManager]Pages vaddress %x, Arena balance: %d\n",arena,arena->counter);
    }
    locks[index].unlock();
    return ans;
}
bool ByteMemoryManager::getNewArena(AddressPoolType type, int index)
{
    void *ptr = (void*)memoryManager.allocatePages(type, 1);

    if (ptr == nullptr)
        return false;
    printf_warning("[ByteMemoryManager]Pages vaddress %x\n",(ptr+sizeof(Arena)));
    // 内存块的数量
    int times = (PAGE_SIZE - sizeof(Arena)) / arenaSize[index];
    // 内存块的起始地址
    int address = (int)ptr + sizeof(Arena);

    // 记录下内存块的数据
    Arena *arena = (Arena *)ptr;
    arena->type = (ArenaType)index;
    arena->counter = times;
    printf_warning("[ByteMemoryManager]NewArena: type: %d, Arena balance:  %d\n", index, arena->counter);

    MemoryBlockListItem *prevPtr = (MemoryBlockListItem *)address;
    MemoryBlockListItem *curPtr = nullptr;
    arenas[index] = prevPtr;
    prevPtr->previous = nullptr;
    prevPtr->next = nullptr;
    --times;

    while (times)
    {
        address += arenaSize[index];
        curPtr = (MemoryBlockListItem *)address;
        prevPtr->next = curPtr;
        curPtr->previous = prevPtr;
        curPtr->next = nullptr;
        prevPtr = curPtr;
        --times;
    }
    return true;
}

void ByteMemoryManager::release(enum AddressPoolType type,void *address)
{
    // 由于Arena是按页分配的，所以其首地址的低12位必定0，
    // 其中划分的内存块的高20位也必定与其所在的Arena首地址相同
    Arena *arena = (Arena *)((int)address & 0xfffff000);
    locks[arena->type].lock();
    if (arena->type == ARENA_MORE)
    {
        int address = (int)arena;

        memoryManager.releasePages(type,address, arena->counter);
    }
    else
    {
        MemoryBlockListItem *itemPtr = (MemoryBlockListItem *)address;
        itemPtr->next = arenas[arena->type];
        itemPtr->previous = nullptr;

        if (itemPtr->next)
        {
            itemPtr->next->previous = itemPtr;
        }

        arenas[arena->type] = itemPtr;
        ++(arena->counter);

        // 若整个Arena被归还，则清空分配给Arena的页
        int amount = (PAGE_SIZE - sizeof(Arena)) / arenaSize[arena->type];
        printf_warning("[ByteMemoryManager]Releasing arena Arena balance: %d, amount: %d\n", arena->counter, amount);

        if (arena->counter == amount)
        {
            // 将属于Arena的内存块从链上删除
            while (itemPtr)
            {
                if ((int)arena != ((int)itemPtr & 0xfffff000))
                {
                    itemPtr = itemPtr->next;
                    continue;
                }

                if (itemPtr->previous == nullptr) // 链表中的第一个节点
                {
                    arenas[arena->type] = itemPtr->next;
                    if (itemPtr->next)
                    {
                        itemPtr->next->previous = nullptr;
                    }
                }
                else
                {
                    itemPtr->previous->next = itemPtr->next;
                }

                if (itemPtr->next)
                {
                    itemPtr->next->previous = itemPtr->previous;
                }

                itemPtr = itemPtr->next;
            }

            memoryManager.releasePages(type,(int)address, 1);
            printf_warning("[ByteMemoryManager]Released page vaddress:%x\n",address);
            locks[arena->type].unlock();
        }
    }
}