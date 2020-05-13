namespace ui {
    Item_Stack *held_item_stack;

    void held_item(f32 size = 32.0f) {
        if(!held_item_stack) return;

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
        snprintf(buf, 8, "%d", held_item_stack->count);

        auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);
        drawlist->AddText(
            font,
            font_size,
            { p.x - tsize.x + half_size, p.y - tsize.y + half_size },
            0xFFFFFFFF,
            buf
        );
    }

    template<u32 width, u32 height>
    void slot(Item_Container<width, height> *container, u32 index, f32 size = 32.0f) {
        ImGui::PushID(index);

        // NOTE: OH NO! A REFERENCE! WHAT EVER WILL SCI4ME DO?!?!!!?!$?!%!
        Item_Stack& slot = container->slots[index];

        auto style = ImGui::GetStyle();
        auto font = ImGui::GetFont();
        auto font_size = ImGui::GetFontSize();
        auto drawlist = ImGui::GetForegroundDrawList();

        if(slot.count && held_item_stack != &slot) {
            // TODO: don't use TILE_COAL_ORE texture lol
            if(ImGui::ImageButton((ImTextureID)tile_textures[TILE_COAL_ORE].id, {size, size})) {
                if(held_item_stack) {
                    assert(0);
                } else {
                    held_item_stack = &slot;
                }
            }

            char buf[8];
            snprintf(buf, 8, "%d", slot.count);

            auto tpos = ImGui::GetItemRectMax();
            auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

            drawlist->AddText(font, font_size, {tpos.x-tsize.x, tpos.y-tsize.y}, 0xFFFFFFFF, buf);
        } else {
            if(ImGui::Button("", {size + style.FramePadding.x*2, size + style.FramePadding.y*2})) {
                if(held_item_stack == &slot) {
                    held_item_stack = nullptr;
                } else {
                    assert(0);
                }
            }
        }

        ImGui::PopID();
    }

    template<u32 width, u32 height>
    void inventory(const char *name, Item_Container<width, height> *container, f32 slot_size = 32.0f) {
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