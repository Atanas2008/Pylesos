package com.azheimer.dto;

import com.azheimer.entity.MedicationEventType;
import jakarta.validation.constraints.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.time.OffsetDateTime;

/**
 * Dose event payload sent by the medication box (ESP8266/ESP32).
 */
@Getter
@Setter
@NoArgsConstructor
public class MedicationEventRequest {

    @NotBlank(message = "deviceId is required")
    private String deviceId;

    @NotNull(message = "compartment is required")
    @Min(value = 1, message = "compartment must be >= 1")
    private Short compartment;

    @NotNull(message = "event is required")
    private MedicationEventType event;

    @NotNull(message = "timestamp is required")
    private OffsetDateTime timestamp;
}
