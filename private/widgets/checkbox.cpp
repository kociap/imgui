#include <internal.hpp>

namespace ImGui {
    bool checkbox(anton::String_View const label, u32 const id, bool& v) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        u32 const id_hash = hash_label_with_id(label, id, window->IDStack.back());
        // No idea what this does
        KeepAliveID(id_hash);

        ImGuiStyle const& style = GImGui->Style;
        f32 const square_sz = GetFrameHeight();
        Vec2 const widget_pos = window->DC.CursorPos;
        Vec2 const label_size = CalcTextSize(label.bytes_begin(), label.bytes_end());
        // We only want to add spacing after the label if the label is a non-empty string
        f32 const label_spacing = (label.size_bytes() > 0 ? style.ItemInnerSpacing.x : 0.0f);

        ImRect const total_bb(widget_pos, widget_pos + label_size + Vec2(square_sz + label_spacing, style.FramePadding.y * 2.0f));
        ItemSize(total_bb, style.FramePadding.y);
        if(!ItemAdd(total_bb, id_hash)) {
            return false;
        }

        Vec2 const text_pos = widget_pos + Vec2{0.0f, style.FramePadding.y};
        render_text(label, text_pos, style.Colors[ImGuiCol_Text]);

        Vec2 const frame_pos = {widget_pos.x + label_size.x + label_spacing, widget_pos.y};
        ImRect const check_bb(frame_pos, frame_pos + ImVec2(square_sz, square_sz));

        bool hovered, held;
        bool const pressed = ButtonBehavior(check_bb, id_hash, &hovered, &held);
        if(pressed) {
            v = !v;
            MarkItemEdited(id_hash);
        }

        RenderNavHighlight(total_bb, id_hash);
        u32 const frame_color = GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderFrame(check_bb.Min, check_bb.Max, frame_color, true, style.FrameRounding);
        u32 const check_col = GetColorU32(ImGuiCol_CheckMark);
        if(window->DC.ItemFlags & ImGuiItemFlags_MixedValue) {
            // Undocumented tristate/mixed/indeterminate checkbox (#2644)
            ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 3.6f)));
            window->DrawList->AddRectFilled(check_bb.Min + pad, check_bb.Max - pad, check_col, style.FrameRounding);
        } else if(v) {
            f32 const pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
            RenderCheckMark(window->DrawList, check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad * 2.0f);
        }

        return pressed;
    }
}
