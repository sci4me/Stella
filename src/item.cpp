enum Item_Type {
    ITEM_COAL_ORE,

    N_ITEM_TYPES
};

struct Item_Stack {
    Item_Type type;
    u32 count;
};

template<u32 size>
struct Item_Container {
    Item_Stack slots[size];

    void init() {
        memset(&slots, 0, sizeof(slots));
    }
};