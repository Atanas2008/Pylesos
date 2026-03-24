package com.azheimer.dto;

import com.azheimer.entity.MedicationEventType;
import lombok.Builder;
import lombok.Getter;

import java.time.OffsetDateTime;

@Getter
@Builder
public class MedicationEventResponse {

    private Long id;
    private Long patientId;
    private String deviceId;
    private Short compartment;
    private MedicationEventType eventType;
    private OffsetDateTime timestamp;
}
