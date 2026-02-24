#include "binary_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace clrc663 {

static const char *const TAG = "clrc663.binary_sensor";

bool CLRC663BinarySensor::process(const std::vector<uint8_t> &data) {
  if (data.size() != this->uid_.size()) {
    return false;
  }

  for (size_t i = 0; i < data.size(); i++) {
    if (data[i] != this->uid_[i]) {
      return false;
    }
  }

  // UID matches
  if (!this->found_) {
    this->found_ = true;
    this->publish_state(true);
  }
  return true;
}

}  // namespace clrc663
}  // namespace esphome
