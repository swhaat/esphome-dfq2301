# ESPHome – DF2301Q Voice Recognition Component

An [ESPHome](https://esphome.io) external component for the
[DFRobot DF2301Q](https://www.dfrobot.com/product-2665.html) (SKU SEN0539)
offline AI voice recognition module.

The module ships with **150 built-in command words** and supports user-defined
command learning.  It communicates with a host MCU over either **UART** (push,
real-time) or **I²C** (polled).

---

## Features

| Feature | UART | I²C |
|---|---|---|
| Real-time command push | ✅ | ❌ (polled) |
| `on_command` trigger | ✅ | ✅ |
| `play_command` action | ✅ | ✅ |
| `set_volume` action | ✅ | ✅ |
| `set_mute` action | ✅ | ✅ |
| `set_wake_time` action | ✅ | ✅ |
| `reset` action | ✅ | ❌ |

---

## Installation

Add to your ESPHome YAML:

```yaml
external_components:
  - source: github://swhaat/esphome-dfq2301@main
    components: [df2301q]
```

---

## UART mode (recommended)

```yaml
uart:
  id: uart_bus
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

df2301q:
  id: voice_sensor
  uart_id: uart_bus

  on_command:
    then:
      - logger.log:
          format: "Recognised command ID: %d"
          args: [x]
      # Play back the reply audio for that command
      - df2301q.play_command:
          id: voice_sensor
          command_id: !lambda return x;
```

### UART wiring (Gravity connector)

| DF2301Q pin | ESP32 pin |
|---|---|
| VCC | 3.3 V or 5 V |
| GND | GND |
| TX  | GPIO16 (RX of ESP32) |
| RX  | GPIO17 (TX of ESP32) |

---

## I²C mode

```yaml
i2c:
  id: i2c_bus
  sda: GPIO21
  scl: GPIO22

df2301q:
  id: voice_sensor
  mode: i2c            # required to select I²C mode
  i2c_id: i2c_bus
  address: 0x64        # default; can be omitted
  update_interval: 100ms

  on_command:
    then:
      - logger.log:
          format: "Recognised command ID: %d"
          args: [x]
```

### I²C wiring

| DF2301Q pin | ESP32 pin |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SDA | GPIO21 |
| SCL | GPIO22 |

---

## Actions

### `df2301q.play_command`

Play the pre-recorded reply audio associated with a command word ID.

```yaml
- df2301q.play_command:
    id: voice_sensor
    command_id: 5        # or !lambda return x; for the current command
```

> **Note (UART):** The module takes ~1 s to play the audio; no delay is added
> automatically.  If you need to chain other actions, add a `delay: 1s`.

---

### `df2301q.set_volume`

Set speaker volume.  Range: **1 – 7**.

```yaml
- df2301q.set_volume:
    id: voice_sensor
    volume: 5
```

---

### `df2301q.set_mute`

Mute (`true`) or unmute (`false`) the speaker.

```yaml
- df2301q.set_mute:
    id: voice_sensor
    mute: true
```

---

### `df2301q.set_wake_time`

Set how long the module stays in the active (awake) state after recognising a
wake word.  Range: **0 – 255 seconds**.

```yaml
- df2301q.set_wake_time:
    id: voice_sensor
    wake_time: 30
```

---

### `df2301q.reset`  *(UART only)*

Perform a full software reset of the voice module.

```yaml
- df2301q.reset:
    id: voice_sensor
```

---

## Full example – home-automation voice control

```yaml
external_components:
  - source: github://swhaat/esphome-dfq2301@main
    components: [df2301q]

uart:
  id: uart_bus
  tx_pin: GPIO17
  rx_pin: GPIO16
  baud_rate: 9600

df2301q:
  id: voice
  uart_id: uart_bus

  on_command:
    then:
      - lambda: |-
          switch (x) {
            // Built-in command IDs (see DFRobot documentation)
            case 5:   // "Turn on the light"
              id(my_light).turn_on();
              break;
            case 6:   // "Turn off the light"
              id(my_light).turn_off();
              break;
            case 103: // "What time is it?" – just play the reply audio
              id(voice).play_command(103);
              break;
            default:
              ESP_LOGD("voice", "Unhandled command ID: %d", x);
          }

light:
  - platform: gpio
    id: my_light
    pin: GPIO2
    name: "Living Room Light"
```

---

## Known command IDs (built-in, English firmware)

| ID | Command |
|---|---|
| 1  | Wake word ("Hey Siri" variant) |
| 5  | Turn on the light |
| 6  | Turn off the light |
| 7  | Increase brightness |
| 8  | Decrease brightness |
| 9  | Maximum brightness |
| 10 | Minimum brightness |
| … | *(see DFRobot wiki for the full list)* |

---

## References

* [DFRobot DF2301Q wiki](https://wiki.dfrobot.com/SKU_SEN0539-EN_Offline_Voice_Recognition_Sensor_I2C_UART)
* [DFRobot Arduino library](https://github.com/DFRobot/DFRobot_DF2301Q)
* [ESPHome external components docs](https://esphome.io/components/external_components.html)
