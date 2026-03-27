package com.azheimer.controller;

import com.azheimer.dto.AlertResponse;
import com.azheimer.dto.ApiResponse;
import com.azheimer.service.AlertService;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

@Slf4j
@RestController
@RequiredArgsConstructor
public class TestController {

    private final AlertService alertService;

    /**
     * GET /alerts/{patientId}?limit=50
     * Returns alert history for a patient, newest first.
     */
    @GetMapping("/test/{patientId}")
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
