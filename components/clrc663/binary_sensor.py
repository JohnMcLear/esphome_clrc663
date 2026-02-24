import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_UID, CONF_ID
from . import clrc663_ns, CLRC663, CONF_CLRC663_ID

DEPENDENCIES = ["clrc663"]

CLRC663BinarySensor = clrc663_ns.class_(
    "CLRC663BinarySensor", binary_sensor.BinarySensor
)


def validate_uid(value):
    value = cv.string_strict(value)
    # Support both hyphen and colon separated formats
    value = value.replace(":", "-")
    
    for part in value.split("-"):
        if len(part) != 2:
            raise cv.Invalid(
                "Each part (separated by '-' or ':') of the UID must be two characters long."
            )
        try:
            int_val = int(part, 16)
        except ValueError as e:
            raise cv.Invalid(
                "Valid characters for parts of a UID are 0123456789ABCDEF."
            ) from e
        if int_val < 0 or int_val > 255:
            raise cv.Invalid(
                "Valid values for UID parts (separated by '-' or ':') are 00 to FF"
            )
    return value


CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(CLRC663BinarySensor).extend(
    {
        cv.GenerateID(CONF_CLRC663_ID): cv.use_id(CLRC663),
        cv.Required(CONF_UID): validate_uid,
    }
)


async def to_code(config):
    var = await binary_sensor.new_binary_sensor(config)
    hub = await cg.get_variable(config[CONF_CLRC663_ID])
    cg.add(hub.register_tag(var))
    
    # Convert UID string to byte array
    uid_str = config[CONF_UID]
    uid_bytes = [int(x, 16) for x in uid_str.split("-")]
    cg.add(var.set_uid(uid_bytes))
