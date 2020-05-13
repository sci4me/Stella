struct UI {
    virtual void show() = 0;
};

// TODO: Change this to UI_Item_Container.
// Also, probably actually just uh, remove the struct entirely.
// Change this to just a function.
template<u32 width, u32 height>
struct UI_Inventory : public UI {
    const char *name;
    Item_Container<width*height> *container;

    void init(const char *name, Item_Container<width*height> *container) {
        this->name = name;
        this->container = container;
    }

    virtual void show() {
        if(ImGui::Begin(name, 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
            auto style = ImGui::GetStyle();
            auto drawlist = ImGui::GetForegroundDrawList();
            auto font = ImGui::GetFont();
            auto font_size = ImGui::GetFontSize();

            for(u32 j = 0; j < height; j++) {
                for(u32 i = 0; i < width; i++) {
                    u32 k = i + j * width;
                    auto slot = container->slots[k];

                    ImGui::PushID(k);

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

                    if(i < width - 1) ImGui::SameLine();

                    ImGui::PopID();
                }
            }
        }
        ImGui::End();
    }
};