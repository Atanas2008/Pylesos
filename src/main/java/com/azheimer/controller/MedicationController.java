package com.azheimer.controller;

import com.azheimer.dto.ApiResponse;
import com.azheimer.dto.MedicationEventRequest;
import com.azheimer.dto.MedicationEventResponse;
import com.azheimer.service.MedicationService;
import jakarta.validation.Valid;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.*;

import java.util.List;

@Slf4j
@RestController
@RequiredArgsConstructor
public class MedicationController {

    private final MedicationService medicationService;

    /**
     * POST /medication
     * Receives a dose event from the medication box (ESP8266/ESP32).
     * Requires X-Device-Key header (validated by DeviceAuthFilter).
     */
    @PostMapping("/medication")
    public ApiResponse<MedicationEventResponse> receiveMedicationEvent(
            @Valid @RequestBody MedicationEventRequest request) {
        try {
            MedicationEventResponse response = medicationService.recordEvent(request);
            return ApiResponse.ok("Medication event recorded", response);
        } catch (IllegalArgumentException e) {
            log.warn("Medication event rejected: {}", e.getMessage());
            return ApiResponse.error(e.getMessage());
        }
    }

    /**
     * GET /medication/{patientId}?limit=50
     * Returns medication event history for a patient, newest first.
     */
    @GetMapping("/medication/{patientId}")
    public ApiResponse<List<MedicationEventResponse>> getHistory(
            @PathVariable Long patientId,
            @RequestParam(defaultValue = "50") int limit) {
        try {
            List<MedicationEventResponse> history = medicationService.getHistory(patientId, Math.min(limit, 500));
            return ApiResponse.ok(history);
        } catch (IllegalArgumentException e) {
            log.warn("Medication history error: {}", e.getMessage());
            return ApiResponse.error(e.getMessage());
        }
    }
}
