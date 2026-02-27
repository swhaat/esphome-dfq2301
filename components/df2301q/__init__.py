"""ESPHome component for the DFRobot DF2301Q offline voice recognition module.

The DF2301Q is an offline AI voice recognition module with 150 built-in command
words and support for user-defined commands.  It communicates over either UART
(preferred – device pushes results in real time) or I²C (polled).

Product page : https://www.dfrobot.com/product-2665.html  (SKU SEN0539)
DFRobot lib  : https://github.com/DFRobot/DFRobot_DF2301Q

YAML example (UART mode – recommended):
  uart:
    tx_pin: GPIO17
    rx_pin: GPIO16
    baud_rate: 9600

  df2301q:
    uart_id: uart_0
    on_command:
      then:
        - logger.log:
            format: "Recognised command ID: %d"
            args: [x]

YAML example (I²C mode):
  i2c:
    sda: GPIO21
    scl: GPIO22

  df2301q:
    mode: i2c
    address: 0x64          # default, can be omitted
    update_interval: 100ms # how often to poll for a new command
    on_command:
      then:
        - logger.log:
            format: "Recognised command ID: %d"
            args: [x]
"""

from esphome import automation
import esphome.codegen as cg
from esphome.components import i2c, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_MUTE,
    CONF_TRIGGER_ID,
)

CODEOWNERS = ["@swhaat"]
MULTI_CONF = True

# ── YAML keys ─────────────────────────────────────────────────────────────────
CONF_ON_COMMAND = "on_command"
CONF_COMMAND_ID = "command_id"
CONF_WAKE_TIME = "wake_time"
CONF_MODE = "mode"

# ── C++ namespace ─────────────────────────────────────────────────────────────
df2301q_ns = cg.esphome_ns.namespace("df2301q")

# Base mixin (no ESPHome Component inheritance – that comes from the sub-class)
DF2301QBase = df2301q_ns.class_("DF2301QBase")

# Concrete component types
DF2301QI2CComponent = df2301q_ns.class_(
    "DF2301QI2CComponent", cg.PollingComponent, i2c.I2CDevice
)
DF2301QUARTComponent = df2301q_ns.class_(
    "DF2301QUARTComponent", cg.Component, uart.UARTDevice
)

# Trigger type
DF2301QTrigger = df2301q_ns.class_(
    "DF2301QTrigger", automation.Trigger.template(cg.uint8)
)

# Action types – all operate on DF2301QBase so they work with both modes
DF2301QPlayCommandAction = df2301q_ns.class_(
    "DF2301QPlayCommandAction", automation.Action
)
DF2301QSetVolumeAction = df2301q_ns.class_(
    "DF2301QSetVolumeAction", automation.Action
)
DF2301QSetMuteAction = df2301q_ns.class_(
    "DF2301QSetMuteAction", automation.Action
)
DF2301QSetWakeTimeAction = df2301q_ns.class_(
    "DF2301QSetWakeTimeAction", automation.Action
)
DF2301QResetAction = df2301q_ns.class_(
    "DF2301QResetAction", automation.Action
)

# ── Shared trigger schema ─────────────────────────────────────────────────────
_TRIGGER_SCHEMA = automation.validate_automation(
    {
        cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(DF2301QTrigger),
    }
)

# ── Mode-specific schemas ─────────────────────────────────────────────────────
_I2C_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DF2301QI2CComponent),
            cv.Optional(CONF_ON_COMMAND): _TRIGGER_SCHEMA,
        }
    )
    .extend(cv.polling_component_schema("100ms"))
    .extend(i2c.i2c_device_schema(0x64))
)

_UART_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DF2301QUARTComponent),
            cv.Optional(CONF_ON_COMMAND): _TRIGGER_SCHEMA,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

# Top-level schema selects mode via the optional "mode" key.
# Default is "uart" so existing configs without a mode key keep working.
CONFIG_SCHEMA = cv.typed_schema(
    {
        "uart": _UART_SCHEMA,
        "i2c": _I2C_SCHEMA,
    },
    key=CONF_MODE,
    default_type="uart",
)

# Validate UART settings when in UART mode.
FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "df2301q",
    baud_rate=9600,
    require_tx=True,
    require_rx=True,
)


# ── to_code ───────────────────────────────────────────────────────────────────
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    mode = config.get(CONF_MODE, "uart")
    if mode == "i2c":
        await i2c.register_i2c_device(var, config)
    else:
        await uart.register_uart_device(var, config)

    for conf in config.get(CONF_ON_COMMAND, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.register_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "x")], conf)


# ── Actions ───────────────────────────────────────────────────────────────────
# The action schemas accept an ID for either component variant.
_COMPONENT_ID = cv.Any(
    cv.use_id(DF2301QI2CComponent),
    cv.use_id(DF2301QUARTComponent),
)


# df2301q.play_command
@automation.register_action(
    "df2301q.play_command",
    DF2301QPlayCommandAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(): _COMPONENT_ID,
            cv.Required(CONF_COMMAND_ID): cv.templatable(cv.uint8_t),
        },
        key=CONF_COMMAND_ID,
    ),
)
async def df2301q_play_command_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    cmd = await cg.templatable(config[CONF_COMMAND_ID], args, cg.uint8)
    cg.add(var.set_cmd_id(cmd))
    return var


# df2301q.set_volume  (1–7)
@automation.register_action(
    "df2301q.set_volume",
    DF2301QSetVolumeAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(): _COMPONENT_ID,
            cv.Required("volume"): cv.templatable(cv.int_range(min=1, max=7)),
        },
        key="volume",
    ),
)
async def df2301q_set_volume_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    vol = await cg.templatable(config["volume"], args, cg.uint8)
    cg.add(var.set_volume(vol))
    return var


# df2301q.set_mute
@automation.register_action(
    "df2301q.set_mute",
    DF2301QSetMuteAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(): _COMPONENT_ID,
            cv.Required(CONF_MUTE): cv.templatable(cv.boolean),
        },
        key=CONF_MUTE,
    ),
)
async def df2301q_set_mute_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    mute = await cg.templatable(config[CONF_MUTE], args, bool)
    cg.add(var.set_mute(mute))
    return var


# df2301q.set_wake_time  (0–255 seconds)
@automation.register_action(
    "df2301q.set_wake_time",
    DF2301QSetWakeTimeAction,
    cv.maybe_simple_value(
        {
            cv.GenerateID(): _COMPONENT_ID,
            cv.Required(CONF_WAKE_TIME): cv.templatable(cv.uint8_t),
        },
        key=CONF_WAKE_TIME,
    ),
)
async def df2301q_set_wake_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    wt = await cg.templatable(config[CONF_WAKE_TIME], args, cg.uint8)
    cg.add(var.set_wake_time(wt))
    return var


# df2301q.reset  (UART: full module reset; I²C: no-op)
@automation.register_action(
    "df2301q.reset",
    DF2301QResetAction,
    {cv.GenerateID(): _COMPONENT_ID},
)
async def df2301q_reset_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var
