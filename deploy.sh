#!/bin/bash
# Deploy script - Run from your LOCAL machine to deploy to VM

set -e

VM_HOST="ec2-16-16-142-88.eu-north-1.compute.amazonaws.com"
VM_USER="ec2-user"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SSH_KEY="$SCRIPT_DIR/server-key.pem"
LOCAL_PROJECT="$SCRIPT_DIR"
REMOTE_PROJECT="~/pylesos-clean"

# Parse command line arguments
FORCE_REBUILD=false
if [[ "$1" == "--force" ]] || [[ "$1" == "-f" ]]; then
    FORCE_REBUILD=true
fi

echo "=========================================="
echo "   Alzheimer Tracker - Deploy to VM"
echo "=========================================="
echo ""

# Check if SSH key exists
if [ ! -f "$SSH_KEY" ]; then
    echo "❌ SSH key not found at: $SSH_KEY"
    echo "💡 Place your server-key.pem in: $SCRIPT_DIR"
    exit 1
fi

echo "✅ SSH key found"
echo ""

# Fix SSH key permissions on Windows (Git Bash doesn't support chmod on NTFS)
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    WIN_KEY=$(cygpath -w "$SSH_KEY")
    powershell -Command "\$acl = Get-Acl '$WIN_KEY'; \$acl.SetAccessRuleProtection(\$true, \$false); \$username = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name; \$rule = New-Object System.Security.AccessControl.FileSystemAccessRule(\$username,'Read','Allow'); \$acl.SetAccessRule(\$rule); Set-Acl '$WIN_KEY' \$acl" > /dev/null 2>&1 \
        && echo "✅ SSH key permissions fixed (Windows)" \
        || echo "⚠️  Could not fix key permissions — deploy may fail"
fi

# Sync files to VM
echo "📤 Syncing files to VM..."
ssh -i "$SSH_KEY" -o StrictHostKeyChecking=no "$VM_USER@$VM_HOST" "mkdir -p $REMOTE_PROJECT"
tar czf - -C "$LOCAL_PROJECT" \
    --exclude='./.git' \
    --exclude='./target' \
    --exclude='./.idea' \
    --exclude='./.DS_Store' \
    --exclude='./server-key.pem' \
    . \
  | ssh -i "$SSH_KEY" -o StrictHostKeyChecking=no "$VM_USER@$VM_HOST" "tar xzf - -C $REMOTE_PROJECT"

if [ $? -ne 0 ]; then
    echo "❌ File sync failed"
    exit 1
fi

echo "✅ Files synced successfully"
echo ""

# Build and deploy on VM
if [ "$FORCE_REBUILD" = true ]; then
    echo "🏗️  Force rebuilding Docker image (--no-cache)..."
else
    echo "🏗️  Building Docker image on VM..."
fi

ssh -i "$SSH_KEY" -o StrictHostKeyChecking=no "$VM_USER@$VM_HOST" << ENDSSH
cd ~/pylesos-clean

# Build with or without cache
if [ "$FORCE_REBUILD" = true ]; then
    echo "Building with --no-cache (this takes longer but ensures fresh build)..."
    docker build --no-cache -t azheimer-backend:latest -f Dockerfile . 2>&1 | grep -E "Step|Compiling|BUILD|ERROR" | tail -20
else
    echo "Building Docker image..."
    docker build -t azheimer-backend:latest -f Dockerfile . 2>&1 | tail -20
fi

if [ \$? -ne 0 ]; then
    echo "❌ Docker build failed"
    exit 1
fi

echo ""
echo "🔄 Recreating backend container (ensures new image is used)..."
docker stop azheimer-backend
docker rm azheimer-backend
docker-compose -f docker-compose.production.yml up -d backend

echo ""
echo "⏳ Waiting for startup (30 seconds)..."
sleep 30

echo ""
echo "📊 Container status:"
docker ps | grep azheimer

echo ""
echo "📝 Recent logs:"
docker logs --tail 30 azheimer-backend 2>&1 | grep -E "Started AzhelmerApplication|Tomcat started|ERROR|Exception" || docker logs --tail 15 azheimer-backend

echo ""
echo "🧪 Quick test:"
curl -s http://localhost:8080/swagger-ui.html > /dev/null && echo "✅ Swagger UI is responding" || echo "⚠️ Swagger UI not responding yet"
ENDSSH

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Deployment failed"
    echo ""
    echo "🔍 Troubleshooting:"
    echo "   1. Check logs: ssh -i $SSH_KEY $VM_USER@$VM_HOST \"docker logs azheimer-backend\""
    echo "   2. Check container: ssh -i $SSH_KEY $VM_USER@$VM_HOST \"docker ps -a | grep azheimer\""
    echo "   3. Try force rebuild: $0 --force"
    exit 1
fi

echo ""
echo "=========================================="
echo "✅ Deployment successful!"
echo "=========================================="
echo ""
echo "🌐 API URLs:"
echo "   Base:    http://$VM_HOST:8080"
echo "   Swagger: http://$VM_HOST:8080/swagger-ui.html"
echo ""
echo "📊 View logs:"
echo "   ssh -i $SSH_KEY $VM_USER@$VM_HOST \"docker logs -f azheimer-backend\""
echo ""
echo "🧪 Test API:"
echo "   curl http://$VM_HOST:8080/swagger-ui.html"
echo ""
echo "💡 Tip: Use '$0 --force' to force rebuild without cache (slower but safer)"
echo ""
