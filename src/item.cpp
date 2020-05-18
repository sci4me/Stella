constexpr u32 MAX_ITEM_SLOT_SIZE = 64;

enum Item_Type : u8 {
    ITEM_COBBLESTONE,
    ITEM_COAL_ORE,
    ITEM_IRON_ORE,
    ITEM_GOLD_ORE,

    ITEM_IRON_PLATE,
    ITEM_GOLD_PLATE,

    ITEM_IRON_GEAR,

    ITEM_CHEST,
    ITEM_FURNACE,
    ITEM_MINING_MACHINE,

    N_ITEM_TYPES
};


Texture item_textures[N_ITEM_TYPES];
const char *item_names[N_ITEM_TYPES];

void init_items() {
    item_textures[ITEM_COBBLESTONE]     = assets::textures::cobblestone[6];
    item_textures[ITEM_COAL_ORE]        = assets::textures::coal_ore[6];
    item_textures[ITEM_IRON_ORE]        = assets::textures::iron_ore[6];
    item_textures[ITEM_GOLD_ORE]        = assets::textures::gold_ore[6];
    item_textures[ITEM_IRON_PLATE]      = assets::textures::iron_plate;
    item_textures[ITEM_GOLD_PLATE]      = assets::textures::gold_plate;
    item_textures[ITEM_IRON_GEAR]       = assets::textures::iron_gear;
    item_textures[ITEM_CHEST]           = assets::textures::chest;
    item_textures[ITEM_FURNACE]         = assets::textures::furnace;
    item_textures[ITEM_MINING_MACHINE]  = assets::textures::mining_machine;

    item_names[ITEM_COBBLESTONE]        = "Cobblestone";
    item_names[ITEM_COAL_ORE]           = "Coal Ore";
    item_names[ITEM_IRON_ORE]           = "Iron Ore";
    item_names[ITEM_GOLD_ORE]           = "Gold Ore";
    item_names[ITEM_IRON_PLATE]         = "Iron Plate";
    item_names[ITEM_GOLD_PLATE]         = "Gold Plate";
    item_names[ITEM_IRON_GEAR]          = "Iron Gear";
    item_names[ITEM_CHEST]              = "Chest";
    item_names[ITEM_FURNACE]            = "Furnace";
    item_names[ITEM_MINING_MACHINE]     = "Mining Machine";
}


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

    u32 insert(Item_Stack const& s, bool resort = true) {
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

        memset(&slots[index], 0, sizeof(Item_Stack));
        sort();
    }

    bool remove(Item_Type type, u32 count, bool resort = true) {
        return remove(Item_Stack(type, count), resort);
    }

    bool remove(Item_Stack const& s, bool resort = true) {
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