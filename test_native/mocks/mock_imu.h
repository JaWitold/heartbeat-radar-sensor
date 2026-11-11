#pragma once

#include "core/imu_data.h"
#include "ports/i_imu.h"

#include <cstdint>

namespace hrs::mocks
{

class MockImu : public ports::IImu
{
public:
    MockImu()           = default;
    ~MockImu() override = default;

    bool init() override
    {
        init_called_ = true;
        ++init_call_count_;
        return init_return_;
    }

    bool read_imu(core::ImuData& out) override
    {
        ++read_imu_call_count_;
        if (false == is_connected_ || false == imu_data_.is_valid)
        {
            return false;
        }
        out = imu_data_;
        return true;
    }

    [[nodiscard]] bool is_connected() const override
    {
        return is_connected_;
    }

    void set_init_return(const bool success)
    {
        init_return_ = success;
    }

    void set_connected(const bool connected)
    {
        is_connected_ = connected;
    }

    void set_imu_data(const core::ImuData& data)
    {
        imu_data_ = data;
    }

    [[nodiscard]] bool init_called() const
    {
        return init_called_;
    }

    [[nodiscard]] uint32_t init_call_count() const
    {
        return init_call_count_;
    }

    [[nodiscard]] uint32_t read_imu_call_count() const
    {
        return read_imu_call_count_;
    }

private:
    bool init_called_{false};
    bool init_return_{true};
    bool is_connected_{true};
    uint32_t init_call_count_{0};
    uint32_t read_imu_call_count_{0};
    core::ImuData imu_data_{};
};

} // namespace hrs::mocks
