const f32 PI = 3.14159265359f;


struct ivec2 {
    s32 x;
    s32 y;

    ivec2() : x(0), y(0) {}
    ivec2(s32 _x, s32 _y) : x(_x), y(_y) {}
};


struct vec2 {
    f32 x;
    f32 y;

    constexpr vec2() : x(0), y(0) {}
    constexpr vec2(f32 _x, f32 _y) : x(_x), y(_y) {}

#ifndef IMGUI_DISABLE
    vec2(ImVec2 const& v) : x(v.x), y(v.y) {}
    operator ImVec2() const { return ImVec2(x, y); }
#endif

    f32 length_squared() { return x*x + y*y; }
    f32 length() { return sqrtf32(length_squared()); }

    vec2& operator+=(vec2 const& b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    vec2& operator-=(vec2 const& b) {
        x -= b.x;
        y -= b.y;
        return *this;
    }
};

vec2 operator+(vec2 const& a, vec2 const& b) { return { a.x + b.x, a.y + b.y }; }
vec2 operator-(vec2 const& a, vec2 const& b) { return { a.x - b.x, a.y - b.y }; }
vec2 operator*(vec2 a, f32 b) { return { a.x * b, a.y * b }; }
vec2 operator*(f32 b, vec2 a) { return a * b; }

vec2 normalize(vec2 a) {
    f32 len = a.length();
    return vec2(a.x / len, a.y / len);
}

f32 distance(vec2 a, vec2 b) {
    return (a - b).length();
}


struct vec4 {
    f32 x;
    f32 y;
    f32 z;
    f32 w;

    constexpr vec4() : x(0), y(0), z(0), w(0) {}
    constexpr vec4(f32 _x, f32 _y, f32 _z, f32 _w) : x(_x), y(_y), z(_z), w(_w) {}

#ifndef IMGUI_DISABLE
    vec4(ImVec4 const& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    operator ImVec4() const { return ImVec4(x, y, z, w); }
#endif
    
    f32 length_squared() { return x*x + y*y + z*z + w*w; }
    f32 length() { return sqrtf32(length_squared()); }

    vec4& operator+=(vec4 const& b) {
        x += b.x;
        y += b.y;
        z += b.w;
        w += b.w;
        return *this;
    }

    vec4& operator-=(vec4 const& b) {
        x -= b.x;
        y -= b.y;
        z -= b.z;
        w -= b.w;
        return *this;
    }
};

vec4 operator+(vec4 const& a, vec4 const& b) { return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; }
vec4 operator-(vec4 const& a, vec4 const& b) { return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }; }
vec4 operator*(vec4 const& a, f32 b) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }
vec4 operator*(f32 b, vec4 const& a) { return a * b; }


struct mat4 {
    f32 m[4][4];

    mat4() {
        m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
        m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
        m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
        m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
    }

    mat4 operator*(mat4 const& b) {
        // NOTE TODO: We may want to optimize this eventually.
        mat4 r;
        mlc_memset(&r, 0, sizeof(mat4));

        for(int i = 0; i < 4; i++) {
            for(int j = 0; j < 4; j++) {
                for(int k = 0; k < 4; k++) {
                    r.m[i][j] += m[i][k] * b.m[k][j];
                }
            }    
        }

        return r;
    }

    f32* value_ptr() {
        return &m[0][0];
    }

    static mat4 ortho(f32 left, f32 right, f32 bottom, f32 top, f32 z_near, f32 z_far) {
        f32 w = right - left;
        f32 h = top - bottom;
        f32 d = z_far - z_near;

        mat4 r = {};
        r.m[0][0] = 2.0f / w;
        r.m[1][1] = 2.0f / h; 
        r.m[2][2] = 2.0f / d;
        r.m[3][0] = -(right + left) / w;
        r.m[3][1] = -(top + bottom) / h;
        r.m[3][2] = -z_near / d;       
        return r;
    }

