#ifndef SWAP_H
#define SWAP_H
#include "bitmap.h"
#include "stdio.h"
#include "memory.h"
#include "os_type.h"
#include "os_constant.h"
#include "asm_utils.h"
#include "stdio.h"
#include "os_modules.h"
#include "program.h"
#include "disk.h"

#define SECTOR_SIZE 512

class SwapManager
{
private:
    int beginSector;
    BitMap swapResources;

public:
    void initialize(char *bitmapStartAddr = nullptr, const int _beginSector = 200, const int length = 400);
    int swapOut(uint32 vaddr);
    int swapIn(uint32 vaddr);
};

void SwapManager::initialize(char *bitmapStartAddr , const int _beginSector , const int length )
{
    if(bitmapStartAddr == nullptr)
    {
        printf_error("SwapManager initialze Failed\n");
    }
    swapResources.initialize(bitmapStartAddr, length);
    beginSector = _beginSector;
}
int SwapManager::swapOut(uint32 vaddr)
{
    int* pte = (int *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (((vaddr & 0x003ff000) >> 12) * 4));
    int index = swapResources.allocate(8);
    if(index == -1)
    {
        printf_error("Swapping Out is Failed due to limited swap-disk\n");
        return -1;
    }
    printf_warning("Swapping out Page: 0x%x to Sector %d\n",vaddr, index + beginSector);
    for (int i = 0; i < 8; i++)
    {
        char* ptr = (char *)vaddr + i * PAGE_SIZE; 
        Disk::write(index + i + beginSector, (void*)ptr);
    }
    memoryManager.releasePages(AddressPoolType::USER, vaddr, 1);
    *pte = 3 + (index << 20);
    return 0;
}
int SwapManager::swapIn(uint32 vaddr)
{
    int* pte = (int *)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + (((vaddr & 0x003ff000) >> 12) * 4));
    int index = (*pte) >> 20;
    printf_warning("Swapping in Page: 0x% Sector %d\n",vaddr, index + beginSector);
    int physicalPageAddress = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
    if(physicalPageAddress == 0)
    {
        /*Find One Page and swapout*/
        // TODO
        int swapOutPage;
        swapOut(swapOutPage);
        physicalPageAddress = memoryManager.allocatePhysicalPages(AddressPoolType::USER, 1);
    }
    memoryManager.connectPhysicalVirtualPage((int)vaddr, physicalPageAddress);
    for (int i = 0; i < 8; i++)
    {
        char* ptr = (char *)vaddr + i * PAGE_SIZE; 
        Disk::read(index + i + beginSector, (void*)ptr);
    }
    swapResources.release(index, 8);
}
#endif