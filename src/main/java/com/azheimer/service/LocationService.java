package com.azheimer.service;

import com.azheimer.dto.LocationRequest;
import com.azheimer.dto.LocationResponse;
import com.azheimer.entity.*;
import com.azheimer.repository.*;
import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.springframework.data.domain.PageRequest;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;

@Slf4j
@Service
@RequiredArgsConstructor
public class LocationService {

    private final LocationHistoryRepository locationHistoryRepository;
    private final DeviceRepository deviceRepository;
    private final PatientRepository patientRepository;
    private final AlertService alertService;

    @Transactional
    public LocationResponse recordLocation(LocationRequest request) {
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

        LocationHistory entry = new LocationHistory();
        entry.setPatient(patient);
        entry.setDeviceId(request.getDeviceId());
        entry.setLat(request.getLat());
        entry.setLng(request.getLng());
        entry.setAccuracy(request.getAccuracy());
        entry.setTimestamp(request.getTimestamp());
        locationHistoryRepository.save(entry);

        log.info("Location recorded: device={} patient={} lat={} lng={} ts={}",
                request.getDeviceId(), patient.getId(),
                request.getLat(), request.getLng(), request.getTimestamp());

        checkGeofence(patient, request.getLat(), request.getLng());

        return toResponse(entry);
    }

    @Transactional(readOnly = true)
    public List<LocationResponse> getHistory(Long patientId, int limit) {
        if (!patientRepository.existsById(patientId)) {
            throw new IllegalArgumentException("Patient not found: " + patientId);
        }
        return locationHistoryRepository
                .findByPatient_IdOrderByTimestampDesc(patientId, PageRequest.of(0, limit))
                .stream()
                .map(this::toResponse)
                .toList();
    }

    private void checkGeofence(Patient patient, double lat, double lng) {
        if (patient.getHomeLat() == null || patient.getHomeLng() == null) {
            return; // no home location configured
        }
        double distanceMeters = haversineMeters(lat, lng, patient.getHomeLat(), patient.getHomeLng());
        if (distanceMeters > patient.getSafeZoneRadiusMeters()) {
            log.warn("Geofence breach for patient {}: distance={}m radius={}m",
                    patient.getId(), (int) distanceMeters, patient.getSafeZoneRadiusMeters());
            alertService.createGeofenceAlert(patient, lat, lng);
        }
    }

    /** Haversine great-circle distance in metres. */
    private double haversineMeters(double lat1, double lng1, double lat2, double lng2) {
        final double R = 6_371_000.0;
        double dLat = Math.toRadians(lat2 - lat1);
        double dLng = Math.toRadians(lng2 - lng1);
        double a = Math.sin(dLat / 2) * Math.sin(dLat / 2)
                + Math.cos(Math.toRadians(lat1)) * Math.cos(Math.toRadians(lat2))
                * Math.sin(dLng / 2) * Math.sin(dLng / 2);
        return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    }

    private LocationResponse toResponse(LocationHistory h) {
        return LocationResponse.builder()
                .id(h.getId())
                .patientId(h.getPatient().getId())
                .deviceId(h.getDeviceId())
                .lat(h.getLat())
                .lng(h.getLng())
                .accuracy(h.getAccuracy())
                .timestamp(h.getTimestamp())
                .build();
    }
}
