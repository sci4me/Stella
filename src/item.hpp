#ifndef ITEM_H
#define ITEM_H

constexpr u32 MAX_ITEM_SLOT_SIZE = 64;


#define _ITEM_DEFS(X, _X) \
    X(_X, ITEM_COBBLESTONE,         "Cobblestone",             false,           tile,       cobblestone         ) \
    X(_X, ITEM_COAL_ORE,            "Coal Ore",                false,           tile,       coal_ore            ) \
    X(_X, ITEM_IRON_ORE,            "Iron Ore",                false,           tile,       iron_ore            ) \
    X(_X, ITEM_GOLD_ORE,            "Gold Ore",                false,           tile,       gold_ore            ) \
    X(_X, ITEM_IRON_PLATE,          "Iron Plate",              false,           item,       iron_plate          ) \
    X(_X, ITEM_GOLD_PLATE,          "Gold Plate",              false,           item,       gold_plate          ) \
    X(_X, ITEM_IRON_GEAR,           "Iron Gear",               false,           item,       iron_gear           ) \
    X(_X, ITEM_CHEST,               "Chest",                   true,            tile,       chest               ) \
    X(_X, ITEM_FURNACE,             "Furnace",                 true,            tile,       furnace             ) \
    X(_X, ITEM_MINING_MACHINE,      "Mining Machine",          true,            tile,       mining_machine      ) \
    X(_X, ITEM_TUBE,                "Tube",                    true,            tile,       tube                )
#define ITEM_DEFS(X) _ITEM_DEFS(_IGNORE__X, X)


typedef u8 Item_Type;
enum Item_Type_ : Item_Type {
    #define _X(id, name, placeable, pfx, texname) id,
    ITEM_DEFS(_X)
    #undef _X

    N_ITEM_TYPES
};


struct Item_Stack {
    Item_Type type;
    u32 count;

    Item_Stack(Item_Type _type, u32 _count) : type(_type), count(_count) {}

    inline bool is_valid() const {
        // NOTE: This does not check if count < MAX_ITEM_SLOT_SIZE on purpose!
        // We want to be able to do things like say 
        // inv->insert(Item_Stack(type, some_count_that_is_above_max_slot_size))
        // in order to just insert into multiple slots.
        //                  - sci4me, 5/18/20
        return type < N_ITEM_TYPES && count > 0;
    }
};

#endif