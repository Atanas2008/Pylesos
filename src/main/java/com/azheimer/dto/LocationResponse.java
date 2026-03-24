package com.azheimer.dto;

import lombok.Builder;
import lombok.Getter;

import java.time.OffsetDateTime;

@Getter
@Builder
public class LocationResponse {

    private Long id;
    private Long patientId;
    private String deviceId;
    private Double lat;
    private Double lng;
    private Double accuracy;
    private OffsetDateTime timestamp;
}
