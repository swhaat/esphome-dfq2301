#pragma once

#include <vector>
#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace df2301q {

// ── Protocol constants (shared between I²C and UART) ─────────────────────────

// I²C register map
static const uint8_t DF2301Q_I2C_ADDR           = 0x64;
static const uint8_t DF2301Q_REG_CMDID          = 0x02;
static const uint8_t DF2301Q_REG_PLAY_CMDID     = 0x03;
static const uint8_t DF2301Q_REG_MUTE           = 0x04;
static const uint8_t DF2301Q_REG_VOLUME         = 0x05;
static const uint8_t DF2301Q_REG_WAKE_TIME      = 0x06;

// UART frame delimiters / types
static const uint8_t DF2301Q_UART_HEAD_LOW      = 0xF4;
static const uint8_t DF2301Q_UART_HEAD_HIGH     = 0xF5;
static const uint16_t DF2301Q_UART_HEAD         = 0xF5F4;  // little-endian in struct
static const uint8_t DF2301Q_UART_TAIL          = 0xFB;
static const uint8_t DF2301Q_UART_MAX_DATA      = 8;       // max payload bytes

// UART message types (direction)
static const uint8_t DF2301Q_TYPE_CMD_UP        = 0xA0;  // device → host
static const uint8_t DF2301Q_TYPE_CMD_DOWN      = 0xA1;  // host → device

// UART command codes
static const uint8_t DF2301Q_CMD_ASR_RESULT     = 0x91;  // voice recognition result
static const uint8_t DF2301Q_CMD_PLAY_VOICE     = 0x92;  // play a reply audio
static const uint8_t DF2301Q_CMD_RESET_MODULE   = 0x95;  // full reset
static const uint8_t DF2301Q_CMD_SET_CONFIG     = 0x96;  // runtime settings

// UART play sub-commands (msgData[0] / msgData[1])
static const uint8_t DF2301Q_PLAY_START         = 0x80;
static const uint8_t DF2301Q_PLAY_BY_CMD_ID     = 0x92;

// UART config sub-commands (msgData[0])
static const uint8_t DF2301Q_CFG_VOLUME         = 0x80;
static const uint8_t DF2301Q_CFG_ENTER_WAKEUP   = 0x81;
static const uint8_t DF2301Q_CFG_MUTE           = 0x83;
static const uint8_t DF2301Q_CFG_WAKE_TIME      = 0x84;

// ── Trigger ───────────────────────────────────────────────────────────────────

/// Fired every time the module recognises a command word.
/// The automation receives the command word ID (uint8_t) as variable `x`.
class DF2301QTrigger : public Trigger<uint8_t> {
 public:
  void process(uint8_t cmd_id) { this->trigger(cmd_id); }
};

// ── Base mixin (shared API for both I²C and UART components) ──────────────────

/// Non-Component mixin that provides the trigger list and the shared action API.
/// Both DF2301QI2CComponent and DF2301QUARTComponent inherit from this.
class DF2301QBase {
 public:
  void register_trigger(DF2301QTrigger *t) { this->triggers_.push_back(t); }

  // Actions implemented by each sub-class
  virtual void play_command(uint8_t cmd_id) = 0;
  virtual void set_volume(uint8_t vol)      = 0;
  virtual void set_mute(bool mute)          = 0;
  virtual void set_wake_time(uint8_t secs)  = 0;
  virtual void reset_module() {}  // UART only; no-op on I²C

 protected:
  void fire_trigger_(uint8_t cmd_id);
  std::vector<DF2301QTrigger *> triggers_;
};

// ── I²C component ─────────────────────────────────────────────────────────────

/// Polling-based I²C implementation.
/// Reads register 0x02 every `update_interval` and fires the trigger when a
/// non-zero command ID is returned.
class DF2301QI2CComponent : public DF2301QBase,
                             public PollingComponent,
                             public i2c::I2CDevice {
 public:
  // PollingComponent
  void setup() override;
  void update() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

  // DF2301QBase actions
  void play_command(uint8_t cmd_id) override;
  void set_volume(uint8_t vol) override;
  void set_mute(bool mute) override;
  void set_wake_time(uint8_t secs) override;

 protected:
  /// Write one byte to an I²C register.
  void write_register_(uint8_t reg, uint8_t value);
  /// Read one byte from an I²C register.  Returns false on error.
  bool read_register_(uint8_t reg, uint8_t &value);
};

