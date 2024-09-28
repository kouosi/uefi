#pragma once
#include <efi/efi.h>
#include <efi/efilib.h>

#define BG_COLOR 0x00222222

EFI_STATUS gfx_init (void);
EFI_STATUS gfx_set_fbsize (uint32_t width, uint32_t height);
void gfx_get_fbsize (size_t *width, size_t *height);
void gfx_put_pixel (size_t x, size_t y, uint32_t pixel);
