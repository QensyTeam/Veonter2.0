#include <kernel/kernel.h>

void kernel_early(__attribute__((unused)) multiboot_info_t* mbd, __attribute__((unused)) unsigned int magic) {
    terminal_initialize(); 
    init_hal(mbd);
    check();
    printf("PC Speaker testing!\n\n");
    beep(6, 10);
    terminal_startscreen();
    printf("Com INIT: %d",__com_init(0x3f8));
	qemu_log("Hyi");
	qemu_log("Surpice %d",0x0000);
    printf("\nWelcome to Veonter v0.0.2!\n");
    printf("/>");


    keyboard_init();
    
}

void kernel_main(void) {
    for (;;) ;
}