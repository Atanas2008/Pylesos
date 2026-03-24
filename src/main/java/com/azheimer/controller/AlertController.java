package com.azheimer.controller;

import com.azheimer.dto.AlertRequest;
import com.azheimer.dto.AlertResponse;
import com.azheimer.dto.ApiResponse;
import com.azheimer.service.AlertService;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@Slf4j
@RestController
@RequiredArgsConstructor
public class AlertController {

    private final AlertService alertService;

    /**
     * POST /alerts
     * Manually triggers a caregiver alert (e.g. from the mobile app or internal service).
     */
    @PostMapping("/alerts")
    public ApiResponse<AlertResponse> createAlert(@Valid @RequestBody AlertRequest request) {
        try {
            AlertResponse response = alertService.createAlert(request);
            return ApiResponse.ok("Alert created", response);
        } catch (IllegalArgumentException e) {
            log.warn("Alert creation error: {}", e.getMessage());
            return ApiResponse.error(e.getMessage());
        }
    }

    /**
     * GET /alerts/{patientId}?limit=50
     * Returns alert history for a patient, newest first.
     */
    @GetMapping("/alerts/{patientId}")
    public ApiResponse<List<AlertResponse>> getHistory(
            @PathVariable Long patientId,
            @RequestParam(defaultValue = "50") int limit) {
        try {
            List<AlertResponse> history = alertService.getHistory(patientId, Math.min(limit, 500));
            return ApiResponse.ok(history);
        } catch (IllegalArgumentException e) {
            log.warn("Alert history error: {}", e.getMessage());
            return ApiResponse.error(e.getMessage());
        }
    }
}
