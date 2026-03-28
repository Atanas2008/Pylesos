package com.azheimer.filter;

import com.azheimer.entity.Device;
import com.azheimer.repository.DeviceRepository;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.servlet.FilterChain;
import jakarta.servlet.ServletException;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.http.MediaType;
import org.springframework.stereotype.Component;
import org.springframework.web.filter.OncePerRequestFilter;

import java.io.IOException;
import java.time.OffsetDateTime;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

/**
 * Validates X-Device-Key for hardware device endpoints.
 * Only POST /location and POST /medication require a device key.
 */
@Slf4j
@Component
@RequiredArgsConstructor
public class DeviceAuthFilter extends OncePerRequestFilter {

    private static final String HEADER = "X-Device-Key";

    /** Paths that require device API key authentication */
    private static final Set<String> PROTECTED_PATHS = Set.of("/location", "/medication");

    private final DeviceRepository deviceRepository;
    private final ObjectMapper objectMapper;

    @Override
    protected void doFilterInternal(HttpServletRequest request,
                                    HttpServletResponse response,
                                    FilterChain chain) throws ServletException, IOException {

        if (!requiresAuth(request)) {
            chain.doFilter(request, response);
            return;
        }

        String apiKey = request.getHeader(HEADER);
        if (apiKey == null || apiKey.isBlank()) {
            log.warn("Missing {} header from {}", HEADER, request.getRemoteAddr());
            writeUnauthorized(response, "Missing " + HEADER + " header");
            return;
        }

        Optional<Device> deviceOpt = deviceRepository.findByApiKey(apiKey);
        if (deviceOpt.isEmpty()) {
            log.warn("Invalid API key from {}", request.getRemoteAddr());
            writeUnauthorized(response, "Invalid device API key");
            return;
        }

        Device device = deviceOpt.get();
        if (!device.isActive()) {
            log.warn("Inactive device attempted auth: {}", device.getDeviceId());
            writeUnauthorized(response, "Device is deactivated");
            return;
        }

        // Update heartbeat timestamp
        device.setLastHeartbeat(OffsetDateTime.now());
        deviceRepository.save(device);

        log.debug("Device authenticated: {} ({})", device.getDeviceId(), device.getDeviceType());
        chain.doFilter(request, response);
    }

    private boolean requiresAuth(HttpServletRequest request) {
        if (!"POST".equalsIgnoreCase(request.getMethod())) {
            return false;
        }
        String path = request.getRequestURI();
        return PROTECTED_PATHS.stream().anyMatch(path::startsWith);
    }

    private void writeUnauthorized(HttpServletResponse response, String message) throws IOException {
        response.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
        response.setContentType(MediaType.APPLICATION_JSON_VALUE);
        Map<String, Object> body = Map.of("success", false, "message", message);
        response.getWriter().write(objectMapper.writeValueAsString(body));
    }
}
