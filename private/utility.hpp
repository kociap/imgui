#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui {
    [[nodiscard]] u32 hash_id(u32 id, u32 seed);
    [[nodiscard]] u32 hash_label_with_id(anton::String_View label, u32 id, u32 seed);

    void push_id(u32 id);
    void push_id(anton::String_View id);
    void pop_id();

    [[nodiscard]] Vec4 get_interactive_element_color(bool hovered, bool held, Vec4 default_color, Vec4 hovered_color, Vec4 active_color);
}
