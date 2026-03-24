package com.azheimer.repository;

import com.azheimer.entity.MedicationSchedule;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;

public interface MedicationScheduleRepository extends JpaRepository<MedicationSchedule, Long> {

    List<MedicationSchedule> findByPatient_IdAndActiveTrue(Long patientId);
}
