import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation, pins
from esphome.components import i2c, spi
from esphome.const import (
    CONF_ID,
    CONF_ON_TAG,
    CONF_ON_TAG_REMOVED,
    CONF_TRIGGER_ID,
)

CODEOWNERS = ["@JohnMcLear"]
DEPENDENCIES = []
AUTO_LOAD = []

CONF_CLRC663_ID = "clrc663_id"
CONF_IRQ_PIN = "irq_pin"
CONF_RESET_PIN = "reset_pin"
CONF_HEALTH_CHECK_ENABLED = "health_check_enabled"
CONF_HEALTH_CHECK_INTERVAL = "health_check_interval"
CONF_MAX_FAILED_CHECKS = "max_failed_checks"
CONF_AUTO_RESET_ON_FAILURE = "auto_reset_on_failure"

clrc663_ns = cg.esphome_ns.namespace("clrc663")
CLRC663 = clrc663_ns.class_("CLRC663", cg.PollingComponent)
CLRC663I2C = clrc663_ns.class_("CLRC663I2C", CLRC663, i2c.I2CDevice)
CLRC663SPI = clrc663_ns.class_(
    "CLRC663SPI", CLRC663, spi.SPIDevice
)

CLRC663Trigger = clrc663_ns.class_(
    "CLRC663Trigger", automation.Trigger.template(cg.std_string)
)
CLRC663IsWritingCondition = clrc663_ns.class_(
    "CLRC663IsWritingCondition", automation.Condition
)

CLRC663_BASE_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CLRC663),
        cv.Optional(CONF_ON_TAG): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(CLRC663Trigger),
            }
        ),
        cv.Optional(CONF_ON_TAG_REMOVED): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(CLRC663Trigger),
            }
        ),
        cv.Optional(CONF_HEALTH_CHECK_ENABLED, default=True): cv.boolean,
        cv.Optional(
            CONF_HEALTH_CHECK_INTERVAL, default="60s"
        ): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_MAX_FAILED_CHECKS, default=3): cv.int_range(min=1, max=10),
        cv.Optional(CONF_AUTO_RESET_ON_FAILURE, default=True): cv.boolean,
        cv.Optional(CONF_IRQ_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_RESET_PIN): pins.gpio_output_pin_schema,
    }
).extend(cv.polling_component_schema("1s"))


async def setup_clrc663_base(var, config):
    await cg.register_component(var, config)

    # Register triggers
    for conf in config.get(CONF_ON_TAG, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.add_on_tag_callback(trigger.process))
        await automation.build_automation(trigger, [(cg.std_string, "x")], conf)

    for conf in config.get(CONF_ON_TAG_REMOVED, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.add_on_tag_removed_callback(trigger.process))
        await automation.build_automation(trigger, [(cg.std_string, "x")], conf)

    # Health check configuration
    if CONF_HEALTH_CHECK_ENABLED in config:
        cg.add(var.set_health_check_enabled(config[CONF_HEALTH_CHECK_ENABLED]))
    if CONF_HEALTH_CHECK_INTERVAL in config:
        cg.add(var.set_health_check_interval(config[CONF_HEALTH_CHECK_INTERVAL]))
    if CONF_MAX_FAILED_CHECKS in config:
        cg.add(var.set_max_failed_checks(config[CONF_MAX_FAILED_CHECKS]))
    if CONF_AUTO_RESET_ON_FAILURE in config:
        cg.add(var.set_auto_reset_on_failure(config[CONF_AUTO_RESET_ON_FAILURE]))

    # Optional pins
    if CONF_IRQ_PIN in config:
        irq_pin = await cg.gpio_pin_expression(config[CONF_IRQ_PIN])
        cg.add(var.set_irq_pin(irq_pin))
    if CONF_RESET_PIN in config:
        reset_pin = await cg.gpio_pin_expression(config[CONF_RESET_PIN])
        cg.add(var.set_reset_pin(reset_pin))


async def register_clrc663(var, config):
    await setup_clrc663_base(var, config)
