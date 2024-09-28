#include <efi/efi.h>
#include <efi/efilib.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"
#include "gfx.h"

static EFI_GRAPHICS_OUTPUT_PROTOCOL *GOP;

void gfx_put_pixel (size_t x, size_t y, uint32_t pixel) {
    *((uint32_t *) (GOP->Mode->FrameBufferBase + 4 * GOP->Mode->Info->PixelsPerScanLine * y + 4 * x)) = pixel;
}

void gfx_get_fbsize (size_t *width, size_t *height) {
    *width = GOP->Mode->Info->HorizontalResolution;
    *height = GOP->Mode->Info->VerticalResolution;
}

EFI_STATUS gfx_set_fbsize (uint32_t width, uint32_t height) {
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN total_modes, gop_info_size, native_mode, selected_mode = 0;
    /* Get current mode */
    status = uefi_call_wrapper (VOIDPTR (GOP->QueryMode), 4, GOP, GOP->Mode == NULL ? 0 : GOP->Mode->Mode,
                                &gop_info_size, &Info);
    /* Set native mode */
    native_mode = GOP->Mode->Mode;
    total_modes = GOP->Mode->MaxMode;
    /* Get all modes */
    for (UINTN i = 0; i < total_modes; i++) {
        status = uefi_call_wrapper (VOIDPTR (GOP->QueryMode), 4, GOP, i, &gop_info_size, &Info);
        // get the mode for the config resolution
        if (Info->HorizontalResolution == width && Info->VerticalResolution == height) {
            selected_mode = i;
            break;
        }
    }
    /* set new mode */
    if (selected_mode != native_mode) {
        status = uefi_call_wrapper (VOIDPTR (GOP->SetMode), 2, GOP, selected_mode);
        TEST_ERROR (status, TRUE, ERROR L"Failed to set mode %03d" ENDL, native_mode);
    } else {
        status = uefi_call_wrapper (VOIDPTR (GOP->SetMode), 2, GOP, GOP->Mode->Mode);
        TEST_ERROR (status, TRUE, ERROR L"Failed to set mode %03d" ENDL, native_mode);
    }
    return status;
}


EFI_STATUS gfx_init (void) {
    EFI_STATUS status;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN gop_info_size;

    /* Locate GOP protocol */
    status = uefi_call_wrapper (VOIDPTR (BS->LocateProtocol), 3, &gop_guid, NULL, (void **) &GOP);
    TEST_ERROR (status && GOP, TRUE, ERROR L"Failed to locate GOP" ENDL);
    /* Get current mode */
    status = uefi_call_wrapper (VOIDPTR (GOP->QueryMode), 4, GOP, GOP->Mode == NULL ? 0 : GOP->Mode->Mode,
                                &gop_info_size, &Info);
    /* This is needed to get the current video mode */
    if (status == EFI_NOT_STARTED) {
        status = uefi_call_wrapper (VOIDPTR (GOP->SetMode), 2, GOP, 0);
    }
    TEST_ERROR (status, TRUE, ERROR L"Failed to get native GOP mode" ENDL);
    return status;
}
