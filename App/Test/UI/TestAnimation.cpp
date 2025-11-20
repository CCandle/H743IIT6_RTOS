#include "TestAnimation.hpp"
#include "lvgl/lvgl.h"

/**
 * @brief 更丰富的测试动画：渐变背景 + 双点弹跳，便于观察刷新。
 */
void CreateTestAnimation() {
  lv_obj_t* screen = lv_screen_active();
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x0f172a), 0);
  lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x1f3b70), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

  lv_display_t* disp = lv_display_get_default();
  const int16_t screen_w =
      disp ? static_cast<int16_t>(lv_display_get_horizontal_resolution(disp)) : 800;
  const int16_t screen_h =
      disp ? static_cast<int16_t>(lv_display_get_vertical_resolution(disp)) : 480;

  lv_obj_t* card = lv_obj_create(screen);
  lv_obj_set_size(card, 280, 170);
  lv_obj_center(card);
  lv_obj_set_style_bg_color(card, lv_color_hex(0x111827), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_80, 0);
  lv_obj_set_style_radius(card, 18, 0);
  lv_obj_set_style_shadow_width(card, 22, 0);
  lv_obj_set_style_shadow_spread(card, 2, 0);
  lv_obj_set_style_shadow_color(card, lv_color_hex(0x0ea5e9), 0);

  lv_obj_t* title = lv_label_create(card);
  lv_label_set_text(title, "LVGL Preview");
  lv_obj_set_style_text_color(title, lv_color_hex(0x9db5d4), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

  lv_obj_t* subtitle = lv_label_create(card);
  lv_label_set_text(subtitle, "DMA Flush in Progress");
  lv_obj_set_style_text_color(subtitle, lv_color_hex(0x6ee7ff), 0);
  lv_obj_align(subtitle, LV_ALIGN_BOTTOM_MID, 0, -12);

  auto create_dot = [](lv_obj_t* parent, lv_color_t color) {
    lv_obj_t* dot = lv_obj_create(parent);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 22, 22);
    lv_obj_set_style_bg_color(dot, color, 0);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(dot, 12, 0);
    lv_obj_set_style_shadow_color(dot, color, 0);
    return dot;
  };

  lv_obj_t* dot_a = create_dot(screen, lv_color_hex(0x38bdf8));
  lv_obj_t* dot_b = create_dot(screen, lv_color_hex(0xa855f7));

  static const lv_color_t palette[] = {
      lv_color_hex(0x22d3ee),
      lv_color_hex(0x0ea5e9),
      lv_color_hex(0x38bdf8),
      lv_color_hex(0xa855f7),
      lv_color_hex(0xf97316),
  };

  struct AnimCtx {
    lv_obj_t* dot_a;
    lv_obj_t* dot_b;
    int16_t x_a;
    int16_t y_a;
    int16_t x_b;
    int16_t y_b;
    int8_t vx_a;
    int8_t vy_a;
    int8_t vx_b;
    int8_t vy_b;
    uint8_t palette_idx;
    const lv_color_t* palette;
    size_t palette_len;
    int16_t max_x;
    int16_t max_y;
  };

  static AnimCtx ctx{
      dot_a,
      dot_b,
      40,
      40,
      200,
      120,
      4,
      3,
      -3,
      4,
      0,
      palette,
      sizeof(palette) / sizeof(palette[0]),
      static_cast<int16_t>(screen_w - 22),
      static_cast<int16_t>(screen_h - 22),
  };

  lv_timer_create(
      [](lv_timer_t* timer) {
        auto* state = static_cast<AnimCtx*>(lv_timer_get_user_data(timer));
        const int16_t min_x = 0;
        const int16_t min_y = 0;
        const int16_t max_x = state->max_x;
        const int16_t max_y = state->max_y;

        state->x_a += state->vx_a;
        state->y_a += state->vy_a;
        state->x_b += state->vx_b;
        state->y_b += state->vy_b;

        auto bounce = [](int16_t& pos, int8_t& vel, int16_t min_v, int16_t max_v) {
          if (pos <= min_v || pos >= max_v) {
            vel = -vel;
            pos = pos < min_v ? min_v : (pos > max_v ? max_v : pos);
            return true;
          }
          return false;
        };

        bool bounce_a = bounce(state->x_a, state->vx_a, min_x, max_x) | bounce(state->y_a, state->vy_a, min_y, max_y);
        bool bounce_b = bounce(state->x_b, state->vx_b, min_x, max_x) | bounce(state->y_b, state->vy_b, min_y, max_y);

        if (bounce_a || bounce_b) {
          state->palette_idx = static_cast<uint8_t>((state->palette_idx + 1) % state->palette_len);
          lv_color_t color_a = state->palette[state->palette_idx];
          lv_color_t color_b = state->palette[(state->palette_idx + 2) % state->palette_len];
          lv_obj_set_style_bg_color(state->dot_a, color_a, 0);
          lv_obj_set_style_shadow_color(state->dot_a, color_a, 0);
          lv_obj_set_style_bg_color(state->dot_b, color_b, 0);
          lv_obj_set_style_shadow_color(state->dot_b, color_b, 0);
        }

        lv_obj_set_pos(state->dot_a, state->x_a, state->y_a);
        lv_obj_set_pos(state->dot_b, state->x_b, state->y_b);
      },
      16, &ctx);
}