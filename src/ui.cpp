namespace ui {
    Item_Container *held_item_container;
    u32 held_item_index;

    void held_item(f32 size = 32.0f) {
        if(!held_item_container) return;

        f32 half_size = size / 2.0f;
        auto p = ImGui::GetMousePos();
        auto font = ImGui::GetFont();
        auto font_size = ImGui::GetFontSize();
        auto drawlist = ImGui::GetForegroundDrawList();
        
        // TODO: don't just use TILE_COAL_ORE's texture lol
        drawlist->AddImage(
            (ImTextureID) tile_textures[TILE_COAL_ORE].id,
            { p.x - half_size, p.y - half_size },
            { p.x + half_size, p.y + half_size }
        );

        char buf[8];
        snprintf(buf, 8, "%d", held_item_container->slots[held_item_index].count);

        auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);
        drawlist->AddText(
            font,
            font_size,
            { p.x - tsize.x + half_size, p.y - tsize.y + half_size },
            0xFFFFFFFF,
            buf
        );
    }

    void slot(Item_Container *container, u32 index, f32 size = 32.0f) {
        ImGui::PushID(index);

        // NOTE: OH NO! A REFERENCE! WHAT EVER WILL SCI4ME DO?!?!!!?!$?!%!
        Item_Stack& slot = container->slots[index];

        auto style = ImGui::GetStyle();
        auto font = ImGui::GetFont();
        auto font_size = ImGui::GetFontSize();
        auto drawlist = ImGui::GetForegroundDrawList();

        bool clicked;
        if(slot.count && !(held_item_container == container && held_item_index == index)) {
            clicked = ImGui::ImageButton((ImTextureID) item_textures[slot.type].id, {size, size});
            
            char buf[8];
            snprintf(buf, 8, "%d", slot.count);

            auto tpos = ImGui::GetItemRectMax();
            auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

            drawlist->AddText(font, font_size, {tpos.x-tsize.x, tpos.y-tsize.y}, 0xFFFFFFFF, buf);
        } else {
            // TODO: Investigate whether we need to push an ID for this call to `ImGui::Button`...
            clicked = ImGui::Button("", {size + style.FramePadding.x*2, size + style.FramePadding.y*2});
        }

        if(clicked) {
            if(held_item_container != nullptr) {
                if(held_item_container == container) {
                    held_item_container = nullptr;
                } else {
                    container->insert(held_item_container->slots[held_item_index]);
                    held_item_container->remove(held_item_index);

                    held_item_container = nullptr;
                }
            } else if(slot.count) {
                held_item_container = container;
                held_item_index = index;
            }
        }

        ImGui::PopID();
    }

    void container(Item_Container *container, u32 width, u32 height, f32 slot_size = 32.0f) {
        assert(width * height == container->size);

        ImGui::PushID(container); // NOTE: not sure if we _need_ this or not.. but.. ehh yeah.
        for(u32 j = 0; j < height; j++) {
            for(u32 i = 0; i < width; i++) {
                u32 k = i + j * width;

                slot(container, k, slot_size);
                if(i < width - 1) ImGui::SameLine();
            }
        }
        ImGui::PopID();
    }

    void tile_ui(Item_Container *player_inventory, Tile **tile_ref) {
        Tile *tile = *tile_ref;
        if(tile) {
            switch(tile->type) {
                case TILE_CHEST: {
                    Tile_Chest *c = (Tile_Chest*) tile;
                    
                    // NOTE: Currently we only have one GUI open at a time,
                    // so we don't have to push any extra ID info.
                    bool open = true;
                    if(ImGui::Begin("Chest", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
                        // NOTE TODO: hardcoded bs code for days
                        ui::container(&c->container, 5, 5);

                        ImGui::Separator();

                        ui::container(player_inventory, 4, 4);
                        
                        ui::held_item();
                    }
                    ImGui::End();

                    if(!open) *tile_ref = nullptr;
                    break;
                }
                default: {
                    assert(0);
                }
            }
        }
    }

    void player_inventory(Item_Container *inventory, bool *open) {
        if(!*open) return;
        
        if(ImGui::Begin("Inventory", open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            // NOTE TODO: hardcoded bs code for days
            ui::container(inventory, 4, 4);
            ui::held_item();
        }
        ImGui::End();
    }
}