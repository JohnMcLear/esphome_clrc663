#pragma once

#include "clrc663.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace clrc663 {

class CLRC663I2C : public CLRC663, public i2c::I2CDevice {
 public:
  void dump_config() override;
  
  void set_irq_pin(GPIOPin *pin) { this->irq_pin_ = pin; }
  void set_reset_pin(GPIOPin *pin) { this->reset_pin_ = pin; }

 protected:
  bool read_data(std::vector<uint8_t> &data, uint8_t len) override;
  bool write_data(const std::vector<uint8_t> &data) override;
  bool reset_() override;

  GPIOPin *irq_pin_{nullptr};
  GPIOPin *reset_pin_{nullptr};
};

}  // namespace clrc663
}  // namespace esphome
