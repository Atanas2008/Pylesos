# CLAUDE.md — Alzheimer Tracker Project

## Project overview

This is an Alzheimer's patient tracking system consisting of hardware devices,
a REST API backend, a mobile app, and a notification service. The goal is to
monitor patient location, track medication intake, and alert caregivers in
real time.

---

## Architecture

```
Hardware layer          Transport              Backend layer
─────────────────       ─────────────          ──────────────────────────
GPS bracelet            SIM / GPRS    ──┐
(ESP32 + SIM)                           ├──► REST API (Spring Boot)
                                        │         │
Medication box          WiFi / HTTPS ──┘         ├──► Database
(ESP8266/32 + WiFi)                               │    (PostgreSQL)
                                                  │
Mobile app              HTTPS / REST ────────────►├──► Alert service
(Flutter)                                              (Push / SMS / email)
```

### Key rule
**All hardware communicates exclusively through the REST API.
The database is never exposed directly to any device or client.**

---

## Components

### 1. GPS bracelet
- **Hardware**: ESP32 + SIM module (GPRS)
- **Purpose**: Real-time location tracking, vitals monitoring
- **Transport**: HTTP POST over GPRS to REST API
- **Payload**: Keep small — JSON with lat, lng, timestamp, accuracy
- **Offline handling**: Buffer to EEPROM/LittleFS when SIM has no signal,
  flush on reconnect

### 2. Medication box
- **Hardware**: ESP8266 or ESP32 + WiFi
- **Purpose**: Track whether medication compartments were opened at scheduled times
- **Transport**: HTTPS over WiFi to REST API
- **Events**: Compartment opened, dose taken, dose missed, low battery

### 3. Mobile app
- **Framework**: Flutter (iOS and Android)
- **Users**: Caregivers
- **Features**: Live map view, medication schedule, alert history, patient profile
- **Transport**: HTTPS REST calls to the API
- **Notifications**: Receives push notifications from the alert service

### 4. REST API server
- **Framework**: Spring Boot microservices
- **Language**: Java
- **Main endpoints**:
  - `POST /location` — receives GPS ping from bracelet
  - `POST /medication` — receives dose event from medication box
  - `GET  /location/{patientId}` — returns location history
  - `GET  /medication/{patientId}` — returns medication history
  - `POST /alerts` — triggers caregiver alert
  - `GET  /alerts/{patientId}` — returns alert history
- **Auth**: Pre-shared API key per hardware device stored in device flash
- **Validation**: All input validated before persisting

### 5. Database
- **Engine**: PostgreSQL (with PostGIS extension for geospatial queries)
- **Key tables**:
  - `patients` — profile, safe zone radius, emergency contacts
  - `caregivers` — linked to patients via join table (many-to-many)
  - `devices` — ESP device registry, API keys, last heartbeat
  - `location_history` — patient_id, lat, lng, timestamp, accuracy
  - `medication_schedules` — drug, compartment, scheduled time
  - `medication_events` — compartment opened, dose taken/missed, timestamp
  - `alerts` — type, severity, resolved status, timestamp
- **Geofencing**: Use `ST_DWithin(location, home_point, radius)` for safe zone checks

### 6. Alert service
- **Purpose**: Notify caregivers when something requires attention
- **Triggers**:
  - Patient left safe zone (geofence breach)
  - Missed medication dose
  - Device low battery or offline
  - Abnormal vitals (if supported)
- **Channels**: Push notifications (mobile), SMS, email

---

## Tech stack summary

| Layer        | Technology                        |
|--------------|-----------------------------------|
| Bracelet MCU | ESP32                             |
| Box MCU      | ESP8266 / ESP32                   |
| Transport    | GPRS (SIM), WiFi, HTTPS           |
| Mobile app   | Flutter                           |
| Backend      | Spring Boot (Java)                |
| Database     | PostgreSQL + PostGIS               |
| Alerts       | Firebase FCM / Twilio / SendGrid  |
| Dev infra    | Docker, Docker Compose            |

---

## Development environment

- **Host OS**: Windows 11
- **Database runs in**: Docker container which is run locally on Windows 11
- **DB port exposed**: `5432` (PostgreSQL)
- **IDE**: IntelliJ IDEA

---

## Code conventions

- Follow existing Spring Boot project structure
- All REST endpoints return HTTP 200 even on business errors
  (error details in response body)
- Use DTOs for all API input/output — never expose entity classes directly
- Validate all hardware payloads before processing
- Log all device events with device ID and timestamp
- Never hardcode credentials — use environment variables or application.properties

---

## Hardware conventions

- ESP devices authenticate with a pre-shared API key in the HTTP header:
  `X-Device-Key: <key>`
- GPS payload example:
  ```json
  {
    "deviceId": "bracelet-001",
    "lat": 42.6977,
    "lng": 23.3219,
    "timestamp": "2026-03-24T10:30:00Z",
    "accuracy": 5.2
  }
  ```
- Medication event payload example:
  ```json
  {
    "deviceId": "medbox-001",
    "compartment": 2,
    "event": "OPENED",
    "timestamp": "2026-03-24T08:00:05Z"
  }
  ```

---

## What Claude should help with

- Spring Boot endpoint implementation and validation logic
- PostgreSQL schema design and migration scripts
- PostGIS geofencing query implementation
- Flutter mobile app screens and API integration
- ESP32/ESP8266 firmware for HTTP POST and offline buffering
- Docker Compose setup for local development
- Alert service logic and notification integration
