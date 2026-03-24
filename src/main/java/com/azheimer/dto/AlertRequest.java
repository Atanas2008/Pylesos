package com.azheimer.dto;

import com.azheimer.entity.AlertSeverity;
import com.azheimer.entity.AlertType;
import jakarta.validation.constraints.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.time.OffsetDateTime;

/**
 * Manual alert trigger from the mobile app or internal service.
 */
@Getter
@Setter
@NoArgsConstructor
public class AlertRequest {

    @NotNull(message = "patientId is required")
    private Long patientId;

    @NotNull(message = "alertType is required")
    private AlertType alertType;

    @NotNull(message = "severity is required")
    private AlertSeverity severity;

    @Size(max = 1000, message = "message must be at most 1000 characters")
    private String message;

    @NotNull(message = "timestamp is required")
    private OffsetDateTime timestamp;
}
