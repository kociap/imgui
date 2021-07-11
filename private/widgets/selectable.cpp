#include <internal.hpp>

namespace ImGui {
    bool selectable(u32 id_param, anton::String_View text, Selectable_Options options) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        // TODO: We currently do not have an option for this.
        //       Is it even necessary?
        bool const span_all_columns = false; 
        if(span_all_columns && window->DC.CurrentColumns) {
            // FIXME-OPT: Avoid if vertically clipped.
            PushColumnsBackground();
        } 

        // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
        u32 const id = hash_id(id_param, window->IDStack.back());
        KeepAliveID(id);
        ImVec2 label_size = calculate_text_size(text);
        // This used the size parameter, but we have removed it.
        ImVec2 size = label_size;
        ImVec2 pos = window->DC.CursorPos;
        pos.y += window->DC.CurrLineTextBaseOffset;
        ItemSize(size, 0.0f);

        // Fill horizontal space
        const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
        const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
        // if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth))
        size.x = ImMax(label_size.x, max_x - min_x);

        // Text stays at the submission position, but bounding box may be extended on both sides
        const ImVec2 text_min = pos;
        const ImVec2 text_max(min_x + size.x, pos.y + size.y);

        // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
        ImRect bb(min_x, pos.y, text_max.x, text_max.y);
        // if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
        // {
        //     const float spacing_x = style.ItemSpacing.x;
        //     const float spacing_y = style.ItemSpacing.y;
        //     const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
        //     const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
        //     bb.Min.x -= spacing_L;
        //     bb.Min.y -= spacing_U;
        //     bb.Max.x += (spacing_x - spacing_L);
        //     bb.Max.y += (spacing_y - spacing_U);
        // }

        bool item_add;
        if(options.disabled) {
            ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
            window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
            item_add = ItemAdd(bb, id);
            window->DC.ItemFlags = backup_item_flags;
        } else {
            item_add = ItemAdd(bb, id);
        }

        if(!item_add) {
            if(span_all_columns && window->DC.CurrentColumns) {
                PopColumnsBackground();
            }
            return false;
        }

        bool selected = options.selected && !options.disabled;
        // TODO: We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
        ImGuiButtonFlags button_flags = ImGuiButtonFlags_PressedOnRelease;
        if (options.disabled) {
            button_flags |= ImGuiButtonFlags_Disabled;
            selected = false;
        }

        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
        if(pressed) {
            MarkItemEdited(id);
        }

        // if(flags & ImGuiSelectableFlags_AllowItemOverlap)
        //     SetItemAllowOverlap();

        // Render
        // if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld)) {
        //     hovered = true;
        // }

        if(hovered || selected) {
            const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
            RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
            RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
        }

        if(span_all_columns && window->DC.CurrentColumns) {
            PopColumnsBackground();
        }

        if(options.disabled) {
            PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]);
        }

        RenderTextClipped(text_min, text_max, text.bytes_begin(), text.bytes_end(), &label_size, style.SelectableTextAlign, &bb);
        
        if(options.disabled) {
            PopStyleColor();
        }

        // Automatically close popups
        // TODO: We don't have an option for that yet
        bool const should_close_popup = !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup);
        if(pressed && (window->Flags & ImGuiWindowFlags_Popup) && should_close_popup) {
            CloseCurrentPopup();
        }

        return pressed;
    }
}
