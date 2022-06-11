#include "bitmap.h"
#include "stdlib.h"
#include "stdio.h"

BitMap::BitMap()
{
}

void BitMap::initialize(char *bitmap, const int length)
{
    this->bitmap = bitmap;
    this->length = length;

    int bytes = ceil(length, 8);

    for (int i = 0; i < bytes; ++i)
    {
        bitmap[i] = 0;
    }
}

bool BitMap::get(const int index) const
{
    int pos = index / 8;
    int offset = index % 8;

    return (bitmap[pos] & (1 << offset));
}

void BitMap::set(const int index, const bool status)
{
    int pos = index / 8;
    int offset = index % 8;

    // 清0
    bitmap[pos] = bitmap[pos] & (~(1 << offset));

    // 置1
    if (status)
    {
        bitmap[pos] = bitmap[pos] | (1 << offset);
    }
}

int BitMap::allocate(const int count)
{
    /*
    // firstfit 实现
    if (count == 0)
        return -1;

    int index, empty, start;

    index = 0;
    while (index < length)
    {
        // 越过已经分配的资源
        while (index < length && get(index))
            ++index;

        // 不存在连续的count个资源
        if (index == length)
            return -1;

        // 找到1个未分配的资源
        // 检查是否存在从index开始的连续count个资源
        empty = 0;
        start = index;
        while ((index < length) && (!get(index)) && (empty < count))
        {
            ++empty;
            ++index;
        }

        // 存在连续的count个资源
        if (empty == count)
        {
            for (int i = 0; i < count; ++i)
            {
                set(start + i, true);
            }

            return start;
        }
    }

    return -1;
    

   if (count == 0)
        return -1;

    int index, empty, start;
    // best fit实现
    int minReserve = 0x7f7f7f, findResource = 0;

    index = 0;
    while (index < length)
    {
        // 越过已经分配的资源
        while (index < length && get(index))
            ++index;

        // 不存在连续的count个资源
        if (index == length)
            return -1;

        // 找到1个未分配的资源
        // 检查是否存在从index开始的连续count个资源
        empty = 0;
        // start = index;
        while ((index < length) && (!get(index)))
        {
            ++empty;
            ++index;
        }
        // printf_warning("[Update Address]: minReserve is %d; start is %d.\n",minReserve,start);

        // 存在连续的count个资源
        if (empty >= count)
        {
            if((empty - count) <= minReserve)
            {
                // Best-fit: 更小则更新
                minReserve = empty - count;
                start = index - empty + 1;
                // printf_warning("[Update Address]: minReserve is %d; start is %d.\n",minReserve,start);
                findResource = 1;
            }
            
        }
    }
    if(findResource)
    {
        for (int i = 0; i < count; ++i)
        {
            set(start + i, true);
        }

        return start;
    }
    return -1;
    */

    if (count == 0)
        return -1;

    int index, empty, start;
    // worst fit实现
    int maxReserve = 0, findResource = 0;

    index = 0;
    while (index < length)
    {
        // 越过已经分配的资源
        while (index < length && get(index))
            ++index;

        // 不存在连续的count个资源
        if (index == length)
            return -1;

        // 找到1个未分配的资源
        // 检查是否存在从index开始的连续count个资源
        empty = 0;
        // start = index;
        while ((index < length) && (!get(index)))
        {
            ++empty;
            ++index;
        }
        // printf_warning("[Update Address]: maxReserve is %d; start is %d.\n",maxReserve,start);

        // 存在连续的count个资源
        if (empty >= count)
        {
            if((empty - count) >= maxReserve)
            {
                // Best-fit: 更小则更新
                maxReserve = empty - count;
                start = index - empty + 1;
                // printf_warning("[Update Address]: maxReserve is %d; start is %d.\n",maxReserve,start);
                findResource = 1;
            }
            
        }
    }
    if(findResource)
    {
        for (int i = 0; i < count; ++i)
        {
            set(start + i, true);
        }

        return start;
    }
    return -1;
}

void BitMap::release(const int index, const int count)
{
    for (int i = 0; i < count; ++i)
    {
        set(index + i, false);
    }
}

char *BitMap::getBitmap()
{
    return (char *)bitmap;
}

int BitMap::size() const
{
    return length;
}