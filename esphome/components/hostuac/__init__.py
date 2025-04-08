import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import speaker
from esphome.const import CONF_ID

DEPENDENCIES = ['esp32', 'speaker']
CODEOWNERS = ['@votre_username']

hostuac_ns = cg.esphome_ns.namespace('hostuac')
HostUACComponent = hostuac_ns.class_('HostUACComponent', cg.Component, cg.Parented.template(speaker.Speaker))

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(HostUACComponent),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
