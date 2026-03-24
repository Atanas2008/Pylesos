package com.azheimer.repository;

import com.azheimer.entity.MedicationEvent;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;

public interface MedicationEventRepository extends JpaRepository<MedicationEvent, Long> {

    List<MedicationEvent> findByPatient_IdOrderByTimestampDesc(Long patientId, Pageable pageable);
}
