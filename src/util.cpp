#define _IGNORE__X(X, ...) X(__VA_ARGS__)


// TODO: Do something about this; no sense having a file with just this in it...
static constexpr vec2 QUAD_UVS[4][4] = {
    { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } },
    { { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f } },
    { { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f } }
};


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