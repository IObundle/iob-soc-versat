#include "printf.h"

// Only purpose is to push the HandleInterrupt function into the 0x80000020 address, basically hacking away since we do not use a better way to put functions in the addresses that we want 
void NopFunction(){
   asm("nop");
   asm("nop");
   asm("nop");
}

__attribute__((interrupt)) void HandleInterrupt(){
   printf("An interrupt occured\n");
}
