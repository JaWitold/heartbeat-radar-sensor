/**
 * Heartbeat Radar — HTML5 Canvas renderer & WebSocket client
 */

(function () {
    "use strict";

    // --- DOM Elements ---
    const canvas = document.getElementById("radar-canvas");
    const ctx = canvas.getContext("2d");
    const statusBadge = document.getElementById("status-badge");
    const statTargets = document.getElementById("stat-targets");
    const statFps = document.getElementById("stat-fps");
    const dangerBanner = document.getElementById("danger-banner");
    const telemetryBody = document.getElementById("telemetry-body");
    const threatIndicator = document.getElementById("threat-indicator");
    const threatLevel = document.getElementById("threat-level");
    const btnAudio = document.getElementById("btn-audio");
    const imuPitchRoll = document.getElementById("imu-pitch-roll");
    const imuYaw = document.getElementById("imu-yaw");
    const imuAccel = document.getElementById("imu-accel");
    const imuTemp = document.getElementById("imu-temp");

    // --- Radar Constants ---
    const MAX_RANGE_M = 6.0;
    const FOV_MIN_DEG = -60.0;
    const FOV_MAX_DEG = 60.0;

    // --- State ---
    let contacts = [];
    let sweepAngle = 0.0;
    let sweepDirection = 1; // 1 = right, -1 = left
    let sweepSpeed = 45.0;  // degrees per second
    let lastFrameTime = performance.now();
    let frameCount = 0;
    let fpsTimer = performance.now();
    let audioEnabled = false;
    let audioCtx = null;
    let lastBeepTime = 0;

    // --- Canvas Dimensions & HiDPI Setup ---
    let canvasWidth = 0;
    let canvasHeight = 0;

    function resizeCanvas() {
        const rect = canvas.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        const w = Math.floor(rect.width);
        const h = Math.floor(rect.height);
        if (w === 0 || h === 0) return;

        canvasWidth = w;
        canvasHeight = h;
        const targetWidth = Math.floor(w * dpr);
        const targetHeight = Math.floor(h * dpr);
        if (canvas.width !== targetWidth || canvas.height !== targetHeight) {
            canvas.width = targetWidth;
            canvas.height = targetHeight;
        }
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    }

    if (window.ResizeObserver) {
        const ro = new ResizeObserver(() => {
            resizeCanvas();
        });
        ro.observe(canvas.parentElement || canvas);
    } else {
        window.addEventListener("resize", resizeCanvas);
    }
    resizeCanvas();

    // --- Web Audio Alert ---
    btnAudio.addEventListener("click", () => {
        audioEnabled = !audioEnabled;
        if (audioEnabled) {
            btnAudio.textContent = "AUDIO: ON";
            btnAudio.classList.add("active");
            if (!audioCtx) {
                audioCtx = new (window.AudioContext || window.webkitAudioContext)();
            }
        } else {
            btnAudio.textContent = "AUDIO: OFF";
            btnAudio.classList.remove("active");
        }
    });

    function triggerBeep(freq = 2400, duration = 0.08) {
        if (!audioEnabled || !audioCtx) return;
        const now = audioCtx.currentTime;
        if (now - lastBeepTime < 0.25) return; // Debounce beeps
        lastBeepTime = now;

        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();
        osc.type = "sine";
        osc.frequency.setValueAtTime(freq, now);
        gain.gain.setValueAtTime(0.15, now);
        gain.gain.exponentialRampToValueAtTime(0.001, now + duration);
        osc.connect(gain);
        gain.connect(audioCtx.destination);
        osc.start(now);
        osc.stop(now + duration);
    }

    // --- WebSocket Connection ---
    function connectWebSocket() {
        const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
        const wsUrl = `${protocol}//${window.location.host}/ws`;

        const ws = new WebSocket(wsUrl);

        ws.onopen = () => {
            statusBadge.textContent = "CONNECTED";
            statusBadge.className = "badge badge-connected";
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.type === "radar_frame" && Array.isArray(data.contacts)) {
                    updateContacts(data.contacts);
                }
                if (data.imu) {
                    updateImu(data.imu);
                }
            } catch (err) {
                console.error("Failed to parse WebSocket JSON:", err);
            }
        };

        ws.onclose = () => {
            statusBadge.textContent = "DISCONNECTED";
            statusBadge.className = "badge badge-disconnected";
            setTimeout(connectWebSocket, 2000);
        };

        ws.onerror = () => {
            ws.close();
        };
    }
    connectWebSocket();

    // --- Telemetry & Blip Updates ---
    function updateImu(imu) {
        if (!imuPitchRoll) return;
        const pitchSign = imu.pitch >= 0 ? "+" : "";
        const rollSign = imu.roll >= 0 ? "+" : "";
        const yawSign = imu.yaw >= 0 ? "+" : "";
        imuPitchRoll.textContent = `${pitchSign}${imu.pitch.toFixed(1)}° / ${rollSign}${imu.roll.toFixed(1)}°`;
        if (imuYaw) imuYaw.textContent = `${yawSign}${imu.yaw.toFixed(1)}°`;
        if (imuAccel) imuAccel.textContent = `${imu.ax.toFixed(2)} / ${imu.ay.toFixed(2)} / ${imu.az.toFixed(2)} G`;
        if (imuTemp) imuTemp.textContent = `${imu.temp.toFixed(1)}°C`;
    }

    function updateContacts(newContacts) {
        const now = performance.now();
        contacts = newContacts.map(c => ({
            ...c,
            receivedAt: now,
        }));

        statTargets.textContent = contacts.length;
        renderSidebar();
    }

    function renderSidebar() {
        if (contacts.length === 0) {
            telemetryBody.innerHTML = `<tr><td colspan="6" class="no-data">NO CONTACTS IN SECTOR</td></tr>`;
            threatIndicator.className = "threat-box threat-safe";
            threatLevel.textContent = "ZONE CLEAR";
            dangerBanner.classList.add("hidden");
            return;
        }

        let hasDanger = false;
        let hasCaution = false;
        let html = "";

        contacts.forEach(c => {
            let zoneClass = "zone-safe";
            if (c.threat_zone === "Danger") {
                zoneClass = "zone-danger";
                hasDanger = true;
            } else if (c.threat_zone === "Caution") {
                zoneClass = "zone-caution";
                hasCaution = true;
            }

            html += `
                <tr>
                    <td><strong>#${c.id}</strong></td>
                    <td>${c.x > 0 ? "+" : ""}${c.x.toFixed(2)}</td>
                    <td>${c.y.toFixed(2)}</td>
                    <td>${c.distance.toFixed(2)}m</td>
                    <td>${c.speed.toFixed(1)}m/s</td>
                    <td><span class="zone-tag ${zoneClass}">${c.threat_zone}</span></td>
                </tr>
            `;
        });

        telemetryBody.innerHTML = html;

        if (hasDanger) {
            threatIndicator.className = "threat-box threat-danger";
            threatLevel.textContent = "PROXIMITY HAZARD";
            dangerBanner.classList.remove("hidden");
            triggerBeep(2600, 0.12);
        } else if (hasCaution) {
            threatIndicator.className = "threat-box threat-caution";
            threatLevel.textContent = "CAUTION // TARGET IN RANGE";
            dangerBanner.classList.add("hidden");
        } else {
            threatIndicator.className = "threat-box threat-safe";
            threatLevel.textContent = "ZONE CLEAR";
            dangerBanner.classList.add("hidden");
        }
    }

    // --- Render Loop (60 FPS) ---
    function render(currentTime) {
        requestAnimationFrame(render);

        const dt = (currentTime - lastFrameTime) / 1000.0;
        lastFrameTime = currentTime;

        // FPS counter
        frameCount++;
        if (currentTime - fpsTimer >= 1000.0) {
            statFps.textContent = Math.round((frameCount * 1000.0) / (currentTime - fpsTimer));
            frameCount = 0;
            fpsTimer = currentTime;
        }

        // Oscillating sweep angle calculation
        sweepAngle += sweepDirection * sweepSpeed * dt;
        if (sweepAngle >= FOV_MAX_DEG) {
            sweepAngle = FOV_MAX_DEG;
            sweepDirection = -1;
        } else if (sweepAngle <= FOV_MIN_DEG) {
            sweepAngle = FOV_MIN_DEG;
            sweepDirection = 1;
        }

        drawRadar();
    }

    function drawRadar() {
        if (canvasWidth === 0 || canvasHeight === 0) {
            resizeCanvas();
            if (canvasWidth === 0 || canvasHeight === 0) return;
        }

        const dpr = window.devicePixelRatio || 1;
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        ctx.clearRect(0, 0, canvasWidth, canvasHeight);

        // 120° sector fits:
        // Horizontal span = 2 * R * sin(60°) = 1.732 * R.
        // Vertical span = R.
        // Margins: 32px left/right, 24px top/bottom
        const availW = Math.max(100, canvasWidth - 64);
        const availH = Math.max(100, canvasHeight - 56);
        const maxRadius = Math.max(60, Math.min(availW / 1.732, availH));

        const originX = Math.round(canvasWidth / 2);
        const originY = Math.round((canvasHeight + maxRadius) / 2);

        // Danger border flash if any target is in danger zone
        const isHazard = contacts.some(c => c.threat_zone === "Danger");
        if (isHazard) {
            ctx.save();
            ctx.strokeStyle = "rgba(255, 51, 51, 0.65)";
            ctx.lineWidth = 4;
            ctx.strokeRect(2, 2, canvasWidth - 4, canvasHeight - 4);
            ctx.restore();
        }

        const arcStart = (-90 + FOV_MIN_DEG) * Math.PI / 180;
        const arcEnd = (-90 + FOV_MAX_DEG) * Math.PI / 180;

        ctx.save();

        // 1. Radar Sector Backdrop
        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.arc(originX, originY, maxRadius, arcStart, arcEnd);
        ctx.closePath();
        ctx.fillStyle = "rgba(4, 20, 12, 0.70)";
        ctx.fill();
        ctx.strokeStyle = "rgba(0, 255, 136, 0.35)";
        ctx.lineWidth = 1.5;
        ctx.stroke();

        // 2. Concentric Range Rings (1m to 6m)
        for (let r = 1; r <= 6; r++) {
            const radius = (r / MAX_RANGE_M) * maxRadius;
            ctx.beginPath();
            ctx.arc(originX, originY, radius, arcStart, arcEnd);

            if (r <= 2) {
                ctx.strokeStyle = "rgba(255, 51, 51, 0.35)";
            } else if (r === 3) {
                ctx.strokeStyle = "rgba(255, 170, 0, 0.30)";
            } else {
                ctx.strokeStyle = "rgba(0, 255, 136, 0.22)";
            }
            ctx.lineWidth = 1;
            ctx.stroke();

            // Distance labels along centerline
            ctx.fillStyle = (r <= 2) ? "rgba(255, 80, 80, 0.75)" : "rgba(0, 255, 136, 0.65)";
            ctx.font = "10px monospace";
            ctx.textAlign = "center";
            ctx.fillText(`${r}m`, originX, originY - radius - 3);
        }

        // 3. Azimuth Rays
        const rays = [-60, -45, -30, 0, 30, 45, 60];
        rays.forEach(deg => {
            const rad = (-90 + deg) * Math.PI / 180;
            const rx = originX + Math.cos(rad) * maxRadius;
            const ry = originY + Math.sin(rad) * maxRadius;

            ctx.beginPath();
            ctx.moveTo(originX, originY);
            ctx.lineTo(rx, ry);
            ctx.strokeStyle = (deg === 0) ? "rgba(0, 255, 136, 0.50)" : "rgba(0, 255, 136, 0.20)";
            ctx.lineWidth = (deg === 0) ? 1.5 : 1;
            ctx.stroke();

            // Degree label
            const labelDist = maxRadius + 14;
            const lx = originX + Math.cos(rad) * labelDist;
            const ly = originY + Math.sin(rad) * labelDist + 4;
            ctx.fillStyle = (deg === 0) ? "rgba(0, 255, 136, 0.90)" : "rgba(0, 255, 136, 0.55)";
            ctx.font = "9px monospace";
            ctx.textAlign = "center";
            ctx.fillText(`${deg > 0 ? "+" : ""}${deg}°`, lx, ly);
        });

        // 4. Phosphor Sweep Fan (multi-slice angular fade clamped to FOV)
        const trailSpanDeg = 20.0;
        const trailSteps = 12;
        for (let i = 0; i < trailSteps; i++) {
            const frac1 = i / trailSteps;
            const frac2 = (i + 1) / trailSteps;
            const a1_deg = sweepAngle - sweepDirection * (frac1 * trailSpanDeg);
            const a2_deg = sweepAngle - sweepDirection * (frac2 * trailSpanDeg);

            // Clamp both angles to FOV so trail never spills outside radar sector
            const ca1_deg = Math.max(FOV_MIN_DEG, Math.min(FOV_MAX_DEG, a1_deg));
            const ca2_deg = Math.max(FOV_MIN_DEG, Math.min(FOV_MAX_DEG, a2_deg));
            if (Math.abs(ca1_deg - ca2_deg) < 0.01) continue;

            const r1 = (-90 + ca1_deg) * Math.PI / 180;
            const r2 = (-90 + ca2_deg) * Math.PI / 180;
            const alpha = 0.24 * (1.0 - frac1);

            ctx.beginPath();
            ctx.moveTo(originX, originY);
            ctx.arc(originX, originY, maxRadius, Math.min(r1, r2), Math.max(r1, r2));
            ctx.closePath();
            ctx.fillStyle = `rgba(0, 255, 136, ${alpha.toFixed(3)})`;
            ctx.fill();
        }

        // 5. Leading Sweep Beam Line
        const beamRad = (-90 + sweepAngle) * Math.PI / 180;
        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(
            originX + Math.cos(beamRad) * maxRadius,
            originY + Math.sin(beamRad) * maxRadius
        );
        ctx.strokeStyle = "#FFFFFF";
        ctx.shadowColor = "#00FF88";
        ctx.shadowBlur = 6;
        ctx.lineWidth = 2;
        ctx.stroke();
        ctx.shadowBlur = 0;

        // 6. Sensor Origin Node & Forward Triangle
        ctx.beginPath();
        ctx.arc(originX, originY, 4, 0, Math.PI * 2);
        ctx.fillStyle = "#00FF88";
        ctx.fill();

        ctx.beginPath();
        ctx.moveTo(originX, originY - maxRadius - 2);
        ctx.lineTo(originX - 5, originY - maxRadius + 6);
        ctx.lineTo(originX + 5, originY - maxRadius + 6);
        ctx.closePath();
        ctx.fillStyle = "#00FF88";
        ctx.fill();

        // 7. Tracked Contacts (Blips)
        const now = performance.now();
        contacts.forEach(c => {
            const ageSec = (now - c.receivedAt) / 1000.0;
            if (ageSec > 3.0) return; // Stale

            const opacity = Math.max(0.15, 1.0 - ageSec / 3.0);
            const dist = Math.min(MAX_RANGE_M, Math.max(0, c.distance));
            const normDist = dist / MAX_RANGE_M;
            const screenDist = normDist * maxRadius;
            const targetRad = (-90 + c.angle_deg) * Math.PI / 180;

            const bx = originX + Math.cos(targetRad) * screenDist;
            const by = originY + Math.sin(targetRad) * screenDist;

            let color = "#00FF88";
            let glow = "rgba(0, 255, 136, ";
            if (c.threat_zone === "Danger") {
                color = "#FF3333";
                glow = "rgba(255, 51, 51, ";
            } else if (c.threat_zone === "Caution") {
                color = "#FFAA00";
                glow = "rgba(255, 170, 0, ";
            }

            // Outer Pulse Halo
            ctx.beginPath();
            ctx.arc(bx, by, 7, 0, Math.PI * 2);
            ctx.fillStyle = `${glow}${0.35 * opacity})`;
            ctx.fill();

            // Core Blip
            ctx.beginPath();
            ctx.arc(bx, by, 4, 0, Math.PI * 2);
            ctx.fillStyle = color;
            ctx.shadowColor = color;
            ctx.shadowBlur = 6;
            ctx.fill();
            ctx.shadowBlur = 0;

            // Target Tag Label
            ctx.fillStyle = "rgba(255, 255, 255, 0.9)";
            ctx.font = "bold 9px monospace";
            ctx.textAlign = "left";
            ctx.fillText(`T${c.id} [${dist.toFixed(1)}m]`, bx + 7, by - 4);
        });

        ctx.restore();
    }

    // Launch animation loop
    requestAnimationFrame(render);
})();
