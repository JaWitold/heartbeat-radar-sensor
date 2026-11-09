# HLK-LD2450 UART Protocol Sequence

The HLK-LD2450 mmWave sensor operates at **256000 bps (8N1)**. By default, it streams periodic binary reporting frames containing coordinates, speed, and distance for up to 3 detected targets.

```mermaid
sequenceDiagram
    autonumber
    participant ESP as ESP32 (LD2450Adapter)
    participant LD as HLK-LD2450 Sensor

    Note over ESP,LD: Sensor Initialization
    ESP->>LD: Send Config Mode Command (0xFD, 0xFC, 0xFB, 0xFA...)
    LD-->>ESP: Ack Configuration (0x04, 0x00, 0x00...)
    ESP->>LD: Enable Target Tracking & Multi-target Mode
    LD-->>ESP: Ack Success
    ESP->>LD: End Config Mode (0xFE, 0xFC, 0xFB, 0xFA...)

    Note over ESP,LD: Continuous Stream (~10Hz periodic frames)
    loop Every ~100 ms
        LD->>ESP: Frame Header (0xAA, 0xFF, 0x03, 0x00)
        LD->>ESP: Target 1 Data (X: 2B, Y: 2B, Speed: 2B, Distance Resolution: 2B)
        LD->>ESP: Target 2 Data (X: 2B, Y: 2B, Speed: 2B, Distance Resolution: 2B)
        LD->>ESP: Target 3 Data (X: 2B, Y: 2B, Speed: 2B, Distance Resolution: 2B)
        LD->>ESP: Frame Tail (0x55, 0xCC)
        ESP->>ESP: Validate Header & Tail Checksum
        ESP->>ESP: Parse Coordinates (convert 16-bit signed cm to float meters)
        ESP->>ESP: Filter inactive/noise targets
    end
```

---

## Frame Byte Layout

| Offset | Length | Field | Description |
|:------:|:------:|:-----:|:------------|
| 0 | 4 Bytes | Header | `0xAA 0xFF 0x03 0x00` |
| 4 | 8 Bytes | Target 1 | `X_L`, `X_H`, `Y_L`, `Y_H`, `Speed_L`, `Speed_H`, `Dist_Res_L`, `Dist_Res_H` |
| 12 | 8 Bytes | Target 2 | `X_L`, `X_H`, `Y_L`, `Y_H`, `Speed_L`, `Speed_H`, `Dist_Res_L`, `Dist_Res_H` |
| 20 | 8 Bytes | Target 3 | `X_L`, `X_H`, `Y_L`, `Y_H`, `Speed_L`, `Speed_H`, `Dist_Res_L`, `Dist_Res_H` |
| 28 | 2 Bytes | Tail | `0x55 0xCC` |

Total Frame Length: **30 Bytes**
