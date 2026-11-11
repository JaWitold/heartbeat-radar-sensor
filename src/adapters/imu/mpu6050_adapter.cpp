#include "mpu6050_adapter.h"

#include "core/imu_data.h"

#ifdef ARDUINO
#include "core/mpu6050_parser.h"
#endif

namespace hrs::adapters::imu
{

#ifdef ARDUINO

namespace
{
constexpr uint8_t REG_SMPLRT_DIV   = 0x19;
constexpr uint8_t REG_CONFIG       = 0x1A;
constexpr uint8_t REG_GYRO_CONFIG  = 0x1B;
constexpr uint8_t REG_ACCEL_CONFIG = 0x1C;
constexpr uint8_t REG_ACCEL_XOUT_H = 0x3B;
constexpr uint8_t REG_PWR_MGMT_1   = 0x6B;
constexpr uint8_t REG_WHO_AM_I     = 0x75;

constexpr size_t MPU6050_BURST_LEN = 14;
} // namespace

MPU6050Adapter::MPU6050Adapter(
    TwoWire& wire, const int sda_pin, const int scl_pin, const uint8_t i2c_address, const uint32_t i2c_freq)
    : wire_(wire)
    , sda_pin_(sda_pin)
    , scl_pin_(scl_pin)
    , i2c_address_(i2c_address)
    , i2c_freq_(i2c_freq)
{
}

bool MPU6050Adapter::init()
{
    if (false == wire_.begin(sda_pin_, scl_pin_, i2c_freq_))
    {
        is_connected_ = false;
        return false;
    }

    uint8_t who_am_i = 0;
    if (false == read_registers(REG_WHO_AM_I, &who_am_i, 1))
    {
        is_connected_ = false;
        return false;
    }

    if (0x68 != who_am_i && 0x69 != who_am_i)
    {
        is_connected_ = false;
        return false;
    }

    // Wake device from sleep and select PLL with X axis gyroscope reference
    if (false == write_register(REG_PWR_MGMT_1, 0x01))
    {
        is_connected_ = false;
        return false;
    }

    // DLPF config: ~42Hz bandwidth
    write_register(REG_CONFIG, 0x03);

    // Sample rate divider: 1kHz / (1 + 9) = 100Hz
    write_register(REG_SMPLRT_DIV, 0x09);

    // Full-scale gyroscope range: ±250 deg/s
    write_register(REG_GYRO_CONFIG, 0x00);

    // Full-scale accelerometer range: ±2g
    write_register(REG_ACCEL_CONFIG, 0x00);

    is_connected_ = true;
    return true;
}

bool MPU6050Adapter::is_connected() const
{
    return is_connected_;
}

bool MPU6050Adapter::read_imu(core::ImuData& out)
{
    if (false == is_connected_)
    {
        out.is_valid = false;
        return false;
    }

    uint8_t buffer[MPU6050_BURST_LEN] = {0};
    if (false == read_registers(REG_ACCEL_XOUT_H, buffer, MPU6050_BURST_LEN))
    {
        // No latch: one I2C glitch drops this sample only, the next tick retries.
        out.is_valid = false;
        return false;
    }

    return core::parse_mpu6050_burst(buffer, MPU6050_BURST_LEN, out);
}

bool MPU6050Adapter::write_register(const uint8_t reg, const uint8_t value)
{
    wire_.beginTransmission(i2c_address_);
    wire_.write(reg);
    wire_.write(value);
    return (0 == wire_.endTransmission(true));
}

bool MPU6050Adapter::read_registers(const uint8_t reg, uint8_t* const buffer, const size_t length)
{
    wire_.beginTransmission(i2c_address_);
    wire_.write(reg);
    if (0 != wire_.endTransmission(false))
    {
        return false;
    }

    const uint8_t requested = static_cast<uint8_t>(length);
    if (requested != wire_.requestFrom(i2c_address_, requested))
    {
        return false;
    }

    for (size_t i = 0; length > i; ++i)
    {
        buffer[i] = static_cast<uint8_t>(wire_.read());
    }
    return true;
}

#else

MPU6050Adapter::MPU6050Adapter() = default;

bool MPU6050Adapter::init()
{
    is_connected_ = true;
    return true;
}

bool MPU6050Adapter::is_connected() const
{
    return is_connected_;
}

bool MPU6050Adapter::read_imu(core::ImuData& out)
{
    out.is_valid = is_connected_;
    return is_connected_;
}

#endif

} // namespace hrs::adapters::imu
