import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import spi
from esphome.const import CONF_ID
from . import clrc663_ns, CLRC663SPI, CLRC663_BASE_SCHEMA, setup_clrc663_base

AUTO_LOAD = ["clrc663"]
CODEOWNERS = ["@JohnMcLear"]
DEPENDENCIES = ["spi"]

CONFIG_SCHEMA = CLRC663_BASE_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(CLRC663SPI),
    }
).extend(spi.spi_device_schema())


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await setup_clrc663_base(var, config)
    await spi.register_spi_device(var, config)
