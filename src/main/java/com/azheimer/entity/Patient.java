package com.azheimer.entity;

import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;
import lombok.NoArgsConstructor;

import java.time.LocalDate;
import java.time.OffsetDateTime;
import java.util.HashSet;
import java.util.Set;

@Entity
@Table(name = "patients")
@Getter
@Setter
@NoArgsConstructor
public class Patient {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private String name;

    private LocalDate birthDate;

    private Double homeLat;
    private Double homeLng;

    @Column(nullable = false)
    private Integer safeZoneRadiusMeters = 200;

    private String emergencyContactName;
    private String emergencyContactPhone;

    @Column(nullable = false, updatable = false)
    private OffsetDateTime createdAt = OffsetDateTime.now();

    @ManyToMany(mappedBy = "patients")
    private Set<Caregiver> caregivers = new HashSet<>();
}