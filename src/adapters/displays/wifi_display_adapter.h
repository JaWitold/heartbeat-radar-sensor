#pragma once

#include "adapters/displays/i_telemetry_display.h"

#ifdef ARDUINO
#include <WiFi.h>
#endif

#include <cstdint>
#include <string>

namespace hrs::adapters::displays
{

/// Streams the shared JSON telemetry line over a TCP server on WiFi-capable boards.
/// HUDs connect to `socket://<hostname>.local:<port>` (gui.sh --port, web.sh --serial); one client at a time.
class WifiDisplayAdapter : public ITelemetryDisplay
{
public:
    static constexpr uint16_t DEFAULT_TCP_PORT    = 3333;
    static constexpr const char* DEFAULT_HOSTNAME = "heartbeat-radar";

#ifdef ARDUINO
    WifiDisplayAdapter(const char* ssid,
                       const char* password,
                       const uint16_t port,
                       const char* hostname,
                       const float danger_distance_m,
                       const float caution_distance_m);
#else
    WifiDisplayAdapter(const float danger_distance_m, const float caution_distance_m);

    /// Host-only: the last line handed to the transport.
    [[nodiscard]] const std::string& last_frame() const;
#endif
    ~WifiDisplayAdapter() override = default;

    bool init() override;

protected:
    void send_line(const std::string& line) override;

private:
#ifdef ARDUINO
    const char* ssid_;
    const char* password_;
    const char* hostname_;
    uint16_t port_;
    WiFiServer server_;
    WiFiClient client_{};
#else
    std::string last_frame_{};
#endif
};

} // namespace hrs::adapters::displays
