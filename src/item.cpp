constexpr u32 MAX_ITEM_SLOT_SIZE = 64;

enum Item_Type {
    ITEM_COAL_ORE,

    N_ITEM_TYPES
};

struct Item_Stack {
    Item_Type type;
    u32 count;
};


struct Item_Container {
    // NOTE: If Item_Stack ever needs to hold much more
    // data than it "currently" does (type and count)
    // then, we'll probably want to change these to pointers?
    // Or maybe not? I don't know...
    Item_Stack *slots;
    u32 size;

    void init(u32 size) {
        assert(size > 0);
        this->size = size;

        slots = (Item_Stack*) calloc(size, sizeof(Item_Stack));
    }

    void free() {
        ::free(slots);
    }

    u32 insert(Item_Stack stack) {
        u32 remaining = stack.count;

        // First try to insert into slots that 
        // already contain this item type.
        for(u32 i = 0; i < size; i++) {
            if(slots[i].count > 0 && slots[i].type == stack.type) {
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
            for(u32 i = 0; i < size; i++) {
                if(!slots[i].count) {
                    u32 n = min(remaining, MAX_ITEM_SLOT_SIZE);
                    slots[i].type = stack.type;
                    slots[i].count = n;
                    remaining -= n;

                    if(!n) return 0;
                }
            }

            // TODO: (maybe) sort the inventory
        }

        return remaining;
    }

    void remove(u32 index) {
        assert(index < size);

        memset(&slots[index], 0, sizeof(Item_Stack));
        // TODO: sort the inventory
    }
};