#pragma once

#include "core/target_data.h"
#include "core/threat_assessment.h"
#include "ports/i_display.h"

#ifdef ARDUINO
#include <WiFi.h>
#endif

#include <cstdint>
#include <string>
#include <vector>

namespace hrs::adapters::displays
{

/// Streams the shared JSON telemetry line over a TCP server on WiFi-capable boards.
/// HUDs connect to `socket://<hostname>.local:<port>` (gui.sh --port, web.sh --serial); one client at a time.
class WifiDisplayAdapter : public ports::IDisplay
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
    void clear() override;
    void draw_sweep(const float angle_deg) override;
    void draw_radar(const std::vector<core::TargetData>& targets) override;
    void draw_alert(const core::ThreatAssessment& threat) override;
    void flush() override;

private:
    void send_line(const std::string& line);

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
    float danger_distance_m_;
    float caution_distance_m_;
    float sweep_angle_deg_{0.0f};
    core::ThreatAssessment threat_{};
    std::vector<core::TargetData> targets_{};
};

} // namespace hrs::adapters::displays