// ── UART component ────────────────────────────────────────────────────────────

/// Real-time UART implementation.
/// The module pushes ASR results autonomously; loop() parses the framed
/// protocol and fires the trigger for every valid CMD_UP / ASR_RESULT packet.
class DF2301QUARTComponent : public DF2301QBase,
                              public Component,
                              public uart::UARTDevice {
 public:
  // Component
  void setup() override;
  void loop() override;
  void dump_config() override;

  float get_setup_priority() const override { return setup_priority::DATA; }

  // DF2301QBase actions
  void play_command(uint8_t cmd_id) override;
  void set_volume(uint8_t vol) override;
  void set_mute(bool mute) override;
  void set_wake_time(uint8_t secs) override;
  void reset_module() override;

 protected:
  /// Raw UART frame (matches DFRobot protocol, packed).
  struct __attribute__((packed)) UartMsg {
    uint16_t header;                        // 0xF5F4
    uint16_t data_length;                   // payload byte count (type+cmd+seq not included)
    uint8_t  msg_type;
    uint8_t  msg_cmd;
    uint8_t  msg_seq;
    uint8_t  msg_data[DF2301Q_UART_MAX_DATA];
  };

  /// Receive-side state machine states.
  enum RecvState : uint8_t {
    HEAD0   = 0,
    HEAD1,
    LEN0,
    LEN1,
    TYPE,
    CMD,
    SEQ,
    DATA,
    CKSUM0,
    CKSUM1,
    TAIL,
  };

  void     send_packet_(const UartMsg &msg, uint8_t payload_len);
  void     send_setting_(uint8_t cfg_type, uint32_t value);

  // Receive state-machine fields (persistent across loop() calls)
  RecvState  recv_state_{HEAD0};
  uint16_t   recv_data_len_{0};
  uint16_t   recv_data_count_{0};
  uint16_t   recv_cksum_{0};
  uint8_t    recv_cksum_lo_{0};
  UartMsg    recv_msg_{};
  uint8_t    send_seq_{0};
};

// ── Action templates ──────────────────────────────────────────────────────────
// All actions use Parented<DF2301QBase> so they work with both I²C and UART.

template<typename... Ts>
class DF2301QPlayCommandAction : public Action<Ts...>,
                                  public Parented<DF2301QBase> {
 public:
  TEMPLATABLE_VALUE(uint8_t, cmd_id)
  void play(const Ts &...x) override {
    this->parent_->play_command(this->cmd_id_.value(x...));
  }
};

template<typename... Ts>
class DF2301QSetVolumeAction : public Action<Ts...>,
                                public Parented<DF2301QBase> {
 public:
  TEMPLATABLE_VALUE(uint8_t, volume)
  void play(const Ts &...x) override {
    this->parent_->set_volume(this->volume_.value(x...));
  }
};

template<typename... Ts>
class DF2301QSetMuteAction : public Action<Ts...>,
                              public Parented<DF2301QBase> {
 public:
  TEMPLATABLE_VALUE(bool, mute)
  void play(const Ts &...x) override {
    this->parent_->set_mute(this->mute_.value(x...));
  }
};

template<typename... Ts>
class DF2301QSetWakeTimeAction : public Action<Ts...>,
                                  public Parented<DF2301QBase> {
 public:
  TEMPLATABLE_VALUE(uint8_t, wake_time)
  void play(const Ts &...x) override {
    this->parent_->set_wake_time(this->wake_time_.value(x...));
  }
};

template<typename... Ts>
class DF2301QResetAction : public Action<Ts...>,
                            public Parented<DF2301QBase> {
 public:
  void play(const Ts &...x) override {
    this->parent_->reset_module();
  }
};

}  // namespace df2301q
}  // namespace esphome
