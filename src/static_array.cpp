// NOTE: NOT thread safe!

template<typename T, u32 capacity>
struct Static_Array {
    T data[capacity];
    u32 count;

    void clear() {
        count = 0;
    }

    u32 push(T datum) {
        assert(count < capacity);
        data[count] = datum;
        return count++;
    }

    T pop() {
        assert(count > 0);
        return data[--count];
    }

    T& operator[](s32 x) { return data[x]; }
    T const& operator[](s32 x) const { return data[x]; }
};