#define min(a, b) ((a)<(b)?(a):(b))
#define max(a, b) ((a)>(b)?(a):(b))
#define square(x) ((x)*(x))
#define sign(x) (((x)<(0))?(-1):(1))

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


struct AABB {
    glm::vec2 min;
    glm::vec2 max;

    bool intersects(AABB const& b) {
        return !(
            (b.max.x <= min.x) ||
            (b.min.x >= max.x) ||
            (b.max.y <= min.y) ||
            (b.min.y >= max.y)
        );
    }

    AABB add(AABB const& b) {
        return {
            {
                min(min.x, b.min.x),
                min(min.y, b.min.y)
            },
            {
                max(max.x, b.max.x),
                max(max.y, b.max.y)
            }
        };
    }

    static AABB from_center(glm::vec2 const& center, glm::vec2 const& half_size) {
        return { center - half_size, center + half_size };
    }
};


f32 line_segment_intersection_no_parallel(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d) {
    auto e = b - a;
    auto f = d - c;
    glm::vec2 p = { -e.y, e.x };

    auto denom = glm::dot(f, p);
    assert(denom != 0);

    return glm::dot(a - c, p) / denom;
}