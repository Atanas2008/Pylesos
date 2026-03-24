package com.azheimer.repository;

import com.azheimer.entity.Alert;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;

public interface AlertRepository extends JpaRepository<Alert, Long> {

    List<Alert> findByPatient_IdOrderByTimestampDesc(Long patientId, Pageable pageable);
}
