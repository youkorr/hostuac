#include "hostuac.h"
#include "esphome/core/log.h"
#include "usb/usb_host.h"

namespace esphome {
namespace hostuac {

static const char *const TAG = "hostuac";

void HostUACComponent::setup() {
  ESP_LOGI(TAG, "Initializing USB Host");

  const usb_host_config_t host_config = {
    .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  ESP_ERROR_CHECK(usb_host_install(&host_config));

  const usb_host_client_config_t client_config = {
    .is_synchronous = false,
    .max_num_event_msg = 5,
    .async = {
      .client_event_callback = usb_event_callback,
      .callback_arg = this
    }
  };
  ESP_ERROR_CHECK(usb_host_client_register(&client_config, &client_hdl_));
}

void HostUACComponent::loop() {
  // Gestion périodique des événements USB
  usb_host_client_handle_events(client_hdl_, 0);
}

void HostUACComponent::usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg) {
  auto *this_ = static_cast<HostUACComponent*>(arg);
  
  switch (event_msg->event) {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
      ESP_LOGI(TAG, "New USB device connected");
      this_->device_connected_ = true;
      break;
      
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
      ESP_LOGI(TAG, "USB device disconnected");
      this_->device_connected_ = false;
      break;
  }
}

}  // namespace hostuac
}  // namespace esphome
