#include <kernel/kernel.h>

void kernel_early(__attribute__((unused)) multiboot_info_t* mbd, __attribute__((unused)) unsigned int magic) {
    terminal_initialize(); 
    init_hal(mbd);
    terminal_startscreen();
    //beep();
    printf("\nWelcome to Veonter v0.0.2!\n");
    printf("/>");


    keyboard_init();
    
}

void kernel_main(void) {
    for (;;) ;
}