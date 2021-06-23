#include <internal.hpp>
#include <anton/math/math.hpp>

namespace ImGui {
    using namespace anton::literals;

    bool begin_window_context_menu() {
        u32 const id = make_id("window_context"_sv);
        return begin_window_context_menu(id);
    }

    bool begin_window_context_menu(Context_Menu_Style style) {
        u32 const id = make_id("window_context"_sv);
        return begin_window_context_menu(id, style);
    }

    bool begin_window_context_menu(u32 const id) {
        ImGuiWindow* const window = GImGui->CurrentWindow;
        u32 const id_hash = hash_id(id, window->IDStack.back());
        KeepAliveID(id_hash);

        int mouse_button = ImGuiMouseButton_Right;
        if(IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            OpenPopupEx(id_hash);
        }

        return BeginPopupEx(id_hash, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
    }

    bool begin_window_context_menu(u32 const id, Context_Menu_Style style) {
        ImGuiWindow* const window = GImGui->CurrentWindow;
        u32 const id_hash = hash_id(id, window->IDStack.back());
        KeepAliveID(id_hash);

        int mouse_button = ImGuiMouseButton_Right;
        if(IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            OpenPopupEx(id_hash);
        }

        PushStyleColor(ImGuiCol_PopupBg, style.background);
        bool const opened = BeginPopupEx(id_hash, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
        PopStyleColor();
        return opened;
    }

    void end_context_menu() {
        EndPopup();
    }

    void close_context_menu() {
        CloseCurrentPopup();
    }

    // bool context_menu_item(u32 id, anton::String_View text, Menu_Item_Options item) {
    //     return false;
    // }

    bool context_menu_item(u32 const id, anton::String_View const text, Menu_Item_Options const options, Menu_Item_Style const style) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        ImGuiContext& g = *GImGui;
        f32 const font_size = g.FontSize;
        Vec2 const text_size = calculate_text_size(text);

        // Save the size for the next frame
        f32 const context_menu_width = window->DC.context_menu_width;
        {
            f32 const max_current = anton::math::max(text_size.x, context_menu_width);
            window->DC.next_context_menu_width = anton::math::max(window->DC.next_context_menu_width, max_current);
        }

        u32 const id_hash = hash_id(id, window->IDStack.back());
        KeepAliveID(id_hash);

        Vec2 const position = window->DC.CursorPos;
        Vec2 const spaced_box_min = position;
        Vec2 const spaced_box_max = spaced_box_min + 2.0f * (style.spacing + style.padding) + Vec2{context_menu_width, text_size.y};
        Vec2 const box_min = spaced_box_min + style.spacing;
        Vec2 const box_max = spaced_box_max - style.spacing;
        Vec2 const text_min = box_min + style.padding;
        Vec2 const text_max = box_max - style.padding;

        // f32 const min_x = window->ParentWorkRect.Min.x;
        // f32 const max_x = window->ParentWorkRect.Max.x;
        
        ImRect spaced_bb{spaced_box_min, spaced_box_max};
        ItemSize(spaced_bb);
        if(!options.disabled) {
            if(!ItemAdd(spaced_bb, id_hash)) {
                return false;
            }
        } else {
            ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
            window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
            bool const item_add = ItemAdd(spaced_bb, id_hash);
            window->DC.ItemFlags = backup_item_flags;
            if(!item_add) {
                return false;
            }
        }

        ImGuiButtonFlags button_flags = ImGuiButtonFlags_PressedOnRelease;
        if(options.disabled) {
            button_flags |= ImGuiButtonFlags_Disabled;
        }

        ImRect bb{box_min, box_max};
        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id_hash, &hovered, &held, button_flags);

        if(pressed) {
            MarkItemEdited(id_hash);
        }

        Vec4 const background = get_interactive_element_color(hovered, held, style.background, style.background_hover, style.background_active);
        render_frame(bb, background);

        Vec4 const text_color = !options.disabled ? style.text : style.text_disabled;
        ImRect text_clip{text_min, text_max};
        render_text_clipped(text, text_min, text_clip, text_color);

        return pressed;
    }

    void context_menu_separator() {
        ImGuiStyle& style = GImGui->Style;
        Vec2 const spacing = style.FramePadding;
        Vec4 const color = style.Colors[ImGuiCol_context_menu_separator];
        context_menu_separator(spacing, color);
    }

    void context_menu_separator(Vec2 const spacing, Vec4 const color) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return;
        }


        ImGuiContext& ctx = *GImGui;
        f32 const font_size = ctx.FontSize;
        u32 const color_u32 = GetColorU32(color);
        f32 const content_width = GetContentRegionAvail().x;
        Vec2 cursor = window->DC.CursorPos;

        ImRect bb{cursor, cursor + Vec2{content_width, 1.0f + 2.0f * spacing.y}};
        if(!ItemAdd(bb, 0)) {
            return;
        }

        // Add spacing above
        cursor.y += spacing.y;
        window->DrawList->AddLine(cursor + Vec2{spacing.x, 0.0f}, cursor + Vec2{content_width - spacing.x, 0.0f}, color_u32, 1.0f);
        // Add spacing below + 1 pixel for the line width
        cursor.y += spacing.y + 1.0f;
        window->DC.CursorPos = cursor;
    }
} // namespace ImGui
