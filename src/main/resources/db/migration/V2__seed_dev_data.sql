-- ─── Development seed data ─────────────────────────────────────────────────────
-- Sample patient, caregiver, and devices so the API can be tested immediately.
-- DO NOT apply this migration in production.

INSERT INTO patients (name, birth_date, home_lat, home_lng, safe_zone_radius_meters,
                      emergency_contact_name, emergency_contact_phone)
VALUES ('Ivan Petrov', '1942-06-15', 42.6977, 23.3219, 300,
        'Maria Petrova', '+359-888-123456');

INSERT INTO caregivers (name, email, phone)
VALUES ('Maria Petrova', 'maria.petrova@example.com', '+359-888-123456');

INSERT INTO patient_caregivers (patient_id, caregiver_id) VALUES (1, 1);

-- Bracelet device for patient 1
INSERT INTO devices (device_id, device_type, api_key, patient_id, active)
VALUES ('bracelet-001', 'BRACELET', 'dev-key-bracelet-001-secret', 1, TRUE);

-- Medication box for patient 1
INSERT INTO devices (device_id, device_type, api_key, patient_id, active)
VALUES ('medbox-001', 'MEDBOX', 'dev-key-medbox-001-secret', 1, TRUE);

-- Medication schedule: morning pill in compartment 1 at 08:00
INSERT INTO medication_schedules (patient_id, drug_name, compartment, scheduled_time, active)
VALUES (1, 'Donepezil 10mg', 1, '08:00:00', TRUE);

-- Medication schedule: evening pill in compartment 2 at 20:00
INSERT INTO medication_schedules (patient_id, drug_name, compartment, scheduled_time, active)
VALUES (1, 'Memantine 10mg', 2, '20:00:00', TRUE);
