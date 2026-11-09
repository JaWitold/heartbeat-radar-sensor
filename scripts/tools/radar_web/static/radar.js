/**
 * Tactical Heartbeat Radar — HTML5 Canvas Engine & WebSocket Client
 * Call of Duty MW2 / Rainbow Six Siege tactical CRT radar HUD
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

    // HiDPI Canvas setup
    function resizeCanvas() {
        const rect = canvas.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        canvas.width = rect.width * dpr;
        canvas.height = rect.height * dpr;
        ctx.scale(dpr, dpr);
    }
    window.addEventListener("resize", resizeCanvas);
    resizeCanvas();

    // --- Web Audio Alert ---
    btnAudio.addEventListener("click", () => {
        audioEnabled = !audioEnabled;
        if (audioEnabled) {
            btnAudio.textContent = "🔊 AUDIO: ON";
            btnAudio.classList.add("active");
            if (!audioCtx) {
                audioCtx = new (window.AudioContext || window.webkitAudioContext)();
            }
        } else {
            btnAudio.textContent = "🔊 AUDIO: OFF";
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
            threatLevel.textContent = "⚠️ PROXIMITY HAZARD";
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
        const width = canvas.getBoundingClientRect().width;
        const height = canvas.getBoundingClientRect().height;

        ctx.clearRect(0, 0, width, height);

        const originX = width / 2;
        const originY = height * 0.90;
        const maxRadius = Math.min(width * 0.45, height * 0.82);

        // Danger border flash if any target is in danger zone
        const isHazard = contacts.some(c => c.threat_zone === "Danger");
        if (isHazard) {
            ctx.save();
            ctx.strokeStyle = "rgba(255, 51, 51, 0.6)";
            ctx.lineWidth = 6;
            ctx.strokeRect(3, 3, width - 6, height - 6);
            ctx.restore();
        }

        // 1. Draw Range Rings (1m to 6m)
        ctx.save();
        for (let r = 1; r <= 6; r++) {
            const radius = (r / MAX_RANGE_M) * maxRadius;
            ctx.beginPath();
            // Arc from -60° to +60° relative to forward (-90° in standard math)
            const startAngle = (-90 + FOV_MIN_DEG) * Math.PI / 180;
            const endAngle = (-90 + FOV_MAX_DEG) * Math.PI / 180;
            ctx.arc(originX, originY, radius, startAngle, endAngle);

            if (r === 1 || r === 2) {
                ctx.strokeStyle = "rgba(255, 51, 51, 0.25)";
            } else if (r === 3) {
                ctx.strokeStyle = "rgba(255, 170, 0, 0.25)";
            } else {
                ctx.strokeStyle = "rgba(0, 255, 136, 0.20)";
            }
            ctx.lineWidth = 1;
            ctx.stroke();

            // Ring Label
            ctx.fillStyle = "rgba(0, 255, 136, 0.45)";
            ctx.font = "10px monospace";
            ctx.fillText(`${r}m`, originX + 6, originY - radius - 2);
        }

        // 2. Draw Azimuth Rays
        const rays = [-60, -45, -30, 0, 30, 45, 60];
        rays.forEach(deg => {
            const rad = (-90 + deg) * Math.PI / 180;
            const rx = originX + Math.cos(rad) * maxRadius;
            const ry = originY + Math.sin(rad) * maxRadius;

            ctx.beginPath();
            ctx.moveTo(originX, originY);
            ctx.lineTo(rx, ry);
            ctx.strokeStyle = (deg === 0) ? "rgba(0, 255, 136, 0.35)" : "rgba(0, 255, 136, 0.15)";
            ctx.lineWidth = (deg === 0) ? 1.5 : 1;
            ctx.stroke();

            // Ray label
            const labelDist = maxRadius + 14;
            const lx = originX + Math.cos(rad) * labelDist;
            const ly = originY + Math.sin(rad) * labelDist;
            ctx.fillStyle = "rgba(0, 255, 136, 0.5)";
            ctx.font = "9px monospace";
            ctx.textAlign = "center";
            ctx.fillText(`${deg > 0 ? "+" : ""}${deg}°`, lx, ly);
        });

        // 3. Draw Trailing Phosphor Sweep Fan
        const trailSpan = 18; // degrees of trail
        const trailRad = (-90 + sweepAngle) * Math.PI / 180;
        const trailBackRad = (-90 + (sweepAngle - sweepDirection * trailSpan)) * Math.PI / 180;

        const grad = ctx.createRadialGradient(originX, originY, 10, originX, originY, maxRadius);
        grad.addColorStop(0, "rgba(0, 255, 136, 0.25)");
        grad.addColorStop(1, "rgba(0, 255, 136, 0.0)");

        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.arc(
            originX, originY, maxRadius,
            Math.min(trailRad, trailBackRad),
            Math.max(trailRad, trailBackRad)
        );
        ctx.closePath();
        ctx.fillStyle = grad;
        ctx.fill();

        // 4. Draw Leading Sweep Beam Line
        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(
            originX + Math.cos(trailRad) * maxRadius,
            originY + Math.sin(trailRad) * maxRadius
        );
        ctx.strokeStyle = "#FFFFFF";
        ctx.shadowColor = "#00FF88";
        ctx.shadowBlur = 8;
        ctx.lineWidth = 2;
        ctx.stroke();
        ctx.shadowBlur = 0; // reset

        // 5. Draw Sensor Origin Node
        ctx.beginPath();
        ctx.arc(originX, originY, 4, 0, Math.PI * 2);
        ctx.fillStyle = "#00FF88";
        ctx.fill();

        // 6. Draw Tracked Contacts (Blips)
        const now = performance.now();
        contacts.forEach(c => {
            const ageSec = (now - c.receivedAt) / 1000.0;
            if (ageSec > 3.0) return; // Target timed out

            const opacity = Math.max(0.15, 1.0 - ageSec / 3.0);
            const normDist = Math.min(1.0, c.distance / MAX_RANGE_M);
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
            ctx.arc(bx, by, 8, 0, Math.PI * 2);
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
            ctx.fillStyle = "rgba(255, 255, 255, 0.85)";
            ctx.font = "bold 9px monospace";
            ctx.textAlign = "left";
            ctx.fillText(`T${c.id} [${c.distance.toFixed(1)}m]`, bx + 7, by - 4);
        });

        ctx.restore();
    }

    // Launch animation loop
    requestAnimationFrame(render);
})();
