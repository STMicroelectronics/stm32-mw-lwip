/**
 * Return if SW component has been enabled in the project (or at least its dependancies)
 * @param {object} used_components used components returned by SWProjectAPI.getUsedComponents getter
 * @param {string} component component to be check
 */
function helper_lwip_component_is_enabled(used_components, component) {
  let result = 0;
  try {
    used_components.forEach((comp) => {
      if (
        comp["cgroup"] === "LWIP" &&
        comp["csub"].toUpperCase() === component.toUpperCase()
      ) {
        result = 1;
        return;
      }
    });
  } catch (e) {
    console.error(`[ERROR] helper_common_if_component_is_enabled: ${e}`);
  }
  return result;
}


module.exports = {
  helper_lwip_component_is_enabled
};
