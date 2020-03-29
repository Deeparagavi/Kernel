#include "gdt.h"

GlobalDescriptorTable::GlobalDescriptorTable()
:nullSegmentSelector(0,0,0),
unusedSegmentSelector(0,0,0),
codeSegmentSelector(0, 64*1024*1024, 0x9A),
dataSegmentSelector(0, 64*1024*1024, 0x92) //to know abt this param vals check lowlevel.edu
{

    uint32_t i[2];
    i[1] = (uint32_t)this;
    i[0] = sizeof(GlobalDescriptorTable)<<16;
    
    // To tell processor to use This GlobalDescriptorTable
    asm volatile("lgdt (%0)"::"p"(((uint8_t *) i) +2));

}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
    // unload GlobalDescriptorTable
}

uint16_t GlobalDescriptorTable::DataSegmentSelector()
{
    return (uint8_t*)&dataSegmentSelector - (uint8_t*)this;
}


uint16_t GlobalDescriptorTable::CodeSegmentSelector()
{
    return (uint8_t*)&codeSegmentSelector - (uint8_t*)this;
}

GlobalDescriptorTable::SegmentDescriptor::SegmentDescriptor(uint32_t base,uint32_t limit,uint8_t flags)
{

    uint8_t* target = (uint8_t*)this;

    if(limit <= 65536)
    {
        //if its 16 bit address
        target[6] = 0x40;
    }
    else
    {
        // length 20 bits is not sufficent.
        // so number 2+(1/2)=20 bits will be added by 12 (i.e)2 pow 20 multiplied by 2 pow 12
        // these 12 is virtual  bit
        // this can be only done when virtual numbers is all one
        // this can also be made one if not but that will go beyond the expected limit
        // So we subtract 1 from actual limit and make virtual values to all ones when they are not
        if((limit & 0XFFFF) != 0xFFFF)
        {
            limit = (limit >> 12)-1;
        }
        else
        {
            limit = limit >> 12;
        }
        target[6] = 0xc0;
    }
    target[0] = limit & 0xFF;
    target[1] = (limit >>8) & 0xFF;
    target[6] |= (limit >>16) & 0xFF;

    target[2] = base  & 0XFF;
    target[3] = (base >> 8) & 0XFF;
    target[4] = (base >> 16) & 0XFF;
    target[7] = (base >> 24) & 0XFF;  
    target[5] = flags;
}


uint32_t GlobalDescriptorTable::SegmentDescriptor::Base()
{
    uint8_t* target = (uint8_t*)this;
    uint32_t result = target[7];
    result = (result << 8) + target[4];
    result = (result << 8) + target[3];
    result = (result << 8) + target[2];

    return result;
}

uint32_t GlobalDescriptorTable::SegmentDescriptor::Limit()
{
    uint8_t* target = (uint8_t*)this;
    uint32_t result = target[6] & 0xF;
    result = (result << 8) + target[1];
    result = (result << 8) + target[0];
    
    if((target[6] &0xC0) == 0xC0 )
    {
        result = (result << 12) | 0xFF;
    }
    return result;
    
}