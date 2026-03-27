# How to Deploy

## Quick Deploy

```bash
./deploy.sh --force
```

**Done!** Wait 6-7 minutes, then check [Swagger](http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/swagger-ui.html).

---

## When to Use What

| Command | When | Time |
|---------|------|------|
| `./deploy.sh` | Small changes | 1-2 min |
| `./deploy.sh --force` | **New endpoints/files** | 6-7 min |

**Rule:** Use `--force` for new controllers or endpoints (avoids cache issues).

---

## Verify Deployment

```bash
# Check Swagger
open http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/swagger-ui.html

# Check logs
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker logs azheimer-backend | grep Started"

# Test endpoint
curl http://ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:8080/test
```

---

## Common Issues

### Endpoint not in Swagger?
**Cause:** Docker cache
**Fix:** `./deploy.sh --force`

### Backend keeps restarting?
**Check logs:**
```bash
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker logs azheimer-backend" | tail -50
```

Common cause: `application.properties` has `ddl-auto=validate` instead of `none`

### Old code still running?
**Fix:** Container wasn't recreated
```bash
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "cd ~/pylesos-clean && docker stop azheimer-backend && docker rm azheimer-backend && \
   docker-compose -f docker-compose.production.yml up -d backend"
```

---

## What Happens During Deploy

1. **Sync files** (10s) - Code → VM
2. **Build** (5-6 min) - Maven compile + Docker image
3. **Deploy** (30s) - Recreate container + start

---

## Manual Deploy (if needed)

```bash
# 1. Sync
rsync -avz --exclude='.git' --exclude='target' -e "ssh -i ./server-key.pem" \
  ./ ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com:~/pylesos-clean/

# 2. SSH
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com

# 3. Build & deploy
cd ~/pylesos-clean
docker build --no-cache -t azheimer-backend:latest -f Dockerfile .
docker stop azheimer-backend && docker rm azheimer-backend
docker-compose -f docker-compose.production.yml up -d backend
docker logs -f azheimer-backend
```

---

## Backup Before Major Changes

```bash
ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com \
  "docker exec azheimer-postgres pg_dump -U postgres azheimer_db" > backup_$(date +%Y%m%d).sql
```

---

## Quick Commands

```bash
# Deploy
./deploy.sh --force

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
