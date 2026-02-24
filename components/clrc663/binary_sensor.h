#pragma once

#include "clrc663.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace clrc663 {

class CLRC663BinarySensor : public binary_sensor::BinarySensor {
 public:
  void set_uid(const std::vector<uint8_t> &uid) { this->uid_ = uid; }

  bool process(const std::vector<uint8_t> &data);

  void on_scan_end() {
    if (this->publish_state(false)) {
      this->found_ = false;
    }
  }

 protected:
  std::vector<uint8_t> uid_;
  bool found_{false};
};

}  // namespace clrc663
}  // namespace esphome
