#include "wifi_display_adapter.h"

#include "adapters/displays/i_telemetry_display.h"

#ifdef ARDUINO
#include <ESPmDNS.h>
#include <lwip/sockets.h>
#endif

#include <string>

namespace hrs::adapters::displays
{

#ifdef ARDUINO
WifiDisplayAdapter::WifiDisplayAdapter(const char* ssid,
                                       const char* password,
                                       const uint16_t port,
                                       const char* hostname,
                                       const float danger_distance_m,
                                       const float caution_distance_m)
    : ITelemetryDisplay(danger_distance_m, caution_distance_m)
    , ssid_(ssid)
    , password_(password)
    , hostname_(hostname)
    , port_(port)
    , server_(port)
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
    : ITelemetryDisplay(danger_distance_m, caution_distance_m)
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

} // namespace hrs::adapters::displays
