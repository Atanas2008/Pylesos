package com.azheimer.service;

import com.azheimer.dto.AlertRequest;
import com.azheimer.dto.AlertResponse;
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
public class AlertService {

    private final AlertRepository alertRepository;
    private final PatientRepository patientRepository;
    private final CaregiverRepository caregiverRepository;
    private final NotificationService notificationService;

    @Transactional
    public AlertResponse createAlert(AlertRequest request) {
        Patient patient = patientRepository.findById(request.getPatientId())
                .orElseThrow(() -> new IllegalArgumentException(
                        "Patient not found: " + request.getPatientId()));

        Alert alert = buildAlert(patient, request.getAlertType(), request.getSeverity(),
                request.getMessage(), request.getTimestamp());
        alertRepository.save(alert);

        log.info("Alert created: patient={} type={} severity={}", patient.getId(),
                request.getAlertType(), request.getSeverity());

        dispatchNotifications(alert);
        return toResponse(alert);
    }

    @Transactional(readOnly = true)
    public List<AlertResponse> getHistory(Long patientId, int limit) {
        if (!patientRepository.existsById(patientId)) {
            throw new IllegalArgumentException("Patient not found: " + patientId);
        }
        return alertRepository
                .findByPatient_IdOrderByTimestampDesc(patientId, PageRequest.of(0, limit))
                .stream()
                .map(this::toResponse)
                .toList();
    }

    /** Called internally when geofence breach is detected. */
    @Transactional
    public void createGeofenceAlert(Patient patient, double lat, double lng) {
        String msg = "Patient left safe zone (lat=%.5f, lng=%.5f)".formatted(lat, lng);
        Alert alert = buildAlert(patient, AlertType.GEOFENCE_BREACH, AlertSeverity.HIGH,
                msg, OffsetDateTime.now());
        alertRepository.save(alert);
        dispatchNotifications(alert);
    }

    /** Called internally when a dose is missed. */
    @Transactional
    public void createMissedMedicationAlert(Patient patient, int compartment) {
        String msg = "Missed medication in compartment " + compartment;
        Alert alert = buildAlert(patient, AlertType.MISSED_MEDICATION, AlertSeverity.MEDIUM,
                msg, OffsetDateTime.now());
        alertRepository.save(alert);
        dispatchNotifications(alert);
    }

    /** Called internally when a device reports low battery. */
    @Transactional
    public void createLowBatteryAlert(Patient patient, String deviceId) {
        String msg = "Low battery on device " + deviceId;
        Alert alert = buildAlert(patient, AlertType.LOW_BATTERY, AlertSeverity.LOW,
                msg, OffsetDateTime.now());
        alertRepository.save(alert);
        dispatchNotifications(alert);
    }

    private Alert buildAlert(Patient patient, AlertType type, AlertSeverity severity,
                             String message, OffsetDateTime timestamp) {
        Alert alert = new Alert();
        alert.setPatient(patient);
        alert.setAlertType(type);
        alert.setSeverity(severity);
        alert.setMessage(message);
        alert.setTimestamp(timestamp);
        return alert;
    }

    private void dispatchNotifications(Alert alert) {
        List<Caregiver> caregivers = caregiverRepository.findByPatients_Id(alert.getPatient().getId());
        caregivers.forEach(c -> notificationService.notify(c, alert));
    }

    private AlertResponse toResponse(Alert a) {
        return AlertResponse.builder()
                .id(a.getId())
                .patientId(a.getPatient().getId())
                .alertType(a.getAlertType())
                .severity(a.getSeverity())
                .message(a.getMessage())
                .resolved(a.isResolved())
                .resolvedAt(a.getResolvedAt())
                .timestamp(a.getTimestamp())
                .build();
    }
}
