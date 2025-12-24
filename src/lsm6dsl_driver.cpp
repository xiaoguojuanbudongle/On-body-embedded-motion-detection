#include "lsm6dsl_driver.h"
#include "config.h"

// I2C2: PB_11 = SDA, PB_10 = SCL (UM2153: I2C2_SDA / SCL)
static I2C i2c_lsm(PB_11, PB_10);

// LSM6DSL I2C address: 7-bit = 0x6A => 8-bit write address = 0xD4
static constexpr int LSM6DSL_I2C_ADDR_WRITE = 0xD4;
static constexpr int LSM6DSL_I2C_ADDR_READ  = 0xD5;

// Register addresses
static constexpr uint8_t REG_WHO_AM_I   = 0x0F;
static constexpr uint8_t REG_CTRL1_XL   = 0x10; // accelerometer control
static constexpr uint8_t REG_CTRL2_G    = 0x11; // gyroscope control
static constexpr uint8_t REG_CTRL3_C    = 0x12; // some global settings
static constexpr uint8_t REG_OUTX_L_XL  = 0x28; // accel X LSB (continues to ZH)

// WHO_AM_I expected = 0x6A
static constexpr uint8_t WHO_AM_I_EXPECTED = 0x6A;

// Write a register
static bool write_reg(uint8_t reg, uint8_t value)
{
    char data[2];
    data[0] = static_cast<char>(reg);
    data[1] = static_cast<char>(value);
    int rc = i2c_lsm.write(LSM6DSL_I2C_ADDR_WRITE, data, 2);
    return (rc == 0);
}

// Read multiple registers
static bool read_regs(uint8_t start_reg, uint8_t *buffer, size_t len)
{
    char reg = static_cast<char>(start_reg);
    // Write register address with repeated start
    int rc = i2c_lsm.write(LSM6DSL_I2C_ADDR_WRITE, &reg, 1, true);
    if (rc != 0) {
        return false;
    }
    rc = i2c_lsm.read(LSM6DSL_I2C_ADDR_READ, reinterpret_cast<char *>(buffer), len);
    return (rc == 0);
}

bool lsm6dsl_init()
{
    // I2C 400kHz
    i2c_lsm.frequency(400000);

    // Small delay to allow power-up to settle
    ThisThread::sleep_for(10ms);

    // Read WHO_AM_I
    uint8_t id = 0;
    if (!read_regs(REG_WHO_AM_I, &id, 1)) {
        printf("[LSM6DSL] WHO_AM_I read failed\r\n");
        return false;
    }

    if (id != WHO_AM_I_EXPECTED) {
        printf("[LSM6DSL] WHO_AM_I mismatch: 0x%02X (expected 0x%02X)\r\n", id, WHO_AM_I_EXPECTED);
        // Do not return false here to allow the rest of the firmware to
        // build/run when an IMU is not present. For real debugging it's
        // recommended to return false.
    } else {
        printf("[LSM6DSL] WHO_AM_I OK: 0x%02X\r\n", id);
    }

    // CTRL3_C defaults IF_INC=1 (auto address increment). Enable BDU
    // (Block Data Update) here for safer reads.
    // See Table 56: BOOT BDU H_LACTIVE PP_OD SIM IF_INC BLE SW_RESET
    // Set BDU = 1, IF_INC = 1, others = 0: 0b01000100 = 0x44
    if (!write_reg(REG_CTRL3_C, 0x44)) {
        printf("[LSM6DSL] Failed to write CTRL3_C\r\n");
        return false;
    }

    // Configure accelerometer CTRL1_XL:
    // ODR_XL[3:0] = 0b0011 => 52 Hz
    // FS_XL[1:0]  = 0b00   => ±2 g
    // BW0_XL / LPF1_BW_SEL left at 0
    // => 0b0011 0000 = 0x30
    if (!write_reg(REG_CTRL1_XL, 0x30)) {
        printf("[LSM6DSL] Failed to write CTRL1_XL\r\n");
        return false;
    }

    // Configure gyroscope CTRL2_G:
    // ODR_G[3:0] = 0b0011 => 52 Hz
    // FS_G[1:0]  = 0b00   => ±245 dps (sufficient)
    // => 0b0011 0000 = 0x30
    if (!write_reg(REG_CTRL2_G, 0x30)) {
        printf("[LSM6DSL] Failed to write CTRL2_G\r\n");
        return false;
    }

    printf("[LSM6DSL] Init done\r\n");
    return true;
}

bool lsm6dsl_read_accel(float &ax_g, float &ay_g, float &az_g)
{
    uint8_t raw[6] = {0};

    if (!read_regs(REG_OUTX_L_XL, raw, sizeof(raw))) {
        return false;
    }

    // According to the datasheet order:
    // OUTX_L_XL, OUTX_H_XL, OUTY_L_XL, OUTY_H_XL, OUTZ_L_XL, OUTZ_H_XL
    int16_t raw_x = static_cast<int16_t>(static_cast<int16_t>(raw[1]) << 8 | raw[0]);
    int16_t raw_y = static_cast<int16_t>(static_cast<int16_t>(raw[3]) << 8 | raw[2]);
    int16_t raw_z = static_cast<int16_t>(static_cast<int16_t>(raw[5]) << 8 | raw[4]);

    ax_g = raw_x * ACC_G_PER_LSB;
    ay_g = raw_y * ACC_G_PER_LSB;
    az_g = raw_z * ACC_G_PER_LSB;

    return true;
}
