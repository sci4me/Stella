// NOTE: This is extremely similar to Static_Array
// but, at the moment, I feel like keeping them separate
// since they have different intended uses.
//          - sci4me, 5/12/20

template<typename T, u32 size>
struct Slot_Allocator {
    T slots[size];
    s32 count;

    void clear() {
        count = 0;
    }

    s32 alloc(T x) {
        if(count == size) return -1;
        
        for(s32 i = 0; i < count; i++) {
            if(slots[i] == x) return i;
        }

        slots[count] = x;
        return count++;
    }
};