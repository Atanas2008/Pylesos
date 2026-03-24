package com.azheimer.service;

import com.azheimer.dto.MedicationEventRequest;
import com.azheimer.dto.MedicationEventResponse;
import com.azheimer.entity.*;
import com.azheimer.repository.*;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.data.domain.PageRequest;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.OffsetDateTime;
import java.util.List;

@Slf4j
@Service
@RequiredArgsConstructor
public class MedicationService {

    private final MedicationEventRepository medicationEventRepository;
    private final DeviceRepository deviceRepository;
    private final PatientRepository patientRepository;
    private final AlertService alertService;

    @Transactional
    public MedicationEventResponse recordEvent(MedicationEventRequest request) {
        Device device = deviceRepository.findByDeviceId(request.getDeviceId())
                .orElseThrow(() -> new IllegalArgumentException(
                        "Unknown device: " + request.getDeviceId()));

        if (!device.isActive()) {
            throw new IllegalArgumentException("Device is inactive: " + request.getDeviceId());
        }

        Patient patient = device.getPatient();
        if (patient == null) {
            throw new IllegalArgumentException(
                    "Device " + request.getDeviceId() + " is not assigned to any patient");
        }

        MedicationEvent event = new MedicationEvent();
        event.setPatient(patient);
        event.setDeviceId(request.getDeviceId());
        event.setCompartment(request.getCompartment());
        event.setEventType(request.getEvent());
        event.setTimestamp(request.getTimestamp());
        medicationEventRepository.save(event);

        log.info("Medication event recorded: device={} patient={} compartment={} type={} ts={}",
                request.getDeviceId(), patient.getId(),
                request.getCompartment(), request.getEvent(), request.getTimestamp());

        handleAlerts(patient, event);

        return toResponse(event);
    }

    @Transactional(readOnly = true)
    public List<MedicationEventResponse> getHistory(Long patientId, int limit) {
        if (!patientRepository.existsById(patientId)) {
            throw new IllegalArgumentException("Patient not found: " + patientId);
        }
        return medicationEventRepository
                .findByPatient_IdOrderByTimestampDesc(patientId, PageRequest.of(0, limit))
                .stream()
                .map(this::toResponse)
                .toList();
    }

    private void handleAlerts(Patient patient, MedicationEvent event) {
        switch (event.getEventType()) {
            case DOSE_MISSED -> {
                log.warn("Missed dose: patient={} compartment={}", patient.getId(), event.getCompartment());
                alertService.createMissedMedicationAlert(patient, event.getCompartment());
            }
            case LOW_BATTERY -> {
                log.warn("Medication box low battery: device={}", event.getDeviceId());
                alertService.createLowBatteryAlert(patient, event.getDeviceId());
            }
            default -> { /* OPENED / DOSE_TAKEN — no alert needed */ }
        }
    }

    private MedicationEventResponse toResponse(MedicationEvent e) {
        return MedicationEventResponse.builder()
                .id(e.getId())
                .patientId(e.getPatient().getId())
                .deviceId(e.getDeviceId())
                .compartment(e.getCompartment())
                .eventType(e.getEventType())
                .timestamp(e.getTimestamp())
                .build();
    }
}
