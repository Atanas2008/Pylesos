package com.azheimer.repository;

import com.azheimer.entity.Device;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;

public interface DeviceRepository extends JpaRepository<Device, Long> {

    Optional<Device> findByApiKey(String apiKey);

    Optional<Device> findByDeviceId(String deviceId);
}
