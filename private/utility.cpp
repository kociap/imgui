#include <internal.hpp>

namespace ImGui {
    u32 hash_id(u32 const id, u32 const seed) {
        u32 const id_hash = anton::murmurhash2_32((void*)&id, sizeof(u32), seed);
        return id_hash;
    }

    u32 hash_label_with_id(anton::String_View const label, u32 const id, u32 const seed) {
        u32 const label_hash = anton::murmurhash2_32(label.bytes_begin(), label.size_bytes(), seed);
        u32 const id_hash = anton::murmurhash2_32((void*)&id, sizeof(u32), seed);
        u32 const combined_hash = label_hash ^ (id_hash + 0x9e3779b9 + (label_hash << 6) + (label_hash >> 2));
        return combined_hash;
    }

    void push_id(u32 const id) {
        ImGuiWindow* window = GImGui->CurrentWindow;
        window->IDStack.push_back(id);
    }

    void push_id(anton::String_View const id) {
        u32 const hash = anton::murmurhash2_32(id.bytes_begin(), id.size_bytes());
        ImGuiWindow* window = GImGui->CurrentWindow;
        window->IDStack.push_back(hash);
    }

    void pop_id() {
        ImGuiWindow* window = GImGui->CurrentWindow;
        window->IDStack.pop_back();
    }

    Vec4 get_interactive_element_color(bool const hovered, bool const held, Vec4 const default_color, Vec4 const hovered_color, Vec4 const active_color) {
        if(hovered) {
            if(held) {
                return active_color;
            } else {
                return hovered_color;
            }
        } else {
            return default_color;
        }
    }

    u32 make_id(anton::String_View const string) {
        return anton::murmurhash2_32(string.data(), string.size_bytes());
    }
}
