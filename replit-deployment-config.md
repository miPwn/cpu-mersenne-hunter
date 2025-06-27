# Fix for "Could not find run command" Error

## The Solution: Update Your .replit File

Replace your current `.replit` file with this configuration:

```toml
modules = ["cpp", "nodejs-20"]

[nix]
channel = "stable-24_05"

# This is the key fix for "Could not find run command"
run = "./production-start.sh"

# Optional: Set entry point
entrypoint = "production-start.sh"

# Deployment configuration
[deployment]
deploymentTarget = "cloudrun"
run = ["./production-start.sh"]

# Port configuration
[[ports]]
localPort = 5000
externalPort = 80

# Language support
[languages.cpp]
pattern = "**/*.{cpp,hpp,c,h}"
syntax = "cpp"

[env]
PORT = "5000"
```

## Manual Steps to Fix Deployment

### Step 1: Copy the Config
Since the automated tools can't edit .replit directly:

1. Open your `.replit` file in the editor
2. Replace the contents with the configuration above
3. Save the file

### Step 2: Test the Configuration
Run this command to verify everything works:
```bash
./production-start.sh
```

### Step 3: Deploy
Click the "Deploy" button in Replit - it should now work without the "Could not find run command" error.

## What This Fixes

1. **Missing run command**: Added `run = "./production-start.sh"`
2. **Deployment target**: Properly configured for CloudRun
3. **Port mapping**: Maps internal port 5000 to external port 80
4. **Environment**: Sets up proper PORT variable

## Alternative: Direct Command

If you prefer to deploy directly without the deploy button:
```bash
# This is what's currently running and working
./deploy-and-run.sh
```

Your application is already running successfully on the current workflow. The deploy button issue is just a configuration problem in the .replit file.