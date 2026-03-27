# SSH Key Setup

## Quick Setup

1. **Place key in project root:**
   ```bash
   cp /path/to/your/server-key.pem ./server-key.pem
   chmod 600 server-key.pem
   ```

2. **Test connection:**
   ```bash
   ssh -i ./server-key.pem ec2-user@ec2-16-16-142-88.eu-north-1.compute.amazonaws.com "echo 'Connected!'"
   ```

3. **Deploy:**
   ```bash
   ./deploy.sh --force
   ```

---

## Security

- ✅ Key is gitignored (won't be committed)
- ✅ Permissions set to `600` (owner read/write only)
- ⚠️ **Never share this key publicly**
- ⚠️ **Never commit to git**

---

## VM Details

- **Host:** ec2-16-16-142-88.eu-north-1.compute.amazonaws.com
- **User:** ec2-user
- **Key:** ./server-key.pem
- **Region:** EU North 1 (Stockholm)

---

## Don't Have the Key?

Get it from:
- AWS EC2 console (if you have access)
- Team member who has it
- Person who created the EC2 instance
