package com.azheimer.dto;

import com.azheimer.entity.AlertSeverity;
import com.azheimer.entity.AlertType;
import lombok.Builder;
import lombok.Getter;

import java.time.OffsetDateTime;

@Getter
@Builder
public class AlertResponse {

    private Long id;
    private Long patientId;
    private AlertType alertType;
    private AlertSeverity severity;
    private String message;
    private boolean resolved;
    private OffsetDateTime resolvedAt;
    private OffsetDateTime timestamp;
}
