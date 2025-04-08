#pragma once
#include "esphome.h"
#include "esphome/components/media_player/media_player.h"
#include "esphome/components/speaker/speaker.h"

namespace esphome {
namespace hostuac {

class HostUACComponent : public Component,
                       public media_player::MediaPlayer,
                       public Parented<speaker::Speaker> {
 public:
  void setup() override;
  void loop() override;
  
  void set_output_mode(uint8_t mode) { output_mode_ = mode; }
  void set_priority(uint8_t priority) { priority_ = priority; }

  // MediaPlayer interface
  void control(const media_player::MediaPlayerCall &call) override;
  media_player::MediaPlayerTraits get_traits() override;

 protected:
  uint8_t output_mode_;
  uint8_t priority_;
  bool usb_connected_{false};
};

}  // namespace hostuac
}  // namespace esphome
