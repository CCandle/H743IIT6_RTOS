#include "Tasks/Screen/MainScreen.hpp"
#include <cstdio>

namespace UI {

namespace {
QueueHandle_t g_ctrl_queue = nullptr;
SnapshotStore* g_snap_store = nullptr;

lv_obj_t* label_state = nullptr;
lv_obj_t* label_values = nullptr;
lv_obj_t* btn_run = nullptr;
lv_obj_t* btn_reset = nullptr;
lv_obj_t* label_diag = nullptr;

bool running = false;

void sendCmd(IPC::Control::CommandType type) {
  if (!g_ctrl_queue) return;
  IPC::Control::Command cmd{type, 0.0f};
  xQueueSend(g_ctrl_queue, &cmd, 0);
}

void update_labels() {
  if (!g_snap_store) return;
  Snapshot snap;
  if (!g_snap_store->read(snap, 0)) return;

  // 状态显示
  char state_buf[64];
  const char* run_str = running ? "RUN" : "STOP";
  if (snap.last_fault_flag) {
    std::snprintf(state_buf, sizeof(state_buf), "STATE: FAULT 0x%08lX", static_cast<unsigned long>(snap.last_fault_code));
  } else {
    std::snprintf(state_buf, sizeof(state_buf), "STATE: %s", run_str);
  }
  lv_label_set_text(label_state, state_buf);

  if (label_diag) {
    if (snap.last_fault_flag) {
      char diag[64];
      std::snprintf(diag, sizeof(diag), "FAULT: 0x%08lX", static_cast<unsigned long>(snap.last_fault_code));
      lv_label_set_text(label_diag, diag);
    } else {
      lv_label_set_text(label_diag, "FAULT: NONE");
    }
  }

  // 数值显示
  char val_buf[160];
  std::snprintf(val_buf, sizeof(val_buf),
                "Vbus: %.1f V\nVcap_up: %.1f V\nVcap_dn: %.1f V\nIbus: %.2f A\nDuty_up: %.3f\nDuty_dn: %.3f",
                snap.v_bus.avg, snap.v_cap_up.avg, snap.v_cap_dn.avg, snap.i_bus.avg, snap.duty_up.avg, snap.duty_dn.avg);
  lv_label_set_text(label_values, val_buf);
}

void on_run_btn(lv_event_t* e) {
  (void)e;
  running = !running;
  sendCmd(running ? IPC::Control::CommandType::Start : IPC::Control::CommandType::Stop);
}

void on_reset_btn(lv_event_t* e) {
  (void)e;
  running = false;
  sendCmd(IPC::Control::CommandType::ResetFault);
}

void timer_cb(lv_timer_t* t) {
  (void)t;
  update_labels();
}
} // namespace

void CreateMainScreen(QueueHandle_t control_queue, SnapshotStore* snapshot_store) {
  g_ctrl_queue = control_queue;
  g_snap_store = snapshot_store;

  lv_obj_t* scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_make(0x10, 0x18, 0x30), 0);

  label_state = lv_label_create(scr);
  lv_obj_set_style_text_color(label_state, lv_color_white(), 0);
  lv_obj_align(label_state, LV_ALIGN_TOP_MID, 0, 16);
  lv_label_set_text(label_state, "STATE: INIT");

  label_values = lv_label_create(scr);
  lv_obj_set_style_text_color(label_values, lv_color_white(), 0);
  lv_label_set_text(label_values, "Vbus: --\nVcap_up: --\nVcap_dn: --\nIbus: --\nDuty_up: --\nDuty_dn: --");
  lv_obj_align(label_values, LV_ALIGN_TOP_LEFT, 16, 60);

  label_diag = lv_label_create(scr);
  lv_obj_set_style_text_color(label_diag, lv_color_hex(0xFF9900), 0);
  lv_obj_align(label_diag, LV_ALIGN_TOP_RIGHT, -16, 60);
  lv_label_set_text(label_diag, "FAULT: NONE");

  btn_run = lv_button_create(scr);
  lv_obj_set_size(btn_run, 120, 50);
  lv_obj_align(btn_run, LV_ALIGN_BOTTOM_LEFT, 40, -40);
  lv_obj_add_event_cb(btn_run, on_run_btn, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lbl_run = lv_label_create(btn_run);
  lv_label_set_text(lbl_run, "RUN/STOP");
  lv_obj_center(lbl_run);

  btn_reset = lv_button_create(scr);
  lv_obj_set_size(btn_reset, 120, 50);
  lv_obj_align(btn_reset, LV_ALIGN_BOTTOM_RIGHT, -40, -40);
  lv_obj_add_event_cb(btn_reset, on_reset_btn, LV_EVENT_CLICKED, NULL);
  lv_obj_t* lbl_reset = lv_label_create(btn_reset);
  lv_label_set_text(lbl_reset, "RESET");
  lv_obj_center(lbl_reset);

  lv_disp_load_scr(scr);
  // 定时刷新 UI
  lv_timer_create(timer_cb, 200, NULL);
}

} // namespace UI
