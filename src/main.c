#include <efi/efi.h>
#include <efi/x86_64/efibind.h>
#include <stdint.h>
#include <stdbool.h>
#include "gfx.h"
#include "util.h"
#include "font.h"

static void fb_write_char(size_t _x, size_t _y, size_t val) {
	size_t x = 1 + _x * FONT_CELL_WIDTH;
	size_t y = _y * FONT_CELL_HEIGHT;

	const uint8_t * c = font_data[val-32];
	for (uint8_t i = 0; i < FONT_CELL_HEIGHT; ++i) {
		for (uint8_t j = 0; j < FONT_CELL_WIDTH; ++j) {
			bool is_pxl = (bool)(c[i] & (1 << (FONT_MASK-j)));
            gfx_put_pixel(x+j, y+i, is_pxl ? 0xffffffff : BG_COLOR);
		}
	}
}

static void gfx_sandbox (void) {
    size_t fb_height = 0, fb_width = 0;
    gfx_get_fbsize (&fb_width, &fb_height);
    for (size_t y = 0; y < fb_height; y++) {
        for (size_t x = 0; x < fb_width; x++) {
            gfx_put_pixel (x, y, BG_COLOR);
        }
    }
    for (size_t i = 'a';i < 'z'; i++) {
        fb_write_char (i - 'a', 0, i);
    }
}

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    EFI_STATUS status;
    InitializeLib (ImageHandle, SystemTable);

    /* Clear screen */
    status =  uefi_call_wrapper (VOIDPTR (ST->ConOut->ClearScreen), 1, ST->ConOut);
    TEST_ERROR (status, FALSE, WARN L"Failed to clear screen!" ENDL);
    /* Disable watchdog timer */
    status = uefi_call_wrapper (VOIDPTR (ST->BootServices->SetWatchdogTimer), 4, 0, 0, 0, NULL);
    TEST_ERROR (status, TRUE, ERROR L"Failed to disable watchdog timer!" ENDL);
    /* Graphics */
    gfx_init ();
    gfx_set_fbsize (640, 480);
    gfx_sandbox ();
    /* Done */
    while (1);
    return EFI_SUCCESS;
}
