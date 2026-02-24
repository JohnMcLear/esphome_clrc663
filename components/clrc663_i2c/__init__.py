import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID
from . import clrc663_ns, CLRC663I2C, CLRC663_BASE_SCHEMA, setup_clrc663_base

AUTO_LOAD = ["clrc663"]
CODEOWNERS = ["@JohnMcLear"]
DEPENDENCIES = ["i2c"]

CONFIG_SCHEMA = (
    CLRC663_BASE_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(CLRC663I2C),
        }
    )
    .extend(i2c.i2c_device_schema(0x28))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await setup_clrc663_base(var, config)
    await i2c.register_i2c_device(var, config)
