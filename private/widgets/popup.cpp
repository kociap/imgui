#include <internal.hpp>

namespace ImGui {
    using namespace anton::literals;

    bool begin_window_context_menu() {
        u32 const id = make_id("window_context"_sv);
        return begin_window_context_menu(id);
    }

    bool begin_window_context_menu(u32 const id) {
        ImGuiWindow* const window = GImGui->CurrentWindow;
        u32 const id_hash = hash_id(id, window->IDStack.back());
        // No idea what this does
        KeepAliveID(id_hash);

        int mouse_button = ImGuiMouseButton_Right;
        if(IsMouseReleased(mouse_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            OpenPopupEx(id_hash);
        }

        return BeginPopupEx(id_hash, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings);
    }
}
