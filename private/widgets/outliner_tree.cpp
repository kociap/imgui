#include <internal.hpp>

namespace ImGui {
    // imgui is single-threaded, so this is safe
    char temporary_input_buffer[1024];

    bool editable_text(anton::String& str, ImVec2 pos_before, bool pressed) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;

        strncpy(temporary_input_buffer, str.c_str(), IM_ARRAYSIZE(temporary_input_buffer));

        bool ret = pressed;

        ImGuiID id = window->GetID("##Input");
        bool temp_input_is_active = TempInputIsActive(id);
        bool temp_input_start = pressed ? IsMouseDoubleClicked(0) : false;

        if(temp_input_start) {
            SetActiveID(id, window);
        }

        if(temp_input_is_active || temp_input_start) {
            ImVec2 pos_after = window->DC.CursorPos;
            window->DC.CursorPos = pos_before;
            bool chgtext =
                TempInputText(window->DC.LastItemRect, id, "##Input", temporary_input_buffer, IM_ARRAYSIZE(temporary_input_buffer), ImGuiInputTextFlags_None);
            if(chgtext) {
                str.reserve(strlen(temporary_input_buffer));
                strcpy(str.data(), temporary_input_buffer);
                str.force_size(strlen(temporary_input_buffer));
            }
            ret |= chgtext;
            window->DC.CursorPos = pos_after;
        } else {
            window->DrawList->AddText(pos_before, GetColorU32(ImGuiCol_Text), temporary_input_buffer);
        }

