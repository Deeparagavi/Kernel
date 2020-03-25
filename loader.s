.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

#Magic number in linux means the format of the file.not visible to user

#put above magic and CHECKSUM to .o file
#multibboot struct is given by bootloader
#what bootloader does?
#ax regis - multibootstruct
#bx regis -magicnumber
.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

.section .text
.extern kernelmain
.extern callConstructors
.global loader

loader:
    mov $kernel_stack, %esp
    call callConstructors
    push %eax
    push %ebx
    call kernelmain

_stop:
    #Infinite loop to make kernel alive always
    cli
    hlt
    jmp _stop

.section .bss
.space 2*1024*1024; #2 megabyte
kernel_stack:
