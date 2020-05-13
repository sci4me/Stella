namespace ui {
    template<u32 width, u32 height>
    void slot(Item_Container<width, height> *container, u32 index) {
        ImGui::PushID(index);

        // NOTE: OH NO! A REFERENCE! WHAT EVER WILL SCI4ME DO?!?!!!?!$?!%!
        Item_Stack& slot = container->slots[index];

        auto style = ImGui::GetStyle();
        auto drawlist = ImGui::GetForegroundDrawList();
        auto font = ImGui::GetFont();
        auto font_size = ImGui::GetFontSize();

        if(slot.count) {
            if(ImGui::ImageButton((ImTextureID)tile_textures[TILE_COAL_ORE].id, {32, 32})) {
                // TODO: handle the case where a user clicked this slot with no item held
                // TODO: handle the case where a user clicked this slot with an item held
            }

            char buf[8];
            snprintf(buf, 8, "%d", slot.count);

            auto tpos = ImGui::GetItemRectMax();
            auto tsize = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, buf);

            drawlist->AddText(font, font_size, {tpos.x-tsize.x, tpos.y-tsize.y}, 0xFFFFFFFF, buf);
        } else {
            if(ImGui::Button("", {32 + style.FramePadding.x*2, 32 + style.FramePadding.y*2})) {
                // TODO: handle the case where a user clicked this slot with an item held
            }
        }

        ImGui::PopID();
    }

    template<u32 width, u32 height>
    void inventory(const char *name, Item_Container<width, height> *container) {
        if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            for(u32 j = 0; j < height; j++) {
                for(u32 i = 0; i < width; i++) {
                    u32 k = i + j * width;

                    slot(container, k);
                    if(i < width - 1) ImGui::SameLine();
                }
            }
        }
        ImGui::End();
    }
}