#include "linux_backtrace_h7.h"

//must define '_GNU_SOURCE' or else cause can't find 'Dl_info'
#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

//#include <st.h>

#ifdef __linux__
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static void* parse_symbol_offset(char* frame){
    char* p = NULL;
    char* p_symbol = NULL;
    int nn_symbol = 0;
    char* p_offset = NULL;
    int nn_offset = 0;

    // Read symbol and offset, for example:
    //      /tools/backtrace(foo+0x1820) [0x555555555820]
    for (p = frame; *p; p++) {
        if (*p == '(') {
            p_symbol = p + 1;
        } else if (*p == '+') {
            if (p_symbol) nn_symbol = p - p_symbol;
            p_offset = p + 1;
        } else if (*p == ')') {
            if (p_offset) nn_offset = p - p_offset;
        }
    }
    if (!nn_symbol && !nn_offset) {
        return NULL;
    }

    // Convert offset(0x1820) to pointer, such as 0x1820.
    char tmp[128];
    if (!nn_offset || nn_offset >= sizeof(tmp)) {
        return NULL;
    }

    int r0 = EOF;
    void* offset = NULL;
    tmp[nn_offset] = 0;
    if ((r0 = sscanf(strncpy(tmp, p_offset, nn_offset), "%p", &offset)) == EOF) {
        return NULL;
    }

    // Covert symbol(foo) to offset, such as 0x2fba.
    if (!nn_symbol || nn_symbol >= sizeof(tmp)) {
        return offset;
    }

    void* object_file;
    if ((object_file = dlopen(NULL, RTLD_LAZY)) == NULL) {
        return offset;
    }

    void* address;
    tmp[nn_symbol] = 0;
    if ((address = dlsym(object_file, strncpy(tmp, p_symbol, nn_symbol))) == NULL) {
        dlclose(object_file);
        return offset;
    }

    Dl_info symbol_info;
    if ((r0 = dladdr(address, &symbol_info)) == 0) {
        dlclose(object_file);
        return offset;
    }

    dlclose(object_file);
    return symbol_info.dli_saddr - symbol_info.dli_fbase + offset;
}

static char* addr2line_format(void* addr, char* symbol,
                              char* buffer, int nn_buffer,
                              const char* exe_name)
{
    char cmd[512] = {0};
    int r0 = snprintf(cmd, sizeof(cmd),
                      "addr2line -C -p -s -f -a -e %s %p",
                      exe_name, addr - 1);
    if (r0 < 0 || r0 >= sizeof(cmd)) return symbol;

    FILE* fp = popen(cmd, "r");
    if (!fp) return symbol;

    char* p = fgets(buffer, nn_buffer, fp);
    pclose(fp);

    if (p == NULL) return symbol;
    if ((r0 = strlen(p)) == 0) return symbol;

    // Trait the last newline if exists.
    if (p[r0 - 1] == '\n') p[r0 - 1] = '\0';

    // Find symbol not match by addr2line, like
    //      0x0000000000021c87: ?? ??:0
    //      0x0000000000002ffa: _start at ??:?
    for (p = buffer; p < buffer + r0 - 1; p++) {
        if (p[0] == '?' && p[1] == '?') return symbol;
    }

    return buffer;
}
#endif

#ifdef __linux__
void _get_backtrace2(const char* exe_name)
{
    void* addresses[64];
    int nn_addresses = backtrace(addresses, sizeof(addresses) / sizeof(void*));
    fprintf(stderr, "\naddresses:\n");
    for (int i = 0; i < nn_addresses; i++) {
        fprintf(stderr, "%p\n", addresses[i]);
    }

    char** symbols = backtrace_symbols(addresses, nn_addresses);
    fprintf(stderr, "\nsymbols:\n");
    for (int i = 0; i < nn_addresses; i++) {
        fprintf(stderr, "%s\n", symbols[i]);
    }

    char buffer[512];
    fprintf(stderr, "\nframes:\n");
    for (int i = 0; i < nn_addresses; i++) {
        void* frame = parse_symbol_offset(symbols[i]);
        char* fmt = addr2line_format(frame, symbols[i],
                                     buffer, sizeof(buffer), exe_name);
        int parsed = (fmt == buffer);
        fprintf(stderr, "%p %d %s\n", frame, parsed, fmt);
    }

    free(symbols);

    //printf("bar2 OK\n");
    return;
}
#endif

int always_use_builtin = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wframe-address"
void _get_backtrace() {
    // Each item in the array pointed to by buffer is of type void *, and is the return address from the corresponding
    // stack frame.
    void* addresses[64];
    int nn_addresses = backtrace(addresses, sizeof(addresses) / sizeof(void*));

    if (!nn_addresses || always_use_builtin) {
        printf("Try to get return addresses by __builtin_return_address\n");
        void* p = NULL; nn_addresses = 0;
        if ((p = __builtin_return_address(0)) != NULL) {
            addresses[nn_addresses++] = p;
            if ((p = __builtin_return_address(1)) != NULL) {
                addresses[nn_addresses++] = p;
                if ((p = __builtin_return_address(2)) != NULL) {
                    addresses[nn_addresses++] = p;
                    if ((p = __builtin_return_address(3)) != NULL) {
                        addresses[nn_addresses++] = p;
                        if ((p = __builtin_return_address(4)) != NULL) {
                            addresses[nn_addresses++] = p;
                            if ((p = __builtin_return_address(5)) != NULL) {
                                addresses[nn_addresses++] = p;
                            }
                        }
                    }
                }
            }
        }
    }

    char** symbols = backtrace_symbols(addresses, nn_addresses);
    fprintf(stderr, "nn_addresses=%d, symbols=%p, symbols[0]=%p\n", nn_addresses, symbols, symbols[0]);

    fprintf(stderr, "\naddresses:\n");
    for (int i = 0; i < nn_addresses; i++) {
        fprintf(stderr, "%p\n", addresses[i]);
    }

    fprintf(stderr, "\nsymbols:\n");
    for (int i = 0; i < nn_addresses; i++) {
        fprintf(stderr, "%s\n", symbols[i]);
    }
    free(symbols);

    //printf("bar OK\n");
    return;
}
#pragma GCC diagnostic pop

