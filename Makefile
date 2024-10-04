BUILD_DIR = build
CC_FLAG = -Wall -Wextra -fpic -ffreestanding -fno-stack-protector \
		  -fno-stack-check -fshort-wchar -mno-red-zone \
		  -maccumulate-outgoing-args -DEFI_FUNCTION_WRAPPER
QEMU_FLAGS = -m 128M -enable-kvm -net none -cpu host
CC_SRCS = $(shell find src -type f -name '*.c')
CC_HDR = $(shell find src -type f -name '*.h')
OBJS = $(patsubst src/%.c, ${BUILD_DIR}/%.o,$(CC_SRCS))

.PHONY: all
all: clean ${BUILD_DIR}/test.img

.PHONY: run
run: ${BUILD_DIR}/test.img ovmf/ovmf.fd
	@qemu-system-x86_64 -bios ovmf/ovmf.fd -hda ${BUILD_DIR}/test.img

${BUILD_DIR}/test.img: ${BUILD_DIR}/loader.efi Makefile
	@dd if=/dev/zero of=$@ bs=512 count=98304
	@sfdisk -q $@ < <(printf "label: gpt\n sector-size: 512\n type=uefi, bootable")
	@mkfs.fat -F 32 --offset=2048 -S 512 $@ $$((96256 * 512 / 1024))
	@mmd -i $@@@1M ::/EFI
	@mmd -i $@@@1M ::/EFI/BOOT
	@mcopy -i $@@@1M ${BUILD_DIR}/loader.efi ::/EFI/BOOT/BOOTX64.EFI

${BUILD_DIR}/loader.efi: $(CC_HDR) $(OBJS) Makefile
	@echo "LD 	$@"
	@cp /usr/lib/crt0-efi-x86_64.o /usr/lib/libgnuefi.a /usr/lib/libefi.a ${BUILD_DIR}/
	@ld -nostdlib -shared -Bsymbolic -L${BUILD_DIR} `ls ${BUILD_DIR}/*.o` \
		-o ${BUILD_DIR}/loader.so -lefi -lgnuefi -T src/link.ld
	@objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym \
		-j .rel  -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 \
		--subsystem=10 ${BUILD_DIR}/loader.so $@

.PHONY: ${BUILD_DIR}/%.o
${BUILD_DIR}/%.o: src/%.c Makefile
	@echo "CC 	$<"
	@mkdir -p $(@D)
	@gcc $(CC_FLAG) -c $< -o $@

ovmf/ovmf.fd:
	@mkdir -p ovmf
	@cp /usr/share/ovmf/x64/OVMF.fd ovmf/ovmf.fd

clean:
	@rm -rf ${BUILD_DIR}/*.efi ${BUILD_DIR}/*.o ${BUILD_DIR}/*.a ${BUILD_DIR}/*.so *.img
