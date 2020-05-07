char* read_entire_file(char *name) {
    FILE *fp = fopen(name, "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0L, SEEK_END);
    u32 size = ftell(fp);
    rewind(fp);

    char *code = (char*) malloc(size + 1);

    // TODO: use fread
    for (u32 i = 0; i < size; i++) {
        code[i] = fgetc(fp);
    }
    code[size] = 0;

    return code;
}

f32 randf32() {
    // TODO REMOVEME
    return (f32)rand() / (f32)RAND_MAX;
}

f32 clampf(f32 x, f32 min, f32 max) {
    if(x < min) return min;
    if(x > max) return max;
    return x;
}