char* tvsprintf(char const* fmt, va_list args) {
    va_list args2;
    va_copy(args2, args);
    s32 len = stbsp_vsnprintf(0, 0, fmt, args);
    va_end(args);

    char *buf = (char*) talloc(len);

    stbsp_vsprintf(buf, fmt, args2);
    va_end(args2);

    buf[len] = 0;

    return buf;
}

char* tsprintf(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    return tvsprintf(fmt, args);
}

void tprintf(char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *buf = tvsprintf(fmt, args);
    mlc_fwrite(STDOUT, buf);
}

void tfprintf(s32 fd, char const* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *buf = tvsprintf(fmt, args);
    mlc_fwrite(STDOUT, buf);
}


void* mlc_realloc(void *p, u64 n) {
    void *p2 = mlc_alloc(n);
    if(!p2) return 0;
    
    if(!p) return p2;

    u64 old_n = *(((u64*) p) - 1) - sizeof(u64);
    if(n < old_n) n = old_n;
    mlc_memcpy(p2, p, n);

    mlc_free(p);
    return p2;
}