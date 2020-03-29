#include "interrupts.h"


void printf(char* str);



InterruptHandler::InterruptHandler(uint8_t interruptNumber, InterruptManager* interruptManager)
{
    this->interruptNumber = interruptNumber;
    this->interruptManager = interruptManager;
    interruptManager->handlers[interruptNumber] = this;
}
InterruptHandler::~InterruptHandler()
{
    if(interruptManager->handlers[interruptNumber] == this)
    {
        interruptManager->handlers[interruptNumber] = 0;
    }
}

uint32_t InterruptHandler::handleInterrupt(uint32_t esp)
{
    return esp;
}








InterruptManager::GateDescriptor InterruptManager::interruptDescriptorTable[256];

InterruptManager* InterruptManager::ActiveInterruptManager = 0;
void InterruptManager::SetInterruptDescriptorTableEntry(
            uint8_t interruptNumber,
            uint16_t codeSegmentSelectorOffset,
            void (*handler)(),
            uint8_t DescriptorPrivilegeLevel,
            uint8_t DescriptorType
            )
{

    const uint8_t IDT_DESC_PRESENT = 0x80;
    interruptDescriptorTable[interruptNumber].handlerAddressLowBits = ((uint32_t)handler) & 0xFFFF;
    interruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector =  codeSegmentSelectorOffset;
    interruptDescriptorTable[interruptNumber].reserved = 0;
    interruptDescriptorTable[interruptNumber].access = IDT_DESC_PRESENT | DescriptorType | ((DescriptorPrivilegeLevel&3)<< 5);
    interruptDescriptorTable[interruptNumber].handlerAddressHighBits = (((uint32_t)handler) >> 16) & 0xFFFF;
}
    
InterruptManager::InterruptManager(GlobalDescriptorTable* gdt)
:picMasterCommand(0x20),
picMasterData(0x21),
picSlaveCommand(0xA0),
picSlaveData(0xA1)
{
    uint16_t CodeSegment = gdt->CodeSegmentSelector();
    const uint8_t IDT_INTERRUPT_GATE =0xE;


    for(uint16_t i=0;i<256;i++)
    {
        if(i==255)
            printf("done255");
        handlers[i] = 0;
        SetInterruptDescriptorTableEntry(i, CodeSegment, &IgnoreInterruptRequest, 0, IDT_INTERRUPT_GATE);
        
    }

    SetInterruptDescriptorTableEntry(0x20, CodeSegment, &handleInterruptRequest0x00, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x21, CodeSegment, &handleInterruptRequest0x01, 0, IDT_INTERRUPT_GATE);
    SetInterruptDescriptorTableEntry(0x2C, CodeSegment, &handleInterruptRequest0x0C, 0, IDT_INTERRUPT_GATE);



    picMasterCommand.Write(0x11);
    picSlaveCommand.Write(0x11);

    picMasterData.Write(0x20);
    picSlaveData.Write(0x28);

    picMasterData.Write(0x04);
    picSlaveData.Write(0x02);

    picMasterData.Write(0x01);
    picSlaveData.Write(0x01);

    picMasterData.Write(0x00);
    picSlaveData.Write(0x00);
    //tell processor to use IDT
    InterruptDescriptorTablePointer idt;
    idt.size = 256 * sizeof(GateDescriptor) -1;
    idt.base = (uint32_t)interruptDescriptorTable;
    printf("donnnne!!");
    //asm volatile("lidt %0"::"m"(idt));
    __asm__ __volatile__ ("lidt %0" : : "m"(idt));
    printf("\nidt done");

}
InterruptManager::~InterruptManager()
{

}




void InterruptManager::Activate()
{
    if(ActiveInterruptManager !=0)
        ActiveInterruptManager->Deactivate();
    ActiveInterruptManager = this;
    
    asm("sti");
}

void InterruptManager::Deactivate()
{
    if(ActiveInterruptManager == this)
    {
        ActiveInterruptManager = 0;
        asm("cli");
    }
}



uint32_t InterruptManager::handleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
    if(ActiveInterruptManager !=0)
    {
        return ActiveInterruptManager->DohandleInterrupt(interruptNumber,esp);
    }
    return esp;
}


uint32_t InterruptManager::DohandleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
    if(handlers[interruptNumber]!=0)
    {
        esp = handlers[interruptNumber]->handleInterrupt(esp);
    }
    else if(interruptNumber != 0x20)
    {
        char* foo ="UNHANDLED INTERRUPT 0X00";
        char* hex = "0123456789ABDCEF";
        foo[22] = hex[(interruptNumber >> 4) & 0x0F];
        foo[23] = hex[interruptNumber & 0x0F];
        printf("\nINTERRUPT\n");
        printf(foo);
    }

    //Send a reply to PIC so that Interruot handling will not stop

    if(0x20<=interruptNumber && interruptNumber < 0x30)
    {
            picMasterCommand.Write(0x20);
            if(0x28 <= interruptNumber && interruptNumber>=0x30)
                picSlaveCommand.Write(0x20);


    }

    return esp;
}