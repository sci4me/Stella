namespace crafting {
    struct Recipe {
        Item_Stack *inputs;
        u32 n_inputs;
        Item_Stack output;
        u32 time; // in ticks
    };

    // TODO: Whenever I get around to making the Arena in sci.h
    // have a configurable block size, consider using that here
    // instead of malloc.
    //              - sci4me, 5/14/20
    Recipe **recipes = nullptr;

    void register_recipe(Item_Stack output, u32 time, std::initializer_list<Item_Stack> inputs) {
        auto r = (Recipe*) malloc(sizeof(Recipe) + sizeof(Item_Stack) * inputs.size());

        r->inputs = (Item_Stack*) (r + 1); // inputs are right after the Recipe struct
        r->n_inputs = inputs.size();
        r->output = output;
        r->time = time;

        u32 i = 0;
        for(auto s : inputs) {
            r->inputs[i++] = s;
        }

        arrput(recipes, r);
    }

    void init() {
        register_recipe(
            { ITEM_FURNACE, 1 },
            100,
            {
                { ITEM_COBBLESTONE, 8 }
            }
        );
    }

    void free() {
        for(u32 i = 0; i < arrlen(recipes); i++) {
            ::free(recipes[i]);
        }
        arrfree(recipes);
    }


    struct Queue {
        Item_Container *inventory;
        Recipe **queue = nullptr;

        Recipe *actively_crafting = nullptr;
        u32 time_left;

        void init(Item_Container *inventory) {
            this->inventory = inventory;
        }

        void deinit() {
            arrfree(queue);
        }

        bool request(Recipe *r) {
            // TODO: Right now we just check if we have the inputs to the recipe
            // and craft it with those. Soon, we'll want to be smarter about this.
            // If the player wants to craft say, I don't know, a mining machine,
            // and say it takes like 8 iron plates, 8 cobblestone, and 2 iron gears,
            // but they don't have the gears, they just have enough iron to make them;
            // we need to request those gears be crafted before the mining machine, 
            // automatically. This gets more complicated when you consider the cases
            // where they have _some_ of the intermediaries but not all of them.
            //
            // Shouldn't be too hard to "brute-force" this though. I think. inb4.
            //
            //                  - sci4me, 5/14/20

            for(u32 i = 0; i < r->n_inputs; i++) {
                if(!inventory->contains(r->inputs[i])) return false;
            }

            for(u32 i = 0; i < r->n_inputs; i++) {
                assert(inventory->remove(r->inputs[i]));
            }

            arrput(queue, r);
            return true;
        }

        void update() {
            if(actively_crafting) {
                if(time_left) {
                    time_left--;
                    return;
                }

                u32 remaining = inventory->insert(actively_crafting->output);
                if(remaining) {
                    // TODO: We need to essentially hold onto the rest of the items to-be-inserted and just insert them whever we can?
                    // This case can definitely happen, say if the player requests a bunch of stuff to be crafted and then
                    // mines a bunch of ores or grabs a bunch of stuff from another inventory.
                    assert(0);
                } else {
                    actively_crafting = nullptr;
                }
            } else if(arrlen(queue) > 0) {
                actively_crafting = queue[0];
                time_left = actively_crafting->time;

                arrdel(queue, 0);
            }
        }

        void draw() {
                
        }
    };
}