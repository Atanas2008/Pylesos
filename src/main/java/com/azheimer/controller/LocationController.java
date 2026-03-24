package com.azheimer.controller;

import com.azheimer.dto.ApiResponse;
import com.azheimer.dto.LocationRequest;
import com.azheimer.dto.LocationResponse;
import com.azheimer.service.LocationService;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@Slf4j
@RestController
@RequiredArgsConstructor
public class LocationController {

    private final LocationService locationService;

    /**
     * POST /location
     * Receives a GPS ping from the bracelet (ESP32 + SIM).
     * Requires X-Device-Key header (validated by DeviceAuthFilter).
     */
    @PostMapping("/location")
    public ApiResponse<LocationResponse> receiveLocation(@Valid @RequestBody LocationRequest request) {
        try {
            LocationResponse response = locationService.recordLocation(request);
            return ApiResponse.ok("Location recorded", response);
        } catch (IllegalArgumentException e) {
            log.warn("Location rejected: {}", e.getMessage());
            return ApiResponse.error(e.getMessage());
        }
    }

    /**
     * GET /location/{patientId}?limit=50
     * Returns location history for a patient, newest first.
     */
    @GetMapping("/location/{patientId}")
    public ApiResponse<List<LocationResponse>> getHistory(
            @PathVariable Long patientId,
            @RequestParam(defaultValue = "50") int limit) {
        try {
            List<LocationResponse> history = locationService.getHistory(patientId, Math.min(limit, 500));
            return ApiResponse.ok(history);
        } catch (IllegalArgumentException e) {
            log.warn("Location history error: {}", e.getMessage());
            return ApiResponse.error(e.getMessage());
        }
    }
}
