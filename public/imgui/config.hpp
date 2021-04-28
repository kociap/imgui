#pragma once

#include <anton/types.hpp>
#include <anton/math/vec2.hpp>
#include <anton/math/vec4.hpp>

namespace ImGui {
    using i8  = anton::i8;
    using i16 = anton::i16;
    using i32 = anton::i32;
    using i64 = anton::i64;
    using u8  = anton::u8;
    using u16 = anton::u16;
    using u32 = anton::u32;
    using u64 = anton::u64;
    using f32 = anton::f32;
    using f64 = anton::f64;

    using char8 = anton::char8;
    using char16 = anton::char16;
    using char32 = anton::char32;

    using Rect_f32 = anton::Rect_f32;

    using Vec2 = anton::math::Vec2;
    using Vec4 = anton::math::Vec4;
}

namespace imgui = ImGui;
