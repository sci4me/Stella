#define min(a, b) ((a)<(b)?(a):(b))
#define max(a, b) ((a)>(b)?(a):(b))

inline f32 square(f32 x) { return x * x; } 

inline glm::vec4 rgba255_to_rgba1(u32 c) {
    u8 r = c & 0xFF;
    u8 g = (c >> 8) & 0xFF;
    u8 b = (c >> 16) & 0xFF;
    u8 a = (c >> 24) & 0xFF;
    return {
        (f32)r / 255.0f,
        (f32)g / 255.0f,
        (f32)b / 255.0f,
        (f32)a / 255.0f
    };
}

inline u32 rgba1_to_rgba255(glm::vec4 c) {
    u8 r = (u8) roundf(c.x * 255.0f);
    u8 g = (u8) roundf(c.y * 255.0f);
    u8 b = (u8) roundf(c.z * 255.0f);
    u8 a = (u8) roundf(c.w * 255.0f);
    return a << 24 | b << 16 | g << 8 | r;
}

inline glm::vec4 rgba1_to_linear(glm::vec4 c) {
    return {
        square(c.x),
        square(c.y),
        square(c.z),
        c.w
    };
}

inline glm::vec4 linear_to_rgba1(glm::vec4 c) {
    return {
        sqrtf(c.x),
        sqrtf(c.y),
        sqrtf(c.z),
        c.w
    };
}

// NOTE: This function expects RGBA values from 0 to 1.
inline glm::vec4 alpha_premultiply(glm::vec4 c) {
    auto p = rgba1_to_linear(c);
    p.x *= p.w;
    p.y *= p.w;
    p.z *= p.w;
    return linear_to_rgba1(p);
}