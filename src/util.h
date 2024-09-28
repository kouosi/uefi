#pragma once

#define ENDL         L"\r\n"
#define WARN         L"WARN: "
#define ERROR        L"ERROR: "
#define INFO         L"INFO: "
#define VOIDPTR(var) (void *) var
#define TEST_ERROR(status, halt, ...)     \
    do {                                  \
        if (EFI_ERROR (status) && halt) { \
            Print (__VA_ARGS__);          \
            Print (ENDL L"Halting...");   \
            while (1) {                   \
            };                            \
        } else if (EFI_ERROR (status)) {  \
            Print (__VA_ARGS__);          \
        }                                 \
    } while (0)