    static mat4 translate(f32 x, f32 y, f32 z) {
        mat4 r = {};
        r.m[3][0] = x;
        r.m[3][1] = y;
        r.m[3][2] = z;
        return r;
    }

    static mat4 scale(f32 x, f32 y, f32 z) {
        mat4 r = {};
        r.m[0][0] = x;
        r.m[1][1] = y;
        r.m[2][2] = z;
        return r;
    }
};


template<typename T>
constexpr T min(T a, T b) noexcept { return a < b ? a : b; }

template<typename T>
constexpr T max(T a, T b) noexcept { return a > b ? a : b; }

// NOTE: These must be declared after the templatized
// versions as they are specializations.....
constexpr vec2 min(vec2 a, vec2 b) noexcept { return vec2(min(a.x, b.x), min(a.y, b.y)); }
constexpr vec2 max(vec2 a, vec2 b) noexcept { return vec2(max(a.x, b.x), max(a.y, b.y)); }

template<typename T>
constexpr T square(T x) noexcept { return x * x; }

template<typename T>
constexpr T sign(T x) noexcept { return x < 0 ? -1 : 1; }

template<typename T>
constexpr T clamp(T x, T min, T max) noexcept {
    if(x < min) return min;
    if(x > max) return max;
    return x;
}

template<typename T>
constexpr T lerp(T t, T a, T b) noexcept {
    return a + t * (b - a);
}


u32 next_pow2_u32(u32 y) {
    u32 x = y;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}


inline vec4 rgba255_to_rgba1(u32 c) {
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

inline u32 rgba1_to_rgba255(vec4 c) {
    u8 r = (u8) roundf32(c.x * 255.0f);
    u8 g = (u8) roundf32(c.y * 255.0f);
    u8 b = (u8) roundf32(c.z * 255.0f);
    u8 a = (u8) roundf32(c.w * 255.0f);
    return a << 24 | b << 16 | g << 8 | r;
}

inline vec4 rgba1_to_linear(vec4 c) {
    return {
        square(c.x),
        square(c.y),
        square(c.z),
        c.w
    };
}

inline vec4 linear_to_rgba1(vec4 c) {
    return {
        sqrtf32(c.x),
        sqrtf32(c.y),
        sqrtf32(c.z),
        c.w
    };
}

// NOTE: This function expects RGBA values from 0 to 1.
inline vec4 alpha_premultiply(vec4 c) {
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
        vec2 n;
    };

    vec2 min;
    vec2 max;

    bool intersects(AABB const& b) const {
        return !(
            (b.max.x <= min.x) ||
            (b.min.x >= max.x) ||
            (b.max.y <= min.y) ||
            (b.min.y >= max.y)
        );
    }

    inline vec2 get_center() const {
        return 0.5f * (min + max);
    }

    inline vec2 get_size() const {
        return max - min;
    }

    inline vec2 get_half_size() const {
        return 0.5f * get_size();
    }

    static AABB from_center(vec2 const& center, vec2 const& half_size) {
        return { center - half_size, center + half_size };
    }

    static AABB disjunction(AABB const& a, AABB const& b) {
        return { ::min(a.min, b.min), ::max(a.max, b.max) };
    }

    static Hit sweep(AABB const& a, AABB const& b, vec2 const& delta) {
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

        f32 entry = ::max(x_entry, y_entry);
        f32 exit = ::min(x_exit, y_exit);

        if((entry > exit) || ((x_entry < 0.0f) && (y_entry < 0.0f)) || (x_entry > 1.0f) || (y_entry > 1.0f)) {
            return { false, 1.0f };
        }

        assert(entry >= 0.0f && entry <= 1.0f);

        vec2 normal = {
            x_entry > y_entry ? -sign(delta.x) : 0,
            x_entry > y_entry ? 0 : -sign(delta.y)
        };
        return { true, entry, normal };
    }
};