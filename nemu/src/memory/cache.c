#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "memory/cache.h"
#include "string.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
Cache cache[1024];

void init_cache()
{
    for(int i=0;i<1024;i++) // initiate cache: make each valid bit to zero
        cache[i].valid=0;
}

uint32_t cache_read(paddr_t paddr, size_t len)
{
    uint32_t low_addr=((uint32_t)paddr) & 0x0000003F; // fetch 32 bits' low six bits of the address
    uint32_t mark_bit=((uint32_t)paddr) >> 13; // fetch 32 bits' high nineteen bits of the mark bit
    uint32_t group=(((uint32_t)paddr) & 0x00001FC0) >> 6; // get cache's group number
    uint32_t start_addr=group*8; // find the start address of the group of the data
    bool hit=false;
    for(uint32_t i=0;i<8;i++)
    {
        if(cache[start_addr+i].valid==1)
        {
            if(mark_bit==cache[start_addr+i].mark)
            {
                hit=true;
                if(low_addr+len>64) //cross line
                {
                    uint32_t dataL=0;
                    uint32_t dataH=0;
                    uint32_t lenL=64-low_addr;
                    uint32_t lenH=low_addr+len-64;
                    for(uint32_t j=0;j<lenL;j++) // get this line's data
                    {
                        uint32_t tempdata=cache[start_addr+i].data[low_addr+j];
                        tempdata=(tempdata << 24) >> 24;
                        dataL+=tempdata << (j*8);
                    }
                    dataH=cache_read(paddr+(paddr_t)lenL,(size_t)lenH); // recursion: get next line data
                    return (dataL+(dataH << (lenL*8))) << (32-len*8) >> (32-len*8); // return the combination of this line's data and next line's data
                }
                else //not cross line
                {
                    uint32_t cacheData=0;
                    for(uint32_t j=0;j<len;j++)
                    {
                        uint32_t tempdata=cache[start_addr+i].data[low_addr+j]; // get each 8 bits data from the start address in cache
                        tempdata=(tempdata << 24) >> 24;
                        cacheData+=tempdata << (j*8);
                    }
                    return (cacheData << (32-len*8)) >> (32-len*8);
                }
            }
        }
    }
    if(!hit) // not hit
    {
        bool invalid=false;
        uint32_t line=0;
        for(uint32_t i=0;i<8;i++)
            if(cache[start_addr+i].valid==0) // find the first line which the valid bit is zero
            {
                invalid=true;
                line=start_addr+i;
                cache[start_addr+i].valid=1;
                break;
            }
        if(!invalid) // choose a random line
        {
            srand((unsigned)time(NULL));
            uint32_t ranNum=(uint32_t)rand();
            ranNum=ranNum%8;
            line=start_addr+ranNum;
            cache[start_addr+ranNum].valid=1;
        }
        uint32_t tempaddr=paddr & 0xFFFFFFC0;
        memcpy((void *)cache[line].data,(void *)(hw_mem+tempaddr),64); // fetch the corresponding memory to this whole line
        uint32_t markline=((uint32_t)paddr) >> 13;
        cache[line].mark=(markline << 13) >> 13;
        return cache_read(paddr,len); // recursion: read data
    }
    return 0;
}

void cache_write(paddr_t paddr, size_t len, uint32_t Data)
{
    uint32_t low_addr=((uint32_t)paddr) & 0x0000003F; // fetch 32 bits' low six bits of the address
    uint32_t mark_bit=((uint32_t)paddr) >> 13; // fetch 32 bits' high nineteen bits of the mark bit
    uint32_t group=(((uint32_t)paddr) & 0x00001FC0) >> 6; // get cache's group number
    uint32_t start_addr=group*8; // find the start address of the group of the data
    if(low_addr+len<=64) // no cross line
    {
        for(uint32_t i=0;i<8;i++)
        {
            if(cache[start_addr+i].valid==1)
            {
                if(mark_bit==cache[start_addr+i].mark) // if valid and corresponding, write cache
                {
                    uint32_t tempData=Data;
                    for(int j=0;j<len;j++)
                    {
                        uint8_t tempdata=tempData & 0x000000FF;
                        cache[start_addr+i].data[low_addr+j]=tempdata;
                        tempData=tempData >> 8;
                    }
                    break;
                }
            }
        }
        hw_mem_write(paddr,len,Data); // set data to main memory
    }
    else // cross line
    {
        uint32_t lenL=64-low_addr;
        uint32_t lenH=low_addr+len-64;
        for(uint32_t i=0;i<8;i++)
        {
            if(cache[start_addr+i].valid==1)
            {
                if(mark_bit==cache[start_addr+i].mark) // if valid and corresponding, write cache
                {
                    uint32_t tempData=Data;
                    for(int j=0;j<lenL;j++)
                    {
                        uint8_t tempdata=tempData & 0x000000FF;
                        cache[start_addr+i].data[low_addr+j]=tempdata;
                        tempData=tempData >> 8;
                    }
                    break;
                }
            }
        }
        hw_mem_write(paddr,(size_t)lenL,Data); // set data to main memory
        cache_write(paddr+(paddr_t)lenL,(size_t)lenH,Data >> (lenL*8)); // recursion to write the next line
    }
}
