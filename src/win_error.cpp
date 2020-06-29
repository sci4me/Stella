#include "win_error.h"
//#include "stb_sprintf.h"

constexpr size_t min_error_message_length = 1024;
char* win_error_message_buffer;
char panic_format_out[1024];

[[noreturn]] void panic_win(int line, char const* file) {
    auto error_code = GetLastError();

    if(!error_code) {
        stbsp_sprintf(panic_format_out, "[%s line %d]\nGot null, but no error code found.", file, line);
        mlc_fwrite(STDOUT, panic_format_out);
        //MessageBoxA(nullptr, panic_format_out, "Error", MB_ICONERROR);
        //mlc_exit(-1);
    }

    DWORD char_count = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error_code, 0, (LPSTR) (void*) &win_error_message_buffer, min_error_message_length, nullptr);
    // TODO: Probably wasn't needed anyway? Decide later.
    //defer(if(win_error_message_buffer) LocalFree(win_error_message_buffer), win_error_message_buffer);

    if(char_count == 0) {
        // The error code did not exist in the system errors.
        // Try Ntdsbmsg.dll for the error code.

        HINSTANCE lib = LoadLibraryA("Ntdsbmsg.dll");

        if(lib != NULL) {
            char_count = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER, lib, error_code, 0, (LPSTR) (void*) &win_error_message_buffer, min_error_message_length, nullptr);
            FreeLibrary(lib);
        }
    }

    if(char_count == 0) {
        stbsp_sprintf(panic_format_out, "[%s line %d]\nUnknown Windows error: 0x%X", file, line, error_code);
        mlc_fwrite(STDOUT, panic_format_out);
        //MessageBoxA(nullptr, panic_format_out, "Error", MB_ICONERROR);
        //mlc_exit(-1);
    }

    stbsp_sprintf(panic_format_out, "[%s line %d]\n%s", file, line, win_error_message_buffer);
    mlc_fwrite(STDOUT, panic_format_out);
    //MessageBoxA(nullptr, panic_format_out, "Error", MB_ICONERROR);
    //mlc_exit(-1);
}

void _panic_if_win_err_impl(DWORD error_code, int line, char const* file) {
    if(error_code == S_OK) return;
    SetLastError(error_code);
    panic_win(line, file);
}