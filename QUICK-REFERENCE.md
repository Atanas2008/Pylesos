# Quick Reference

## Deploy
```bash
./deploy.sh --force    # New endpoints/controllers (6-7 min)
./deploy.sh            # Small changes (1-2 min)
```

## Access
- **Swagger:** http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/swagger-ui.html
- **API:** http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080

## SSH
```bash
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com
```

## Database
```bash
# Connect
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker exec -it azheimer-postgres psql -U postgres -d azheimer_db"

# Queries
\dt                              # List tables
SELECT * FROM patients;          # View patients
SELECT * FROM devices;           # View devices
SELECT * FROM location_history ORDER BY timestamp DESC LIMIT 10;
SELECT * FROM alerts ORDER BY timestamp DESC LIMIT 10;
\q                               # Exit
```

## Logs & Status
```bash
# Logs
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker logs -f azheimer-backend"

# Status
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker ps | grep azheimer"

# Restart
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "cd ~/pylesos-clean && docker-compose -f docker-compose.production.yml restart backend"
```

## Backup
```bash
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker exec azheimer-postgres pg_dump -U postgres azheimer_db" > backup_$(date +%Y%m%d).sql
```

## Test Credentials
- **Bracelet API Key:** `dev-key-bracelet-001-secret`
- **Medbox API Key:** `dev-key-medbox-001-secret`
- **DB Password:** `12345`
