#include "keyboard.h"


KeyboardDriver::KeyboardDriver(InterruptManager* manager)
: InterruptHandler(0x21, manager),
  dataport(0x60),
  commandport(0x64)
{
    while(commandport.Read() & 0x1)
    {
        dataport.Read();
    }
    commandport.Write(0xAE);//Tells PIC to give kryboard interrupts or tell kkeyboard to communicate through interrupts.
    commandport.Write(0x20);//gets the keyboard current state.
    uint8_t status = (dataport.Read() | 1) & ~0x10;//read current state and set right most bit to 1 bcs this will be the new state & clear 5th bit
    commandport.Write(0x60); // set state
    dataport.Write(status);


    dataport.Write(0xF4);//activates the keyboard.
}

KeyboardDriver::~KeyboardDriver()
{

}

void printf(char* );

uint32_t KeyboardDriver::handleInterrupt(uint32_t esp)
{
    uint8_t key = dataport.Read();
    if(key < 0x80)
    {
        switch(key)
        {
            //0x45 & 0xC5 is ignored bcs it may be num lock or other key status
            //0xFA & 0xC5 is ignored for same reasom
            case 0x45: case 0xC5 : break;
            case 0x1E: printf("a");break;
            default:
                char* foo ="KEYBOARD 0X00";
                char* hex = "0123456789ABDCEF";
                foo[11] = hex[(key >> 4) & 0x0F];
                foo[12] = hex[key & 0x0F];
                printf("\nKEYBOARD INTERRUPT\n");
                printf(foo);
                break;
        }
    }
    return esp;
}
