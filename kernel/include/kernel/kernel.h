#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <kernel/sys/ports.h>
#include <kernel/sys/gdt.h>
#include <kernel/sys/idt.h>
#include <kernel/sys/isr.h>
#include <kernel/sys/pic.h>
#include <kernel/panic.h>
#include <kernel/sys/paging.h>
#include <kernel/sys/kheap.h>
#include <kernel/sys/pit.h>

#include <kernel/drv/tty.h>
#include <kernel/drv/cursor.h>
#include <kernel/drv/keyboard.h>
#include <kernel/drv/speaker.h>
#include "../../arch/i386/vga.h"
#include <kernel/multiboot.h>
#include <kernel/hal.h>