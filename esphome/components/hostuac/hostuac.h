#pragma once
#include "esphome.h"
#include "esphome/components/media_player/media_player.h"
#include "driver/usb_host.h"  // Cheval d'entrée principal USB

namespace esphome {
namespace hostuac {

class HostUACComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  
  // Callback USB
  static void usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg);
  
 private:
  usb_host_client_handle_t client_hdl_;
  bool device_connected_{false};
};
}  // namespace hostuac
}  // namespace esphome
