#include <utility.hpp>

namespace ImGui {
    bool drag_i64(anton::String_View const label, u32 const id, i64& value, i64 const step, i64 const v_min, i64 const v_max) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        u32 const id_hash = hash_label_with_id(label, id, window->IDStack.back());
        KeepAliveID(id_hash);

        ImGuiContext& ctx = *GImGui;
        ImGuiStyle const& style = GImGui->Style;
        Vec2 const widget_pos = window->DC.CursorPos;
        // Render label
        Vec2 const label_pos = Vec2{widget_pos.x, widget_pos.y + style.FramePadding.y};
        Vec2 const label_size = CalcTextSize(label.bytes_begin(), label.bytes_end());
        render_text(label, label_pos, style.Colors[ImGuiCol_Text]);
        
        f32 const w = CalcItemWidth();
        Vec2 const frame_pos = label_pos + Vec2{(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), -style.FramePadding.y};
        ImRect const frame_bb(frame_pos, frame_pos + Vec2(w, label_size.y + style.FramePadding.y * 2.0f));
        ImRect const total_bb(widget_pos, frame_bb.Max);
        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id_hash, &frame_bb)) {
            return false;
        }

        {
            bool const hovered = ItemHoverable(frame_bb, id_hash);
            bool const temp_input_allowed = true; // (flags & ImGuiSliderFlags_NoInput) == 0;
            bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id_hash);
            if(!temp_input_is_active) {
                bool const focus_requested = temp_input_allowed && FocusableItemRegister(window, id_hash);
                bool const clicked = (hovered && ctx.IO.MouseClicked[0]);
                bool const double_clicked = (hovered && ctx.IO.MouseDoubleClicked[0]);
                if (focus_requested || clicked || double_clicked) {
                    SetActiveID(id_hash, window);
                    SetFocusID(id_hash, window);
                    FocusWindow(window);
                    if (temp_input_allowed && (focus_requested || double_clicked)) {
                        temp_input_is_active = true;
                        FocusableItemUnregister(window);
                    }
                }
            }
            
            if(temp_input_is_active) {
                // TempInputScalar displays an input field with the label on the right side. 
                // We work around that by supplying empty label.
                return TempInputScalar(frame_bb, id_hash, "", ImGuiDataType_S64, &value, "%I64d", &v_min, &v_max);
            }
        }


        // Draw frame
        bool const hovered = ctx.HoveredId == id_hash;
        bool const active = ctx.ActiveId == id_hash;
        Vec4 const frame_color = get_interactive_element_color(hovered, active, style.Colors[ImGuiCol_FrameBg], style.Colors[ImGuiCol_FrameBgHovered], style.Colors[ImGuiCol_FrameBgActive]);
        render_frame(frame_bb, frame_color);

        // Drag behavior
        bool const value_changed = DragBehavior(id_hash, ImGuiDataType_S64, &value, step, &v_min, &v_max, "%I64d", ImGuiSliderFlags_None);
        if (value_changed) {
            MarkItemEdited(id_hash);
        }

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        char value_buf[64];
        char const* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), ImGuiDataType_S64, &value, "%I64d");
        anton::String_View const text{value_buf, value_buf_end};
        // center text vertically and horizontally
        Vec2 const text_size = CalcTextSize(text.bytes_begin(), text.bytes_end());
        Vec2 const text_pos{anton::math::max(frame_bb.Min.x, frame_bb.Min.x + (frame_bb.Max.x - frame_bb.Min.x - text_size.x) * 0.5f), 
                            anton::math::max(frame_bb.Min.y, frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y - text_size.y) * 0.5f)};
        render_text_clipped(text, text_pos, frame_bb, style.Colors[ImGuiCol_Text]);

        return value_changed;
    }

    bool drag_i64(anton::String_View const label, u32 const id, i64& value, i64 const step) {
        i64 const v_min = -9223372036854775808LL;
        i64 const v_max = 9223372036854775807LL;
        return drag_i64(label, id, value, step, v_min, v_max);
    }

    bool drag_f32(anton::String_View const label, u32 const id, f32& value, f32 const v_speed, f32 const v_min, f32 const v_max, char const* const format) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        ImGuiContext& ctx = *GImGui;
        ImGuiStyle const& style = GImGui->Style;
        u32 const id_hash = hash_label_with_id(label, id, window->IDStack.back());
        KeepAliveID(id_hash);
        f32 const w = CalcItemWidth();
        Vec2 const widget_pos = window->DC.CursorPos;
        Vec2 const label_pos = Vec2{widget_pos.x, widget_pos.y + style.FramePadding.y};
        Vec2 const label_size = CalcTextSize(label.bytes_begin(), label.bytes_end());
        // TODO: Is text clipping necessary here?
        render_text(label, label_pos, style.Colors[ImGuiCol_Text]);

        Vec2 const frame_pos = label_pos + Vec2{(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), -style.FramePadding.y};
        ImRect const frame_bb(frame_pos, frame_pos + Vec2(w, label_size.y + style.FramePadding.y * 2.0f));
        ImRect const total_bb(widget_pos, frame_bb.Max);
        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id_hash, &frame_bb)) {
            return false;
        }

        {
            // Double-click turns the box into an input field
            bool const hovered = ItemHoverable(frame_bb, id_hash);
            bool const temp_input_allowed = true; // (flags & ImGuiSliderFlags_NoInput) == 0;
            bool temp_input_is_active = temp_input_allowed && TempInputIsActive(id_hash);
            if(!temp_input_is_active) {
                bool const focus_requested = temp_input_allowed && FocusableItemRegister(window, id_hash);
                bool const clicked = (hovered && ctx.IO.MouseClicked[0]);
                bool const double_clicked = (hovered && ctx.IO.MouseDoubleClicked[0]);
                if (focus_requested || clicked || double_clicked) {
                    SetActiveID(id_hash, window);
                    SetFocusID(id_hash, window);
                    FocusWindow(window);
                    if (temp_input_allowed && (focus_requested || double_clicked)) {
                        temp_input_is_active = true;
                        FocusableItemUnregister(window);
                    }
                }
            }

            if(temp_input_is_active) {
                bool const is_clamp_input = v_min < v_max;
                // TempInputScalar displays an input field with the label on the right side. 
                // We work around that by supplying empty label.
                return TempInputScalar(frame_bb, id_hash, "", ImGuiDataType_Float, &value, format, is_clamp_input ? &v_min : NULL, is_clamp_input ? &v_max : NULL);
            }
        }
        

        // Draw frame
        bool const hovered = ctx.HoveredId == id_hash;
        bool const active = ctx.ActiveId == id_hash;
        Vec4 const frame_color = get_interactive_element_color(hovered, active, style.Colors[ImGuiCol_FrameBg], style.Colors[ImGuiCol_FrameBgHovered], style.Colors[ImGuiCol_FrameBgActive]);
        render_frame(frame_bb, frame_color);

        // Drag behavior
        bool const value_changed = DragBehavior(id_hash, ImGuiDataType_Float, &value, v_speed, &v_min, &v_max, format, ImGuiSliderFlags_None);
        if (value_changed) {
            MarkItemEdited(id_hash);
        }

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        char value_buf[64] = {};
        char const* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), ImGuiDataType_Float, &value, format);
        anton::String_View const text{value_buf, value_buf_end};
        // center text vertically and horizontally
        Vec2 const text_size = CalcTextSize(text.bytes_begin(), text.bytes_end());
        Vec2 const text_pos{anton::math::max(frame_bb.Min.x, frame_bb.Min.x + (frame_bb.Max.x - frame_bb.Min.x - text_size.x) * 0.5f), 
                            anton::math::max(frame_bb.Min.y, frame_bb.Min.y + (frame_bb.Max.y - frame_bb.Min.y - text_size.y) * 0.5f)};
        render_text_clipped(text, text_pos, frame_bb, style.Colors[ImGuiCol_Text]);

        return value_changed;
    }

    bool drag_f32_n(anton::String_View const label, u32 const id, f32* const value, i32 const components, f32 const v_speed, f32 const v_min, f32 const v_max, char const* const format) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }

        BeginGroup();
        u32 const id_hash = hash_label_with_id(label, id, window->IDStack.back());
        push_id(id_hash);
        ImGuiStyle const& style = GImGui->Style;
        Vec2 const widget_pos = window->DC.CursorPos;
        Vec2 const label_pos = Vec2{widget_pos.x, widget_pos.y + style.FramePadding.y};
        Vec2 const text_size = CalcTextSize(label.bytes_begin(), label.bytes_end());
        // We call ItemSize and ItemAdd manually to fake a widget
        ImRect const text_bb{label_pos, label_pos + text_size};
        ItemSize(text_bb, 0);
        if (!ItemAdd(text_bb, id_hash, &text_bb)) {
            pop_id();
            EndGroup();
            return false;
        }
        // TODO: Is text clipping necessary here?
        render_text(label, label_pos, style.Colors[ImGuiCol_Text]);

        bool value_changed = false;
        PushMultiItemsWidths(components, CalcItemWidth());
        for (i32 i = 0; i < components; i++) {
            SameLine(0, style.ItemInnerSpacing.x);
            value_changed |= drag_f32("", i, value[i], v_speed, v_min, v_max, format);
            PopItemWidth();
        }
        PopID();

        EndGroup();
        return value_changed;
    }
}
