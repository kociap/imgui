#include <internal.hpp>

#include <anton/math/math.hpp>

namespace ImGui {
    using namespace anton::literals;

    [[nodiscard]] Vec2 calculate_text_size(anton::String_View const text) {
        return CalcTextSize(text.bytes_begin(), text.bytes_end(), false, 0.0f);
    }

    void render_text_clipped(anton::String_View const text, Vec2 const text_pos, ImRect const clip_rect_in, Vec4 const text_color) {
        if(text.size_bytes() == 0) {
            return;
        }

        Rect_f32 const clip_rect{clip_rect_in.Min.x, clip_rect_in.Min.y, clip_rect_in.Max.x, clip_rect_in.Max.y};

        Vec2 const text_size = calculate_text_size(text);
        ImGuiWindow* const window = GImGui->CurrentWindow;
        bool const needs_clipping = (text_pos.x < clip_rect.left) || (text_pos.x + text_size.x >= clip_rect.right) || (text_pos.y < clip_rect.top) ||
                                    (text_pos.y + text_size.y >= clip_rect.bottom);
        if(needs_clipping) {
            Vec4 const fine_clip_rect(clip_rect.left, clip_rect.top, clip_rect.right, clip_rect.bottom);
            window->DrawList->AddText(NULL, 0.0f, text_pos, GetColorU32(text_color), text.bytes_begin(), text.bytes_end(), 0.0f, &fine_clip_rect);
        } else {
            window->DrawList->AddText(NULL, 0.0f, text_pos, GetColorU32(text_color), text.bytes_begin(), text.bytes_end(), 0.0f, NULL);
        }
    }

    void render_text(anton::String_View const text, Vec2 const text_pos, Vec4 const text_color) {
        if(text.size_bytes() == 0) {
            return;
        }

        ImGuiWindow* const window = GImGui->CurrentWindow;
        window->DrawList->AddText(NULL, 0.0f, text_pos, GetColorU32(text_color), text.bytes_begin(), text.bytes_end(), 0.0f, NULL);
    }

    void text_ellipsis(anton::String_View const text, f32 const max_width, Vec4 const color) {
        ImGuiWindow* const window = GetCurrentWindow();
        if(window->SkipItems) {
            return;
        }

        if(text.size_bytes() == 0) {
            return;
        }

        Vec2 const text_pos = window->DC.CursorPos;
        Vec2 const text_size = calculate_text_size(text);
        // If the text size is less than max_width, we can render the entire text
        // without any additional work since the ellipsis must not be added.
        if(text_size.x <= max_width) {
            ImRect bb(text_pos, text_pos + text_size);
            ItemSize(text_size, 0.0f);
            if(!ItemAdd(bb, 0)) {
                return;
            }

            u32 const color_u32 = GetColorU32(color);
            window->DrawList->AddText(NULL, 0.0f, text_pos, color_u32, text.bytes_begin(), text.bytes_end(), 0.0f, NULL);
            return;
        }

        // The text is wider than the available space, so we have to trim it and add the ellipsis.
        // We will go through the text and find a substring that will fit in the given space
        // after adding the ellipsis.
        ImFont const* const font = GImGui->Font;
        Font_Glyph const* dot_glyph = font->find_glyph(U'.');
        f32 const dot_left_bearing = dot_glyph->X0;
        f32 const dot_width = dot_glyph->X1 - dot_glyph->X0;
        // TODO: Might want the spacing to scale with the font size.
        f32 const dot_spacing = 1.0f;
        f32 const ellipsis_advance = 3.0f * dot_width + 3.0f * dot_spacing;
        f32 width = 0.0f;
        auto current = text.chars_begin();
        auto const end = text.chars_end();
        f32 previous_glyph_right_bearing = 0.0f;
        // We always want to render at least one character
        {
            char32 const c = *current;
            Font_Glyph const* glyph = font->find_glyph(c);
            width += glyph->X1;
            previous_glyph_right_bearing = glyph->AdvanceX - glyph->X1;
            ++current;
        }

        while(current != end) {
            char32 const c = *current;
            Font_Glyph const* glyph = font->find_glyph(c);
            f32 const width_delta = previous_glyph_right_bearing + glyph->X1;
            f32 const text_width_with_ellipsis = width + width_delta + ellipsis_advance;
            if(text_width_with_ellipsis <= max_width) {
                width += width_delta;
                previous_glyph_right_bearing = glyph->AdvanceX - glyph->X1;
                ++current;
            } else {
                break;
            }
        }

        ImRect bb(text_pos, text_pos + Vec2{width + ellipsis_advance, text_size.y});
        ItemSize(Vec2{width + ellipsis_advance, text_size.y}, 0.0f);
        if(!ItemAdd(bb, 0)) {
            return;
        }

        char8 const* const text_begin = text.bytes_begin();
        char8 const* const text_end = current.get_underlying_pointer();
        u32 const color_u32 = GetColorU32(color);
        // Render the text
        window->DrawList->AddText(nullptr, 0.0f, text_pos, color_u32, text_begin, text_end, 0.0f, NULL);
        // Render the ellipsis. We render each dot separately to minimize the spacing between the dots.
        // We have to subtract dot's left bearing to compensate for the left bearing added by RenderChar.
        Vec2 dot_position = text_pos + Vec2{width - dot_left_bearing, 0.0f};
        for(i64 i = 0; i < 3; ++i) {
            dot_position.x += dot_spacing;
            font->RenderChar(window->DrawList, font_size, dot_position, color_u32, U'.');
            dot_position.x += dot_width;
        }
    }

    void text_ellipsis(anton::String_View const text, f32 const max_width) {
        Vec4 const color = GImGui->Style.Colors[ImGuiCol_Text];
        text_ellipsis(text, max_width, color);
    }
} // namespace ImGui
