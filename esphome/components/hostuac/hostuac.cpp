#include "hostuac.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/components/speaker/speaker.h"

#ifdef USE_ESP32
#include "usb/usb_host.h"
#include "usb/uac_host.h"

namespace esphome {
namespace hostuac {

static const char *const TAG = "hostuac";

// Variables globales pour la gestion USB
static QueueHandle_t usb_event_queue = nullptr;
static uac_host_device_handle_t uac_device = nullptr;
static bool device_connected = false;

void HostUACComponent::setup() {
  ESP_LOGI(TAG, "Initialisation du composant HostUAC");

  // Création de la file d'événements
  usb_event_queue = xQueueCreate(10, sizeof(uac_host_event_t));
  if (!usb_event_queue) {
    ESP_LOGE(TAG, "Échec de création de la file d'événements USB");
    return;
  }

  // Configuration du host USB
  usb_host_config_t host_config = {
    .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  ESP_ERROR_CHECK(usb_host_install(&host_config));

  // Installation du driver UAC
  uac_host_driver_config_t uac_config = {
    .create_background_task = true,
    .task_priority = 5,
    .callback = usb_event_handler,
    .callback_arg = this
  };
  ESP_ERROR_CHECK(uac_host_install(&uac_config));

  ESP_LOGI(TAG, "HostUAC initialisé avec succès");
}

void HostUACComponent::loop() {
  uac_host_event_t event;
  while (xQueueReceive(usb_event_queue, &event, 0)) {
    switch (event.type) {
      case UAC_HOST_DEVICE_CONNECTED:
        ESP_LOGI(TAG, "Périphérique UAC connecté");
        device_connected = true;
        uac_device = event.device_handle;
        this->state = media_player::MEDIA_PLAYER_STATE_IDLE;
        break;

      case UAC_HOST_DEVICE_DISCONNECTED:
        ESP_LOGI(TAG, "Périphérique UAC déconnecté");
        device_connected = false;
        uac_device = nullptr;
        this->state = media_player::MEDIA_PLAYER_STATE_IDLE;
        break;

      case UAC_HOST_AUDIO_DATA:
        if (this->speaker_ && device_connected) {
          this->speaker_->play(event.data, event.data_size);
        }
        break;
    }
    this->publish_state();
  }
}

// Implémentation des commandes MediaPlayer
void HostUACComponent::control(const media_player::MediaPlayerCall &call) {
  if (call.get_volume().has_value()) {
    float volume = *call.get_volume();
    volume = std::clamp(volume, 0.0f, 1.0f);
    if (uac_device) {
      uac_host_device_set_volume(uac_device, static_cast<uint8_t>(volume * 100));
    }
    this->volume = volume;
  }

  if (call.get_command().has_value()) {
    switch (*call.get_command()) {
      case media_player::MEDIA_PLAYER_COMMAND_PLAY:
        if (uac_device) uac_host_device_resume(uac_device);
        this->state = media_player::MEDIA_PLAYER_STATE_PLAYING;
        break;
        
      case media_player::MEDIA_PLAYER_COMMAND_PAUSE:
        if (uac_device) uac_host_device_suspend(uac_device);
        this->state = media_player::MEDIA_PLAYER_STATE_PAUSED;
        break;
        
      case media_player::MEDIA_PLAYER_COMMAND_STOP:
        if (uac_device) uac_host_device_suspend(uac_device);
        this->state = media_player::MEDIA_PLAYER_STATE_IDLE;
        break;
    }
  }
  this->publish_state();
}

media_player::MediaPlayerTraits HostUACComponent::get_traits() {
  auto traits = media_player::MediaPlayerTraits();
  traits.set_supports_pause(true);
  traits.set_supports_stop(true);
  traits.set_supports_volume(true);
  return traits;
}

// Gestionnaire d'événements USB
void IRAM_ATTR HostUACComponent::usb_event_handler(void* arg, uac_host_event_t event) {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(usb_event_queue, &event, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

}  // namespace hostuac
}  // namespace esphome

#endif  // USE_ESP32
