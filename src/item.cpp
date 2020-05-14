constexpr u32 MAX_ITEM_SLOT_SIZE = 64;

enum Item_Type : u8 {
    ITEM_COAL_ORE,
    ITEM_IRON_ORE,

    ITEM_CHEST,

    N_ITEM_TYPES
};


Texture item_textures[N_ITEM_TYPES];

void load_item_textures() {
    item_textures[ITEM_COAL_ORE] = assets::textures::coal_ore[6];
    item_textures[ITEM_IRON_ORE] = assets::textures::iron_ore[6];
    item_textures[ITEM_CHEST]    = assets::textures::chest;
}


struct Item_Stack {
    Item_Type type;
    u32 count;
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

        slots = (Item_Stack*) calloc(size, sizeof(Item_Stack));
    }

    void free() {
        ::free(slots);
    }

    void sort() {
        if(size == 1) return;

        // TODO: This implementation _can't_ be the most efficient/ideal one.....
        // It's kinda just the naive hacky version... I suspect.

        struct {
            Item_Type key;
            u32 value;
        } *stacks = nullptr;

        for(u32 i = 0; i < size; i++) {
            if(!slots[i].count) continue;
            
            auto key = slots[i].type;
            u32 idx = hmgeti(stacks, key);
            if(idx == -1) {
                hmput(stacks, key, slots[i].count);
            } else {
                stacks[idx].value += slots[i].count;
            }
        }

        memset(slots, 0, size * sizeof(Item_Stack));

        u32 slot = 0;
        for(u32 i = 0; i < hmlen(stacks); i++) {
            u32 remaining = stacks[i].value;
            do {
                u32 n = min(remaining, MAX_ITEM_SLOT_SIZE);
                remaining -= n;
                slots[slot].type = stacks[i].key;
                slots[slot].count = n;
                slot++;
            } while(remaining);
        }

        hmfree(stacks);
    }

    bool accepts_item_type(Item_Type type) {
        if(flags & ITEM_CONTAINER_FLAG_NO_INSERT) return false;
        if(flags & ITEM_CONTAINER_FLAG_FILTER_INSERTIONS) return insertion_filter.get(type);
        return true;
    }

    u32 insert(Item_Stack stack) {
        if(!accepts_item_type(stack.type)) return stack.count;

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
            
            bool needs_sort = false;

            for(u32 i = 0; i < size; i++) {
                if(!slots[i].count) {
                    needs_sort = true;

                    u32 n = min(remaining, MAX_ITEM_SLOT_SIZE);
                    slots[i].type = stack.type;
                    slots[i].count = n;
                    remaining -= n;

                    if(!remaining) break;
                }
            }

            if(needs_sort) sort();
        }

        return remaining;
    }

    void remove(u32 index) {
        assert(index < size);

        memset(&slots[index], 0, sizeof(Item_Stack));
        sort();
    }
};