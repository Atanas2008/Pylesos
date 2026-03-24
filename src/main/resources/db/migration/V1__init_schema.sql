-- ─── patients ──────────────────────────────────────────────────────────────────
CREATE TABLE patients (
    id                       BIGSERIAL    PRIMARY KEY,
    name                     VARCHAR(255) NOT NULL,
    birth_date               DATE,
    home_lat                 DOUBLE PRECISION,
    home_lng                 DOUBLE PRECISION,
    safe_zone_radius_meters  INTEGER      NOT NULL DEFAULT 200,
    emergency_contact_name   VARCHAR(255),
    emergency_contact_phone  VARCHAR(50),
    created_at               TIMESTAMPTZ  NOT NULL DEFAULT now()
);

-- ─── caregivers ────────────────────────────────────────────────────────────────
CREATE TABLE caregivers (
    id         BIGSERIAL    PRIMARY KEY,
    name       VARCHAR(255) NOT NULL,
    email      VARCHAR(255) NOT NULL UNIQUE,
    phone      VARCHAR(50),
    fcm_token  VARCHAR(512),
    created_at TIMESTAMPTZ  NOT NULL DEFAULT now()
);

-- ─── patient_caregivers (many-to-many join) ────────────────────────────────────
CREATE TABLE patient_caregivers (
    patient_id   BIGINT NOT NULL REFERENCES patients(id)   ON DELETE CASCADE,
    caregiver_id BIGINT NOT NULL REFERENCES caregivers(id) ON DELETE CASCADE,
    PRIMARY KEY (patient_id, caregiver_id)
);

-- ─── devices ───────────────────────────────────────────────────────────────────
CREATE TABLE devices (
    id             BIGSERIAL    PRIMARY KEY,
    device_id      VARCHAR(100) NOT NULL UNIQUE,
    device_type    VARCHAR(20)  NOT NULL,          -- BRACELET | MEDBOX
    api_key        VARCHAR(255) NOT NULL UNIQUE,
    patient_id     BIGINT       REFERENCES patients(id) ON DELETE SET NULL,
    last_heartbeat TIMESTAMPTZ,
    battery_level  SMALLINT     CHECK (battery_level BETWEEN 0 AND 100),
    active         BOOLEAN      NOT NULL DEFAULT TRUE,
    created_at     TIMESTAMPTZ  NOT NULL DEFAULT now()
);

-- ─── location_history ──────────────────────────────────────────────────────────
CREATE TABLE location_history (
    id         BIGSERIAL        PRIMARY KEY,
    patient_id BIGINT           NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    device_id  VARCHAR(100)     NOT NULL,
    lat        DOUBLE PRECISION NOT NULL,
    lng        DOUBLE PRECISION NOT NULL,
    accuracy   DOUBLE PRECISION,               -- metres; null if not reported
    timestamp  TIMESTAMPTZ      NOT NULL,
    created_at TIMESTAMPTZ      NOT NULL DEFAULT now()
);

CREATE INDEX idx_location_history_patient_ts
    ON location_history (patient_id, timestamp DESC);

-- ─── medication_schedules ──────────────────────────────────────────────────────
CREATE TABLE medication_schedules (
    id             BIGSERIAL    PRIMARY KEY,
    patient_id     BIGINT       NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    drug_name      VARCHAR(255) NOT NULL,
    compartment    SMALLINT     NOT NULL CHECK (compartment >= 1),
    scheduled_time TIME         NOT NULL,
    active         BOOLEAN      NOT NULL DEFAULT TRUE,
    created_at     TIMESTAMPTZ  NOT NULL DEFAULT now()
);

CREATE INDEX idx_medication_schedules_patient
    ON medication_schedules (patient_id) WHERE active = TRUE;

-- ─── medication_events ─────────────────────────────────────────────────────────
CREATE TABLE medication_events (
    id          BIGSERIAL    PRIMARY KEY,
    patient_id  BIGINT       NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    device_id   VARCHAR(100) NOT NULL,
    compartment SMALLINT     NOT NULL CHECK (compartment >= 1),
    event_type  VARCHAR(20)  NOT NULL,   -- OPENED | DOSE_TAKEN | DOSE_MISSED | LOW_BATTERY
    timestamp   TIMESTAMPTZ  NOT NULL,
    created_at  TIMESTAMPTZ  NOT NULL DEFAULT now()
);

CREATE INDEX idx_medication_events_patient_ts
    ON medication_events (patient_id, timestamp DESC);

-- ─── alerts ────────────────────────────────────────────────────────────────────
CREATE TABLE alerts (
    id          BIGSERIAL    PRIMARY KEY,
    patient_id  BIGINT       NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    alert_type  VARCHAR(30)  NOT NULL,   -- GEOFENCE_BREACH | MISSED_MEDICATION | LOW_BATTERY | DEVICE_OFFLINE | ABNORMAL_VITALS
    severity    VARCHAR(10)  NOT NULL,   -- LOW | MEDIUM | HIGH | CRITICAL
    message     TEXT,
    resolved    BOOLEAN      NOT NULL DEFAULT FALSE,
    resolved_at TIMESTAMPTZ,
    timestamp   TIMESTAMPTZ  NOT NULL,
    created_at  TIMESTAMPTZ  NOT NULL DEFAULT now()
);

CREATE INDEX idx_alerts_patient_ts
    ON alerts (patient_id, timestamp DESC);

CREATE INDEX idx_alerts_unresolved
    ON alerts (patient_id) WHERE resolved = FALSE;
