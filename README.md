# Alzheimer Tracker

GPS tracking system for Alzheimer patients with real-time location monitoring, medication reminders, and emergency alerts.

## 🚀 Quick Start

### Deploy Code Changes
```bash
./deploy.sh --force
```

### Access API
- **Swagger UI:** http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/swagger-ui.html
- **Base URL:** http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080

### First Time Setup
1. **[SSH Key Setup](SSH-KEY-SETUP.md)** - Place your `server-key.pem` in project root
2. **[Deploy Guide](HOW-TO-DEPLOY.md)** - Learn deployment workflow
3. **[Quick Reference](QUICK-REFERENCE.md)** - Common commands

## 📚 Documentation

| Doc | Purpose |
|-----|---------|
| [SSH-KEY-SETUP.md](SSH-KEY-SETUP.md) | Set up SSH key (first time only) |
| [HOW-TO-DEPLOY.md](HOW-TO-DEPLOY.md) | Deploy new versions |
| [QUICK-REFERENCE.md](QUICK-REFERENCE.md) | Quick commands |
| [ARCHITECTURE_AND_USAGE.txt](ARCHITECTURE_AND_USAGE.txt) | Full system docs |

## 🔧 Tech Stack

- Spring Boot 4.0 (Java 21) + PostgreSQL 18
- Docker + Docker Compose
- AWS EC2 (ARM64)
- ESP32 (GPS) + ESP8266/ESP32 (medication box)
