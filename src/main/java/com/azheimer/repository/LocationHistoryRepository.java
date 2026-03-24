package com.azheimer.repository;

import com.azheimer.entity.LocationHistory;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;

public interface LocationHistoryRepository extends JpaRepository<LocationHistory, Long> {

    List<LocationHistory> findByPatient_IdOrderByTimestampDesc(Long patientId, Pageable pageable);
}
