#include <internal.hpp>

namespace ImGui {
    void render_frame(ImRect const rect, Vec4 const color) {
        ImGuiWindow* window = GetCurrentWindow();
        u32 const color_u32 = GetColorU32(color);
        window->DrawList->AddRectFilled(rect.Min, rect.Max, color_u32, 0.0f);
        // const float border_size = g.Style.FrameBorderSize;
        // if (border && border_size > 0.0f)
        // {
        //     window->DrawList->AddRect(p_min + ImVec2(1, 1), p_max + ImVec2(1, 1), GetColorU32(ImGuiCol_BorderShadow), rounding, ImDrawCornerFlags_All, border_size);
        //     window->DrawList->AddRect(p_min, p_max, GetColorU32(ImGuiCol_Border), rounding, ImDrawCornerFlags_All, border_size);
        // }
    }
}