        return ret;
    }

    bool selectable_input(anton::String& str, i32 imgui_id, bool selected, ImGuiSelectableFlags flags) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImVec2 pos_before = window->DC.CursorPos;

        PushID(imgui_id);
        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(g.Style.ItemSpacing.x, g.Style.FramePadding.y * 2.0f));
        bool ret = Selectable("##Selectable", selected, flags | ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_AllowItemOverlap);
        PopStyleVar();

        ret |= editable_text(str, pos_before, ret);
        PopID();
        return ret;
    }

    Outliner_Tree_Node_Style get_outliner_tree_node_style() {
        ImGuiStyle& imgui_style = GetStyle();
        Outliner_Tree_Node_Style style;
        style.text_color = imgui_style.Colors[ImGuiCol_Text];
        style.background = imgui_style.Colors[ImGuiCol_outliner_node_bg];
        style.background_hovered = imgui_style.Colors[ImGuiCol_outliner_node_bg_hovered];
        style.background_active = imgui_style.Colors[ImGuiCol_outliner_node_bg_active];
        return style;  
    }

    static void outliner_tree_push(u32 const id, f32 const indent) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        Indent(indent);
        window->DC.indent_stack.emplace_back(indent);
        window->DC.TreeDepth++;
        window->IDStack.push_back(id);
    }

    [[nodiscard]] static bool outliner_tree_node_is_open(u32 const id, Outliner_Tree_Node_Options const& options) {
        if(options.leaf) {
            return true;
        }

        // We only write to the tree storage if the user clicks (or explicitly use the SetNextItemOpen function)
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiStorage* storage = window->DC.StateStorage;

        bool is_open;
        if (g.NextItemData.Flags & ImGuiNextItemDataFlags_HasOpen) {
            if (g.NextItemData.OpenCond & ImGuiCond_Always) {
                is_open = g.NextItemData.OpenVal;
                storage->SetInt(id, is_open);
            }
            else {
                // We treat ImGuiCond_Once and ImGuiCond_FirstUseEver the same because tree node state are not saved persistently.
                const int stored_value = storage->GetInt(id, -1);
                if (stored_value == -1) {
                    is_open = g.NextItemData.OpenVal;
                    storage->SetInt(id, is_open);
                }
                else {
                    is_open = stored_value != 0;
                }
            }
        } else {
            is_open = storage->GetInt(id, options.open_by_default) != 0;
        }

        return is_open;
    }

    bool outliner_tree_node(u32 const id, anton::String& display_text, Outliner_Tree_Node_Options const& options, Outliner_Tree_Node_Style const& style) {
        ImGuiWindow* window = GetCurrentWindow();
        if(window->SkipItems) {
            return false;
        }

        imgui::push_id(id);
        u32 const id_hash = hash_id(0, window->IDStack.back());
        // No idea what this does
        KeepAliveID(id_hash);

        ImGuiContext& g = *GImGui;
        ImGuiStyle const& imgui_style = g.Style;
        Vec2 const padding = imgui_style.FramePadding;
        Vec2 const display_text_size = CalcTextSize(display_text.bytes_begin(), display_text.bytes_end(), false);

        // We vertically grow up to current line height up the typical widget height.
        f32 const frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + imgui_style.FramePadding.y * 2), display_text_size.y + padding.y * 2);
        ImRect frame_bb;
        // We want the node to span the entire width of the parent widget
        frame_bb.Min.x = window->WorkRect.Min.x;
        frame_bb.Max.x = window->WorkRect.Max.x;
        frame_bb.Min.y = window->DC.CursorPos.y;
        frame_bb.Max.y = window->DC.CursorPos.y + frame_height;

        // TODO: There once was code here that expanded frame_bb on x-axis by 0.5 * WindowPadding.x. Figure out why.
        // Original comment:
        // Framed header expand a little outside the default padding, to the edge of InnerClipRect
        // (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)

        // We want to indent each level by exactly the size of the arrow and its frame padding.
        f32 const indent = g.FontSize + padding.x * 2.0f;
        // Collapser arrow width + Spacing
        f32 const text_offset_x = g.FontSize + padding.x * 3.0f;
        // Latch before ItemSize changes it
        f32 const text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);
        // Include collapser
        f32 const text_width = g.FontSize + (display_text_size.x > 0.0f ? display_text_size.x + padding.x * 2 : 0.0f);
        Vec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
        ItemSize(Vec2(text_width, frame_height), padding.y);

        ImRect interact_bb = frame_bb;
        bool const is_leaf = options.leaf;
        bool const item_add = ItemAdd(interact_bb, id_hash);
        bool is_open = outliner_tree_node_is_open(id_hash, options);
        window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
        window->DC.LastItemDisplayRect = frame_bb;

        if(!item_add) {
            if(is_open) {
                outliner_tree_push(id_hash, indent);
            }
            imgui::pop_id();
            return is_open;
        }

        ImGuiButtonFlags button_flags = ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;

        // if (!is_leaf) {
        //     button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;
        // }

        // We allow clicking on the arrow section with keyboard modifiers held, in order to easily
        // allow browsing a tree while preserving selection with code implementing multi-selection patterns.
        // When clicking on the rest of the tree node we always disallow keyboard modifiers.
        f32 const arrow_hit_x1 = (text_pos.x - text_offset_x) - imgui_style.TouchExtraPadding.x;
        f32 const arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + imgui_style.TouchExtraPadding.x;
        bool const is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
        if (window != g.HoveredWindow || !is_mouse_x_over_arrow) {
            button_flags |= ImGuiButtonFlags_NoKeyModifiers;
        }

        bool hovered, held;
        bool pressed = ButtonBehavior(interact_bb, id_hash, &hovered, &held, button_flags);
        bool toggled = false;
        if (!is_leaf) {
            if (pressed && g.DragDropHoldJustPressedId != id_hash) {
                // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
                toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; 
            } else if (pressed && g.DragDropHoldJustPressedId == id_hash) {
                IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
                if (!is_open) {
                    // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
                    toggled = true;
                } 
            }

            if (g.NavId == id_hash && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Left && is_open) {
                toggled = true;
                NavMoveRequestCancel();
            }

            if (g.NavId == id_hash && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right && !is_open) {
                // If there's something upcoming on the line we may want to give it the priority? 
                toggled = true;
                NavMoveRequestCancel();
            }

            if (toggled) {
                is_open = !is_open;
                window->DC.StateStorage->SetInt(id_hash, is_open);
                window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
            }
        }

        SetItemAllowOverlap();

        // Render
        Vec4 const background_color = get_interactive_element_color(hovered, held, style.background, style.background_hovered, style.background_active);
        RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(background_color), false, 0.0f);
        RenderNavHighlight(frame_bb, id_hash, ImGuiNavHighlightFlags_TypeThin);
        if (!is_leaf) {
            RenderArrow(window->DrawList, Vec2(text_pos.x - text_offset_x + padding.x, text_pos.y), GetColorU32(style.text_color), is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
        }

        // if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton) {
        //     frame_bb.Max.x -= g.FontSize + imgui_style.FramePadding.x;
        // }

        if(options.editable_text) {
            editable_text(display_text, text_pos, pressed);
        } else {
            render_text_clipped(display_text, text_pos, frame_bb, style.text_color);
        }

        if(is_open) {
            outliner_tree_push(id_hash, indent);
        }

        imgui::pop_id();
        return is_open;
    }

    bool outliner_tree_node(u32 const id, anton::String& display_string, Outliner_Tree_Node_Options const& options) {
        ImGuiStyle& imgui_style = GetStyle();
        Outliner_Tree_Node_Style style;
        style.text_color = imgui_style.Colors[ImGuiCol_Text];
        style.background = imgui_style.Colors[ImGuiCol_outliner_node_bg];
        style.background_hovered = imgui_style.Colors[ImGuiCol_outliner_node_bg_hovered];
        style.background_active = imgui_style.Colors[ImGuiCol_outliner_node_bg_active];
        return outliner_tree_node(id, display_string, options, style);
    }

    void outliner_tree_pop() {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        Unindent(window->DC.indent_stack.back());
        window->DC.indent_stack.pop_back();
        window->DC.TreeDepth--;
        ImU32 tree_depth_mask = (1 << window->DC.TreeDepth);

        // Handle Left arrow to move to parent tree node (when ImGuiTreeNodeFlags_NavLeftJumpsBackHere is enabled)
        if (g.NavMoveDir == ImGuiDir_Left && g.NavWindow == window && NavMoveRequestButNoResultYet()) {
            if (g.NavIdIsAlive && (window->DC.TreeJumpToParentOnPopMask & tree_depth_mask))
            {
                SetNavID(window->IDStack.back(), g.NavLayer, 0);
                NavMoveRequestCancel();
            }
        }
        window->DC.TreeJumpToParentOnPopMask &= tree_depth_mask - 1;

        // There should always be 1 element in the IDStack (pushed during window creation). If this triggers you called TreePop/PopID too much.
        IM_ASSERT(window->IDStack.Size > 1); 
        PopID();
    }
}
