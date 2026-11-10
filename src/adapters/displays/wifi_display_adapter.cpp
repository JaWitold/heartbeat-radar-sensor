#include "wifi_display_adapter.h"

#include "core/target_data.h"
#include "core/telemetry_frame.h"
#include "core/threat_assessment.h"

#ifdef ARDUINO
#include <ESPmDNS.h>
#include <lwip/sockets.h>
#endif

#include <string>
#include <vector>

namespace hrs::adapters::displays
{

#ifdef ARDUINO
WifiDisplayAdapter::WifiDisplayAdapter(const char* ssid,
                                       const char* password,
                                       const uint16_t port,
                                       const char* hostname,
                                       const float danger_distance_m,
                                       const float caution_distance_m)
    : ssid_(ssid)
    , password_(password)
    , hostname_(hostname)
    , port_(port)
    , server_(port)
    , danger_distance_m_(danger_distance_m)
    , caution_distance_m_(caution_distance_m)
{
}

bool WifiDisplayAdapter::init()
{
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname_);
    // Non-blocking: the radar must start at once even without WiFi; the stack keeps reconnecting.
    WiFi.begin(ssid_, password_);
    Serial.printf(
        "[WifiDisplay] Joining '%s'; HUD: socket://%s.local:%u\n", ssid_, hostname_, static_cast<unsigned>(port_));

    MDNS.begin(hostname_);
    server_.begin();
    server_.setNoDelay(true);
    return true;
}

void WifiDisplayAdapter::send_line(const std::string& line)
{
    // A newer client replaces the old one, so a HUD that vanished without FIN can't lock others out.
    // shortcut: one client at a time (GUI + web together kick each other), use web.sh to fan out to many viewers.
    if (true == server_.hasClient())
    {
        client_.stop();
        client_ = server_.accept();
    }
    if (0 == client_.connected())
    {
        return;
    }

    // NetworkClient::write() retries for up to 10 s on a stalled peer; MSG_DONTWAIT keeps the 20 ms loop.
    // A partially sent line is resynced by the HUD at the next '\n'.
    const int sent = ::send(client_.fd(), line.data(), line.size(), MSG_DONTWAIT);
    if ((0 > sent) && (EAGAIN != errno) && (EWOULDBLOCK != errno))
    {
        client_.stop();
    }
}

#else

WifiDisplayAdapter::WifiDisplayAdapter(const float danger_distance_m, const float caution_distance_m)
    : danger_distance_m_(danger_distance_m)
    , caution_distance_m_(caution_distance_m)
{
}

const std::string& WifiDisplayAdapter::last_frame() const
{
    return last_frame_;
}

bool WifiDisplayAdapter::init()
{
    return true;
}

void WifiDisplayAdapter::send_line(const std::string& line)
{
    last_frame_ = line;
}

#endif

void WifiDisplayAdapter::clear()
{
    targets_.clear();
}

void WifiDisplayAdapter::draw_sweep(const float angle_deg)
{
    sweep_angle_deg_ = angle_deg;
}

void WifiDisplayAdapter::draw_radar(const std::vector<core::TargetData>& targets)
{
    targets_ = targets;
}

void WifiDisplayAdapter::draw_alert(const core::ThreatAssessment& threat)
{
    threat_ = threat;
}

void WifiDisplayAdapter::flush()
{
    send_line(
        core::encode_telemetry_frame(sweep_angle_deg_, threat_, targets_, danger_distance_m_, caution_distance_m_));
}

} // namespace hrs::adapters::displays
