#include <internal.hpp>
#include <imgui/drag_drop.hpp>

#include <anton/array.hpp>
#include <anton/slice.hpp>
#include <anton/assert.hpp>
#include <anton/string_view.hpp>

namespace ImGui::drag_drop {
    using namespace anton::literals;

    struct Context {
        // payload data
        anton::Array<u8> data;
        // ID of the source item
        u64 source_id = 0;
        // ID of the payload
        u64 payload_id = 0;
        // ID of the target in the previous frame
        u64 previous_target_id = 0;
        // ID of the target in the current frame
        u64 current_target_id = 0;
        // whether drag drop is currently active
        bool active = false;
        // whether payload has been set via set_payload()
        bool payload_set = false;
        // whether we're inside the paired begin_source/end_source call
        bool in_source = false;
        // whether we're inside the paired begin_target/end_target call
        bool in_target = false;
    };

    static Context context;

    void end_frame() {
        context.previous_target_id = context.current_target_id;
        context.current_target_id = 0;

        bool const mouse_released = !IsMouseDown(ImGuiMouseButton_Left);
        if(context.active && mouse_released) {
            clear();
        }
    }

    bool is_active() {
        return context.active;
    }

    void clear() {
        context.data.clear();
        context.source_id = 0;
        context.payload_id = 0;
        context.previous_target_id = 0;
        context.current_target_id = 0;
        context.active = false;
        context.payload_set = false;
    }

    void set_payload(u64 const payload_id, anton::Slice<u8 const> const data) {
        ANTON_ASSERT(context.in_source, u8"set_payload() must be called between begin_source() and end_source()");
        context.payload_id = payload_id;
        context.data.assign(data.begin(), data.end());
        context.payload_set = true;
    }

    anton::Slice<u8> get_payload() {
        return context.data;
    }

    bool accept_payload(u64 const payload_id) {
        ANTON_ASSERT(context.in_target, u8"accept_payload() must be called between begin_target() and end_target()");
        if(payload_id != context.payload_id) {
            return false;
        }

        bool const mouse_released = !IsMouseDown(ImGuiMouseButton_Left);
        bool const delivered = context.current_target_id == context.previous_target_id && mouse_released;
        if(delivered) {
            context.active = false;
        }
        return delivered;
    }

    bool begin_source() {
        ANTON_ASSERT(context.in_source, "missing call to end_source()");
        ImGuiContext& ctx = *GImGui;
        ImGuiWindow* window = ctx.CurrentWindow;
        u64 const source_id = window->DC.LastItemId;
        bool const mouse_dragging = !IsMouseDragging(ImGuiMouseButton_Left);
        if(source_id == 0 || ctx.ActiveId != source_id || !mouse_dragging || context.active) {
            return false;
        }

        // Prevent other widgets from being interactive I suppose
        ctx.ActiveIdAllowOverlap = false;

        clear();
        context.active = true;
        context.source_id = source_id;
        context.in_source = true;
        return true;
    }

    void end_source() {
        ANTON_ASSERT(context.in_source, "end_source() called before begin_source()");
        // Discard the drag if payload has not been set
        if(!context.payload_set) {
            clear();
        }

        context.in_source = false;
    }

    bool begin_target() {
        ANTON_ASSERT(context.in_target, "missing call to end_target()");
        ImGuiContext& ctx = *GImGui;
        ImGuiWindow* window = ctx.CurrentWindow;
        bool const last_item_hovered = window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HoveredRect;
        u64 const target_id = window->DC.LastItemId;
        bool const target_is_source = target_id == context.source_id;
        // Target must not be 0, must not be source, must be hovered
        if(!context.active || target_id == 0 || target_is_source || !last_item_hovered) {
            return false;
        }

        // TODO: Figure out what this check is for
        // ImGuiWindow* hovered_window = g.HoveredWindowUnderMovingWindow;
        // if (hovered_window == NULL || window->RootWindow != hovered_window->RootWindow)
        //     return false;

        context.current_target_id = target_id;
        context.in_target = true;
        return true;
    }

    void end_target() {
        ANTON_ASSERT(context.in_target, "end_target() called before begin_target()");
        context.in_target = false;
    }
}
