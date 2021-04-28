#include <internal.hpp>
#include <anton/assert.hpp>

namespace ImGui {
    bool button(u32 const id, anton::String_View const display_text, Button_Style const& style) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        u32 const id_hash = hash_id(id, window->IDStack.back());
        KeepAliveID(id_hash);

        Vec2 const display_text_size = CalcTextSize(display_text.bytes_begin(), display_text.bytes_end(), false);

        Vec2 const text_pos = window->DC.CursorPos + style.padding;
        Vec2 const size = display_text_size + style.padding * 2.0f;
        Vec2 const pos = window->DC.CursorPos;

        ImRect const frame_bb(pos, pos + size);
        ItemSize(size, style.padding.y);
        if(!ItemAdd(frame_bb, id_hash)) {
            return false;
        }

        ImGuiButtonFlags button_flags;
        bool hovered, held;
        bool const pressed = ButtonBehavior(frame_bb, id_hash, &hovered, &held, button_flags);

        // Render
        Vec4 const bg_color = get_interactive_element_color(hovered, held, style.background, style.background_hovered, style.background_active);
        RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(bg_color), false, 0.0f);
        render_text_clipped(display_text, text_pos, frame_bb, style.text_color);

        return pressed;
    }

    bool button(u32 id, anton::String_View display_text) {
        ImGuiStyle& imgui_style = GetStyle();
        Button_Style style;
        style.text_color = imgui_style.Colors[ImGuiCol_Text];
        style.background = imgui_style.Colors[ImGuiCol_button_bg];
        style.background_hovered = imgui_style.Colors[ImGuiCol_button_bg_hovered];
        style.background_active = imgui_style.Colors[ImGuiCol_button_bg_active];
        style.padding = imgui_style.FramePadding;
        return button(id, display_text, style);
    }

    bool invisible_button(u32 const id, Vec2 const size, Vec2 const position) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        ANTON_ASSERT(size.x > 0.0f && size.y > 0.0f, "imgui::invisible_button size should be greater than 0");

        u32 const id_hash = hash_id(id, window->IDStack.back());
        KeepAliveID(id_hash);
        ImRect const bb(position, position + size);
        ItemSize(size);
        if (!ItemAdd(bb, id_hash)) {
            return false;
        }

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id_hash, &hovered, &held, 0);
        return pressed;
    }

    bool invisible_button(u32 const id, Vec2 const size) {
        ImGuiWindow* window = GetCurrentWindow();
        return invisible_button(id, size, window->DC.CursorPos);
    }
}
