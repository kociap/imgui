#include <internal.hpp>

namespace ImGui {
    bool collapsible_header(anton::String_View display_text, u32 id, Collapsible_Header_Options const& options, Collapsible_Header_Style const& style) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        u32 const id_hash = hash_label_with_id(display_text, id, window->IDStack.back());
        KeepAliveID(id_hash);

        ImGuiContext& ctx = *GImGui;
        ImGuiStyle& imgui_style = ctx.Style;
        ImGuiStorage& storage = *window->DC.StateStorage;

        bool const is_open = storage.GetInt(id_hash, options.open_by_default) != 0;
        Vec2 const padding = imgui_style.FramePadding;
        Vec2 const arrow_size{2.0f * padding + ctx.FontSize};
        ImRect const frame_rect{{window->WorkRect.Min.x, window->DC.CursorPos.y}, {window->WorkRect.Max.x, window->DC.CursorPos.y + padding.y * 2.0f + ctx.FontSize}};
        ImRect const text_rect{{frame_rect.Min.x + arrow_size.x, frame_rect.Min.y + padding.y}, frame_rect.Max - padding};
        ItemSize(frame_rect);
        if(!ItemAdd(frame_rect, id_hash)) {
            return is_open;
        }

        bool hovered, held;
        bool const pressed = ButtonBehavior(frame_rect, id_hash, &hovered, &held, ImGuiButtonFlags_PressedOnClickRelease);
        bool const open_state = pressed ? !is_open : is_open;
        storage.SetInt(id_hash, open_state);

        Vec4 const background_color = get_interactive_element_color(hovered, held, style.background, style.background_hovered, style.background_active);
        render_frame(frame_rect, background_color);

        Vec4 const text_color = style.text_color;
        render_text_clipped(display_text, text_rect.Min, text_rect, text_color);
        RenderArrow(window->DrawList, Vec2(frame_rect.Min + padding), GetColorU32(text_color), open_state ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);

        return open_state;
    }

    bool collapsible_header(anton::String_View display_text, u32 id, Collapsible_Header_Options const& options) {
        ImGuiStyle& imgui_style = GetStyle();
        Collapsible_Header_Style style;
        style.text_color = imgui_style.Colors[ImGuiCol_Text];
        style.background = imgui_style.Colors[ImGuiCol_collapsible_header_bg];
        style.background_hovered = imgui_style.Colors[ImGuiCol_collapsible_header_bg_hovered];
        style.background_active = imgui_style.Colors[ImGuiCol_collapsible_header_bg_active];
        return collapsible_header(display_text, id, options, style);
    }
}
