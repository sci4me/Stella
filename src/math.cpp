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
    struct Hit {
        bool hit;
        f32 h;
        glm::vec2 n;
    };

    glm::vec2 min;
    glm::vec2 max;

    bool intersects(AABB const& b) const {
        return !(
            (b.max.x <= min.x) ||
            (b.min.x >= max.x) ||
            (b.max.y <= min.y) ||
            (b.min.y >= max.y)
        );
    }

    AABB add(AABB const& b) const {
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

    inline glm::vec2 get_center() const {
        return 0.5f * (min + max);
    }

    inline glm::vec2 get_size() const {
        return max - min;
    }

    inline glm::vec2 get_half_size() const {
        return 0.5f * get_size();
    }

    static AABB from_center(glm::vec2 const& center, glm::vec2 const& half_size) {
        return { center - half_size, center + half_size };
    }

    static Hit sweep(AABB const& a, AABB const& b, glm::vec2 const& delta) {
        f32 inv_x_entry;
        f32 inv_y_entry;
        f32 inv_x_exit;
        f32 inv_y_exit;

        if(delta.x > 0.0f) {
            inv_x_entry = b.min.x - a.max.x;
            inv_x_exit = b.max.x - a.min.x;
        } else {
            inv_x_entry = b.max.x - a.min.x;
            inv_x_exit = b.min.x - a.max.x;
        }

        if(delta.y > 0.0f) {
            inv_y_entry = b.min.y - a.max.y;
            inv_y_exit = b.max.y - a.min.y;
        } else {
            inv_y_entry = b.max.y - a.min.y;
            inv_y_exit = b.min.y - a.max.y;
        }

        f32 x_entry;
        f32 y_entry;
        f32 x_exit;
        f32 y_exit;

        if(delta.x == 0.0f) {
            x_entry = -FLT_MAX;
            x_exit = FLT_MAX;
        } else {
            x_entry = inv_x_entry / delta.x;
            x_exit = inv_x_exit / delta.x;
        }

        if(delta.y == 0.0f) {
            y_entry = -FLT_MAX;
            y_exit = FLT_MAX;
        } else {
            y_entry = inv_y_entry / delta.y;
            y_exit = inv_y_exit / delta.y;
        }

        f32 entry = max(x_entry, y_entry);
        f32 exit = min(x_exit, y_exit);

        if((entry > exit) || ((x_entry < 0.0f) && (y_entry < 0.0f)) || (x_entry > 1.0f) || (y_entry > 1.0f)) {
            return { false, 1.0f };
        }

        assert(entry >= 0.0f && entry <= 1.0f);

        glm::vec2 normal = {
            x_entry > y_entry ? -sign(delta.x) : 0,
            x_entry > y_entry ? 0 : -sign(delta.y)
        };
        return { true, entry, normal };
    }
};