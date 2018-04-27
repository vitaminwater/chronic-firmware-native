/*
 * BLE2902.cpp
 *
 *  Created on: Jun 25, 2017
 *      Author: kolban
 */

/*
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLE2901.h"

BLE2901::BLE2901(std::string n) : BLEDescriptor(BLEUUID((uint16_t) 0x2901)), name(n) {
	setValue(name);
}

std::string BLE2901::getName() {
  return name;
}

#endif
