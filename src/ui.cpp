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

        // TODO: simplify this entire thing!!! We've got a bunch of duplicated code... etc.

        if(slot.count && !(held_item_container == container && held_item_index == index)) {
            // TODO: don't use TILE_COAL_ORE texture lol
            if(ImGui::ImageButton((ImTextureID)tile_textures[TILE_COAL_ORE].id, {size, size})) {
                if(held_item_container) {
                    if(held_item_container == container) {
                        held_item_container = nullptr;
                    } else {
                        container->insert(held_item_container->slots[held_item_index]);
                        held_item_container->remove(held_item_index);

                        held_item_container = nullptr;
                    }
                } else {
                    held_item_container = container;
                    held_item_index = index;
                }
            }

            char buf[8];
            snprintf(buf, 8, "%d", slot.count);

            auto tpos = ImGui::GetItemRectMax();
            auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

            drawlist->AddText(font, font_size, {tpos.x-tsize.x, tpos.y-tsize.y}, 0xFFFFFFFF, buf);
        } else {
            // TODO: Investigate whether this call to ImGui::Button requires us to push an ID or not...
            if(ImGui::Button("", {size + style.FramePadding.x*2, size + style.FramePadding.y*2})) {
                if(held_item_container) {
                    if(held_item_container == container) {
                        held_item_container = nullptr;
                    } else {
                        container->insert(held_item_container->slots[held_item_index]);
                        held_item_container->remove(held_item_index);

                        held_item_container = nullptr;
                    }
                } else {
                    // NOTE: The user clicked an empty slot while
                    // not holding any item; no-op.
                }
            }
        }

        ImGui::PopID();
    }

    // NOTE: Maybe the right thing to do is just remove this function entirely?
    // If not that, eventually, we'll want to change it and possibly abstract it
    // to support >1 Item_Container, for example if the player opens a chest;
    // they need to have access to _their_ inventory as well as that of the chest.
    //              - sci4me, 5/13/20
    void inventory(const char *name, Item_Container *container, u32 width, u32 height, f32 slot_size = 32.0f) {
        assert(width * height == container->size);

        if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            for(u32 j = 0; j < height; j++) {
                for(u32 i = 0; i < width; i++) {
                    u32 k = i + j * width;

                    slot(container, k, slot_size);
                    if(i < width - 1) ImGui::SameLine();
                }
            }

            held_item(slot_size);
        }
        ImGui::End();
    }
}