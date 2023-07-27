#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/Veonter.kernel isodir/boot/Veonter.kernel
cat > isodir/boot/grub/grub.cfg << EOF

set timeout=0

# Установить цвета текста и фона
set menu_color_normal=magenta/white
set menu_color_highlight=white/magenta

menuentry "Veonter 0.0.2" {
 	multiboot /boot/Veonter.kernel
 	module /boot/sayori_sefs.img initrd_sefs;
}

# Добавить записи для перезагрузки и выключения
menuentry "Reboot" {
  reboot
}

menuentry "Shut Down" {
  halt
}
EOF
grub-mkrescue -o Veonter.iso isodir
