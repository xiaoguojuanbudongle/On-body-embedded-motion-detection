#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <cstdint>

// Initialize the BLE stack, register custom Service & Characteristics, and start advertising
void ble_service_init();

// Called when detection results update; writes new level values into three characteristics
void ble_service_update(std::uint8_t tremor_level,
                        std::uint8_t dyskinesia_level,
                        std::uint8_t fog_level);

// Call periodically from the main loop to drive BLE protocol stack event processing
void ble_service_process();

#endif // BLE_SERVICE_H
