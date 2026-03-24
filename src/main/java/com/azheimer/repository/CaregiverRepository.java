package com.azheimer.repository;

import com.azheimer.entity.Caregiver;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;

public interface CaregiverRepository extends JpaRepository<Caregiver, Long> {

    List<Caregiver> findByPatients_Id(Long patientId);
}
