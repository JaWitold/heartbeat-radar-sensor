#pragma once

#include "core/sensor_specs.h"
#include "core/target_data.h"

#include <vector>

namespace hrs::ports
{

class ISensor
{
public:
    virtual ~ISensor() = default;

    virtual bool init()                                   = 0;
    virtual std::vector<core::TargetData> scan()          = 0;
    [[nodiscard]] virtual core::SensorSpecs specs() const = 0;
    [[nodiscard]] virtual bool is_connected() const       = 0;
};

} // namespace hrs::ports
