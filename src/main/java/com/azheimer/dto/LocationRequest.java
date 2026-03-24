package com.azheimer.dto;

import jakarta.validation.constraints.*;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.time.OffsetDateTime;

/**
 * GPS ping payload sent by the bracelet (ESP32 + SIM).
 */
@Getter
@Setter
@NoArgsConstructor
public class LocationRequest {

    @NotBlank(message = "deviceId is required")
    private String deviceId;

    @NotNull(message = "lat is required")
    @DecimalMin(value = "-90.0", message = "lat must be >= -90")
    @DecimalMax(value = "90.0",  message = "lat must be <= 90")
    private Double lat;

    @NotNull(message = "lng is required")
    @DecimalMin(value = "-180.0", message = "lng must be >= -180")
    @DecimalMax(value = "180.0",  message = "lng must be <= 180")
    private Double lng;

    @NotNull(message = "timestamp is required")
    private OffsetDateTime timestamp;

    /** GPS accuracy in metres; optional */
    @DecimalMin(value = "0.0", message = "accuracy must be non-negative")
    private Double accuracy;
}
