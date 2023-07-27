#include <kernel/kernel.h>
bool initRD = FALSE;
multiboot_info_t* multiboot;
/**
 * @brief Монтирует виртуальный диск с файловой системой Sayori Easy File System
 *
 * @param int irdst - Точка монтирования
 */
void initrd_sefs(size_t irdst, size_t irded){
    if (initRD){
        return;
    }

    qemu_log("[InitRD] [SEFS] Initialization of the virtual disk. The SEFS virtual file system is used.");
    qemu_log("[InitRD] [SEFS] The virtual disk space is located at address %x.", irdst);
    qemu_log("[InitRD] [SEFS] The virtual disk space is ends at %x.", irded);
    
    //vfs_reg(irdst, irded, VFS_TYPE_MOUNT_SEFS);
    
    initRD = true;
}

void kModules_Init(){
    qemu_log("[kModules] Loading operating system modules...");
    uint32_t*	mod_start = 0;
    uint32_t*	mod_end = 0;
    uint32_t	mods_count = multiboot->mods_count;
    
    char* mod_cmd[16];

    if (mods_count > 0){
		mod_start = (uint32_t*) kmalloc(sizeof(uint32_t)*mods_count);
		mod_end = (uint32_t*) kmalloc(sizeof(uint32_t)*mods_count);

        qemu_log("[kModules] Found '%d' modules",mods_count);

		for (size_t i = 0; i < mods_count; i++){
			mod_start[i] = *(uint32_t*)(multiboot->mods_addr + 8*i);
			mod_end[i] = *(uint32_t*)(multiboot->mods_addr + 8*i + 4);

            multiboot_tag_module_t *mod = (multiboot_tag_module_t *) (uint32_t*)(multiboot->mods_addr + 8*i);
            
            mod_cmd[i] = kcalloc(strlen((char*)mod->cmdline) + 1, sizeof(char));
            strcpy(mod_cmd[i], (char*)mod->cmdline);
            
            qemu_log("[kModules] Found module number `%d`. (Start: %x | End: %x) CMD: %s",i,mod_start[i],mod_end[i],mod_cmd[i]);
            
            if (strcmpn(mod_cmd[i],"initrd_sefs")){
                initrd_sefs(mod_start[i], mod_end[i]);
                continue;
            }
		}
	} else {
        qemu_log("[kModules] No modules were connected to this operating system.");
    }
}


void kernel_early(__attribute__((unused)) multiboot_info_t* mbd, __attribute__((unused)) unsigned int magic) {
    terminal_initialize(); 
    init_hal(mbd);
	multiboot = mbd;
    check();
    printf("PC Speaker testing!\n\n");
    beep(6, 10);
    terminal_startscreen();
    printf("Com INIT: %d",__com_init(0x3f8));
	kModules_Init();
    printf("\nWelcome to Veonter v0.0.2!\n");
    printf("/>");


    keyboard_init();
    
}

void kernel_main(void) {
    for (;;) ;
}