#include <utility.hpp>

namespace ImGui {
    static float CalcMaxPopupHeightFromItemCount(int items_count) {
        ImGuiContext& g = *GImGui;
        if (items_count <= 0) {
            return FLT_MAX;
        }
        return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
    }

    bool begin_combo(anton::String_View const label, u32 const id, anton::String_View const preview_value) {
        // Always consume the SetNextWindowSizeConstraint() call in our early return paths
        ImGuiContext& g = *GImGui;
        bool has_window_size_constraint = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
        g.NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        ImGuiStyle const& style = g.Style;
        u32 const id_hash = hash_label_with_id(label, id, window->IDStack.back());
        KeepAliveID(id_hash);

        f32 const arrow_size = GetFrameHeight();
        Vec2 const label_pos = window->DC.CursorPos + Vec2(0.0f, style.FramePadding.y);
        Vec2 const label_size = CalcTextSize(label.bytes_begin(), label.bytes_end());
        f32 const w = CalcItemWidth();
        f32 const horizontal_spacing = label.size_bytes() != 0 ? style.ItemInnerSpacing.x : 0.0f;
        Vec2 const frame_pos{window->DC.CursorPos.x + label_size.x + horizontal_spacing, window->DC.CursorPos.y};
        ImRect const frame_bb{frame_pos, frame_pos + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f)};
        ImRect const total_bb{window->DC.CursorPos, frame_bb.Max};
        ItemSize(total_bb, style.FramePadding.y);
        if(!ItemAdd(total_bb, id_hash, &frame_bb)) {
            return false;
        }

        bool hovered, held;
        bool pressed = ButtonBehavior(frame_bb, id_hash, &hovered, &held);
        bool popup_open = IsPopupOpen(id_hash, ImGuiPopupFlags_None);

        render_text(label, label_pos, style.Colors[ImGuiCol_Text]);

        ImU32 const frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        // TODO: Why would frame_bb.Max.x be - arrow_size < frame_bb.Min.x be true
        f32 const value_x2 = ImMax(frame_bb.Min.x, frame_bb.Max.x - arrow_size);
        // Render value preview
        window->DrawList->AddRectFilled(frame_bb.Min, ImVec2(value_x2, frame_bb.Max.y), frame_col, style.FrameRounding, ImDrawCornerFlags_Left);
        Vec2 const preview_pos = frame_pos + style.FramePadding;
        // TODO: The clip rect should be shrank by framepadding
        render_text_clipped(preview_value, preview_pos, frame_bb, style.Colors[ImGuiCol_Text]);
        // Render drop-down arrow
        ImU32 bg_col = GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        ImU32 text_col = GetColorU32(ImGuiCol_Text);
        window->DrawList->AddRectFilled(ImVec2(value_x2, frame_bb.Min.y), frame_bb.Max, bg_col, style.FrameRounding, (w <= arrow_size) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right);
        if(value_x2 + arrow_size - style.FramePadding.x <= frame_bb.Max.x) {
            RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, ImGuiDir_Down, 1.0f);
        }
        // ??
        RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);

        if((pressed || g.NavActivateId == id_hash) && !popup_open) {
            if(window->DC.NavLayerCurrent == 0) {
                window->NavLastIds[0] = id_hash;
            }
            OpenPopupEx(id_hash, ImGuiPopupFlags_None);
            popup_open = true;
        }

        if(!popup_open) {
            return false;
        }

        ImGuiComboFlags flags = ImGuiComboFlags_None;
        if(has_window_size_constraint) {
            g.NextWindowData.Flags |= ImGuiNextWindowDataFlags_HasSizeConstraint;
            g.NextWindowData.SizeConstraintRect.Min.x = ImMax(g.NextWindowData.SizeConstraintRect.Min.x, w);
        } else {
            if ((flags & ImGuiComboFlags_HeightMask_) == 0)
                flags |= ImGuiComboFlags_HeightRegular;
            IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_));    // Only one
            int popup_max_height_in_items = -1;
            if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
            else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
            else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
            SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
        }

        char name[16];
        ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

        // Peak into expected window size so we can position it
        if(ImGuiWindow* popup_window = FindWindowByName(name)) {
            if (popup_window->WasActive) {
                ImVec2 size_expected = CalcWindowExpectedSize(popup_window);
                if (flags & ImGuiComboFlags_PopupAlignLeft)
                    popup_window->AutoPosLastDirection = ImGuiDir_Left;
                ImRect r_outer = GetWindowAllowedExtentRect(popup_window);
                ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
                SetNextWindowPos(pos);
            }
        }

        // We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove;

        // Horizontally align ourselves with the framed text
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
        bool ret = Begin(name, NULL, window_flags);
        PopStyleVar();
        if(!ret) {
            EndPopup();
            IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
            return false;
        }
        return true;
    }

    void end_combo() {
        EndPopup();
    }
}
