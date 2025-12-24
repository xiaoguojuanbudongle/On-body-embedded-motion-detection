#include "ble_service.h"

#include "mbed.h"
#include "ble/BLE.h"
#include "ble/gap/AdvertisingDataBuilder.h"
#include "ble/GattServer.h"
#include "ble/gatt/GattCharacteristic.h"
#include "ble/gatt/GattService.h"

using namespace std::chrono_literals;

// Global BLE instance; avoid naming it 'ble' to prevent conflicts with namespace ble
static ble::BLE &g_ble = ble::BLE::Instance();

// Advertising data buffer + builder
static uint8_t adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
static ble::AdvertisingDataBuilder adv_builder(adv_buffer);

// Device name shown on the phone (we place it in the advertising packet)
static const char DEVICE_NAME[] = "RTES-F25";

// Custom Service / Char UUIDs (chosen from 0xF250..0xF253)
static const UUID SERVICE_UUID(0xF250);
static const UUID TREMOR_UUID(0xF251);
static const UUID DYSK_UUID(0xF252);
static const UUID FOG_UUID(0xF253);

// Three levels (0..3)
static uint8_t tremor_level = 0;
static uint8_t dysk_level   = 0;
static uint8_t fog_level    = 0;

static GattCharacteristic *tremor_char = nullptr;
static GattCharacteristic *dysk_char   = nullptr;
static GattCharacteristic *fog_char    = nullptr;
static GattService        *rtes_service = nullptr;

static bool g_ble_ready = false;

// Forward declarations
static void start_advertising();
static void on_ble_init_complete(ble::BLE::InitializationCompleteCallbackContext *params);

// Init-complete callback: build GATT table and start advertising
static void on_ble_init_complete(ble::BLE::InitializationCompleteCallbackContext *params)
{
    if (params->error != BLE_ERROR_NONE) {
        printf("[BLE] init failed, error = %d\r\n", params->error);
        return;
    }

    printf("[BLE] init done.\r\n");

    // 1. Create three characteristics (read + notify)
    tremor_level = 0;
    dysk_level   = 0;
    fog_level    = 0;

    tremor_char = new GattCharacteristic(
        TREMOR_UUID,
        &tremor_level,
        sizeof(tremor_level),
        sizeof(tremor_level),
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );

    dysk_char = new GattCharacteristic(
        DYSK_UUID,
        &dysk_level,
        sizeof(dysk_level),
        sizeof(dysk_level),
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );

    fog_char = new GattCharacteristic(
        FOG_UUID,
        &fog_level,
        sizeof(fog_level),
        sizeof(fog_level),
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
        GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY
    );

    GattCharacteristic *char_table[] = { tremor_char, dysk_char, fog_char };

    rtes_service = new GattService(
        SERVICE_UUID,
        char_table,
        sizeof(char_table) / sizeof(char_table[0])
    );

    ble_error_t err = g_ble.gattServer().addService(*rtes_service);
    if (err) {
        printf("[BLE] addService() failed: %d\r\n", err);
        return;
    }

    printf("[BLE] GATT service ready.\r\n");

    // 2. Configure advertising parameters + payload
    start_advertising();

    g_ble_ready = true;
}

// Use Mbed OS 6 AdvertisingParameters + AdvertisingDataBuilder to set up
// a simple connectable advertisement with a 250 ms interval
static void start_advertising()
{
    using namespace ble;  // used only inside this function; brings in symbols like LEGACY_ADVERTISING_HANDLE

    printf("[BLE] start_advertising()...\r\n");

    adv_builder.clear();

    // Basic flags (LE only)
    adv_builder.setFlags();

    // Put the device name into the advertising packet (what the phone sees)
    adv_builder.setName(DEVICE_NAME);

    AdvertisingParameters adv_params(
        advertising_type_t::CONNECTABLE_UNDIRECTED,
        adv_interval_t(millisecond_t(250))
    );

    ble_error_t err;

    err = g_ble.gap().setAdvertisingParameters(
        LEGACY_ADVERTISING_HANDLE,
        adv_params
    );
    if (err) {
        printf("[BLE] setAdvertisingParameters() failed: %d\r\n", err);
        return;
    }

    err = g_ble.gap().setAdvertisingPayload(
        LEGACY_ADVERTISING_HANDLE,
        adv_builder.getAdvertisingData()
    );
    if (err) {
        printf("[BLE] setAdvertisingPayload() failed: %d\r\n", err);
        return;
    }

    err = g_ble.gap().startAdvertising(LEGACY_ADVERTISING_HANDLE);
    if (err) {
        printf("[BLE] startAdvertising() failed: %d\r\n", err);
        return;
    }

    printf("[BLE] Advertising started.\r\n");
}

// Public interface: called from main()

void ble_service_init()
{
    if (g_ble.hasInitialized()) {
        return;
    }
    g_ble.init(on_ble_init_complete);
}

void ble_service_update(uint8_t tremor, uint8_t dyskinesia, uint8_t fog)
{
    if (!g_ble_ready) {
        return;
    }

    bool changed = false;
    GattServer &server = g_ble.gattServer();

    if (tremor_level != tremor && tremor_char) {
        tremor_level = tremor;
        server.write(
            tremor_char->getValueHandle(),
            &tremor_level,
            sizeof(tremor_level)
        );
        changed = true;
    }

    if (dysk_level != dyskinesia && dysk_char) {
        dysk_level = dyskinesia;
        server.write(
            dysk_char->getValueHandle(),
            &dysk_level,
            sizeof(dysk_level)
        );
        changed = true;
    }

    if (fog_level != fog && fog_char) {
        fog_level = fog;
        server.write(
            fog_char->getValueHandle(),
            &fog_level,
            sizeof(fog_level)
        );
        changed = true;
    }

    if (changed) {
        printf("[BLE] update: tremor=%u, dysk=%u, fog=%u\r\n",
               tremor_level, dysk_level, fog_level);
    }
}

// Call periodically from the main loop so BLE events are processed
void ble_service_process()
{
    g_ble.processEvents();
}
