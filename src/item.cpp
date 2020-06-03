constexpr char const* item_names[N_ITEM_TYPES] = {
    #define _X(id, name, placeable, pfx, texname) [id] = name,
    ITEM_DEFS(_X)
    #undef _X
};

constexpr bool item_is_placeable[N_ITEM_TYPES] = {
    #define _X(id, name, placeable, pfx, texname) [id] = placeable,
    ITEM_DEFS(_X)
    #undef _X
};


struct Item_Stack {
    Item_Type type;
    u32 count;

    Item_Stack(Item_Type _type, u32 _count) : type(_type), count(_count) {}

    bool is_valid() const {
        // NOTE: This does not check if count < MAX_ITEM_SLOT_SIZE on purpose!
        // We want to be able to do things like say 
        // inv->insert(Item_Stack(type, some_count_that_is_above_max_slot_size))
        // in order to just insert into multiple slots.
        //                  - sci4me, 5/18/20
        return type < N_ITEM_TYPES && count > 0;
    }
};


typedef u8 Item_Container_Flags;
enum Item_Container_Flags_ : Item_Container_Flags {
    ITEM_CONTAINER_FLAG_NONE                    = 0,
    ITEM_CONTAINER_FLAG_NO_INSERT               = 1,
    ITEM_CONTAINER_FLAG_NO_EXTRACT              = 2,
    ITEM_CONTAINER_FLAG_FILTER_INSERTIONS       = 4
};

struct Item_Container {
    Item_Container_Flags flags;

    // NOTE: Static_Bitset may not be the best way to implement this;
    // we're using up 32 bytes for something that isn't used extremely
    // often (?). But for now, blegh, who cares. Profiling will tell me
    // if this needs to change.
    //              - sci4me, 5/13/20

    // NOTE: 4 words * 64 bits per word = 256 bits
    Static_Bitset<4> insertion_filter;

    // NOTE: If Item_Stack ever needs to hold much more
    // data than it "currently" does (type and count)
    // then, we'll probably want to change these to pointers?
    // Or maybe not? I don't know...
    Item_Stack *slots;
    u32 size;

    void init(u32 size, Item_Container_Flags flags = ITEM_CONTAINER_FLAG_NONE) {
        this->flags = flags;

        assert(size > 0);
        this->size = size;

        slots = (Item_Stack*) mlc_calloc(size, sizeof(Item_Stack));
    }

    void deinit() {
        mlc_free(slots);
    }

    void sort() {
        TIMED_FUNCTION();

        if(size == 1) return;

        // TODO: This implementation _can't_ be the most efficient/ideal one.....
        // It's kinda just the naive hacky version... I suspect.

        // NOTE TODO: Maybe use qsort? I don't freaking know lol.
        // I'm not going to mess with this until I see it being significant
        // in profiling results, and we're nowhere near there yet. So.
        // But yeh, I don't know what sorting algorithm would suit this task.
        // Sometimes it do be like that.
        //                  - sci4me, 5/28/20

        Hash_Table<Item_Type, u32> stacks;

        // NOTE: We set the size to 30% more than the number of slots we have;
        // this should guarantee that we never have to resize the table.
        // We then round up to the next power of 2 since the table requires that.
        //                  - sci4me, 5/21/20
        stacks.init(next_pow2_u32(size + (u32)(size * (1.0f - HASH_TABLE_LOAD_FACTOR_EXPAND_THRESHOLD))));

        for(u32 i = 0; i < size; i++) {
            if(!slots[i].count) continue;
            
            auto key = slots[i].type;
            s32 idx = stacks.index_of(key);
            if(idx == -1) {
                stacks.set(key, slots[i].count);
            } else {
                stacks.slots[idx].value += slots[i].count;
            }
        }

        mlc_memset(slots, 0, size * sizeof(Item_Stack));

        u32 slot = 0;
        for(u32 i = 0; i < stacks.size; i++) {
            if(stacks.slots[i].hash == 0) continue;

            auto key = stacks.slots[i].key;
            u32 remaining = stacks.slots[i].value;
            do {
                u32 n = min(remaining, MAX_ITEM_SLOT_SIZE);
                remaining -= n;
                slots[slot].type = key;
                slots[slot].count = n;
                slot++;
            } while(remaining);
        }

        stacks.deinit();
    }

    bool accepts_item_type(Item_Type type) {
        if(flags & ITEM_CONTAINER_FLAG_NO_INSERT) return false;
        if(flags & ITEM_CONTAINER_FLAG_FILTER_INSERTIONS) return insertion_filter.get(type);
        return true;
    }

    u32 insert(Item_Stack const& s, bool resort = true) {
        TIMED_FUNCTION();

        assert(s.is_valid());

        if(!accepts_item_type(s.type)) return s.count;

        u32 remaining = s.count;

        // First try to insert into slots that 
        // already contain this item type.
        for(u32 i = 0; i < size; i++) {
            if(slots[i].count > 0 && slots[i].type == s.type) {
                u32 available = MAX_ITEM_SLOT_SIZE - slots[i].count;
                if(available) {
                    u32 n = min(remaining, available);
                    slots[i].count += n;
                    remaining -= n;

                    if(!n) return 0;
                }
            }
        }

        if(remaining) {
            // Next, try to find free slots to insert into.
            
            bool needs_sort = false;

            for(u32 i = 0; i < size; i++) {
                if(!slots[i].count) {
                    needs_sort = true;

                    u32 n = min(remaining, MAX_ITEM_SLOT_SIZE);
                    slots[i].type = s.type;
                    slots[i].count = n;
                    remaining -= n;

                    if(!remaining) break;
                }
            }

            if(needs_sort && resort) sort();
        }

        return remaining;
    }

    void clear_slot(u32 index) {
        assert(index < size);

        mlc_memset(&slots[index], 0, sizeof(Item_Stack));
        sort();
    }

    bool remove(Item_Type type, u32 count, bool resort = true) {
        return remove(Item_Stack(type, count), resort);
    }

    bool remove(Item_Stack const& s, bool resort = true) {
        TIMED_FUNCTION();

        assert(s.is_valid());

        if(!contains(s)) return false;

        u32 remaining = s.count;
        for(u32 i = 0; i < size; i++) {
            if(slots[i].type == s.type) {
                u32 n = min(slots[i].count, remaining);
                slots[i].count -= n;
                remaining -= n;

                if(!remaining) break;
            }
        }

        assert(!remaining);
        
        // NOTE: There are definitely cases where this isn't necessary.
        // But it's easier, for now _at least_, to just ignore that and
        // re-sort every time.
        if(resort) sort();

        return true;
    }

    u32 total_count() {
        u32 count = 0;
        for(u32 i = 0; i < size; i++) {
            count += slots[i].count;
        }
        return count;
    }

    u32 count_type(Item_Type type) {
        u32 count = 0;
        for(u32 i = 0; i < size; i++) {
            if(slots[i].type == type) count += slots[i].count;
        }
        return count;
    }

    bool contains(Item_Stack const& s) {
        return count_type(s.type) >= s.count;
    }
};