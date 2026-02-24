#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/hal.h"
#include <vector>

namespace esphome {
namespace clrc663 {

class CLRC663BinarySensor;

enum CLRC663Error {
  NONE = 0,
  WAKEUP_FAILED,
  SAM_COMMAND_FAILED,
};

class CLRC663 : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void register_tag(CLRC663BinarySensor *tag) { this->binary_sensors_.push_back(tag); }

  void add_on_tag_callback(std::function<void(std::string)> callback) {
    this->on_tag_callbacks_.add(std::move(callback));
  }
  void add_on_tag_removed_callback(std::function<void(std::string)> callback) {
    this->on_tag_removed_callbacks_.add(std::move(callback));
  }
  
  void set_health_check_enabled(bool enabled) { this->health_check_enabled_ = enabled; }
  void set_health_check_interval(uint32_t interval_ms) { this->health_check_interval_ = interval_ms; }
  void set_max_failed_checks(uint8_t max_fails) { this->max_failed_checks_ = max_fails; }
  void set_auto_reset_on_failure(bool enabled) { this->auto_reset_on_failure_ = enabled; }

 protected:
  /// Read data from CLRC663 via the interface (I2C or SPI)
  virtual bool read_data(std::vector<uint8_t> &data, uint8_t len) = 0;
  
  /// Write data to CLRC663 via the interface (I2C or SPI)
  virtual bool write_data(const std::vector<uint8_t> &data) = 0;
  
  /// Reset the CLRC663 (implementation depends on interface)
  virtual bool reset_() = 0;

  bool is_ready_();
  bool read_passive_target_id_(std::vector<uint8_t> &uid);
  bool get_firmware_version_(std::vector<uint8_t> &response);
  
  // Health check methods
  void perform_health_check_();
  bool verify_communication_();
  void handle_health_failure_();

  std::vector<uint8_t> current_uid_;
  std::vector<CLRC663BinarySensor *> binary_sensors_;
  CallbackManager<void(std::string)> on_tag_callbacks_;
  CallbackManager<void(std::string)> on_tag_removed_callbacks_;
  enum CLRC663Error error_code_{NONE};
  
  // Health check state
  bool health_check_enabled_{true};
  uint32_t health_check_interval_{60000};  // 60 seconds default
  uint32_t last_health_check_{0};
  uint8_t failed_health_checks_{0};
  uint8_t max_failed_checks_{3};
  bool auto_reset_on_failure_{true};
  bool component_healthy_{true};
  
  // Backoff for communication errors
  uint32_t next_read_after_{0};
  uint32_t read_backoff_{0};
};

class CLRC663Trigger : public Trigger<std::string> {
 public:
  void process(std::string uid) { this->trigger(std::move(uid)); }
};

template<typename... Ts> class CLRC663IsWritingCondition : public Condition<Ts...>, public Parented<CLRC663> {
 public:
  bool check(Ts... x) override { return false; }  // Placeholder for write operations
};

}  // namespace clrc663
}  // namespace esphome
