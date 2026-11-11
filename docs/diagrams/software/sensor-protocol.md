# HLK-LD2450 UART Protocol Sequence

HLK-LD2450: **256000 bps (8N1)**. Streams binary report frames by default: X, Y, speed, distance resolution for up to 3 targets.

```mermaid
sequenceDiagram
    autonumber
    participant ESP as ESP32 (LD2450Adapter)
    participant LD as HLK-LD2450 Sensor

    Note over ESP,LD: Optional config mode (LD2450Adapter does not send it)
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
        ESP->>ESP: Validate header & tail (no checksum)
        ESP->>ESP: Parse sign-magnitude int16: X/Y mm, speed cm/s, dist res uint16 mm
        ESP->>ESP: Skip empty slots (X=Y=speed=0)
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

Total: **30 bytes**. Signed fields: sign-magnitude (bit 15 set = positive), little-endian.
