#pragma once

#include "core/imu_data.h"
#include "ports/i_imu.h"

#ifdef ARDUINO
#include <Arduino.h>
#include <Wire.h>
#endif

#include <cstddef>
#include <cstdint>

namespace hrs::adapters::imu
{

/// I2C hardware adapter for InvenSense MPU-6050 6-DOF IMU.
class MPU6050Adapter : public ports::IImu
{
public:
    static constexpr uint8_t DEFAULT_I2C_ADDRESS = 0x68;
    static constexpr uint32_t DEFAULT_I2C_FREQ   = 400000; // 400 kHz

#ifdef ARDUINO
    MPU6050Adapter(TwoWire& wire,
                   int sda_pin,
                   int scl_pin,
                   uint8_t i2c_address = DEFAULT_I2C_ADDRESS,
                   uint32_t i2c_freq   = DEFAULT_I2C_FREQ);
#else
    MPU6050Adapter();
#endif
    ~MPU6050Adapter() override = default;

    bool init() override;
    [[nodiscard]] bool is_connected() const override;
    bool read_imu(core::ImuData& out) override;

private:
#ifdef ARDUINO
    bool write_register(uint8_t reg, uint8_t value);
    bool read_registers(uint8_t reg, uint8_t* buffer, size_t length);

    TwoWire& wire_;
    int sda_pin_;
    int scl_pin_;
    uint8_t i2c_address_;
    uint32_t i2c_freq_;
#endif
    bool is_connected_{false};
};

} // namespace hrs::adapters::imu
