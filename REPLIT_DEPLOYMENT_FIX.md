# 🔧 Fix "Could not find run command" Error

## The Problem
Your .replit file is missing the essential `run` command that Replit needs for deployment. This causes the "Could not find run command" error.

## ✅ The Solution

### 1. Replace Your .replit File
Copy the contents from `replit-config.toml` to your `.replit` file:

```toml
modules = ["cpp", "nix"]

[nix]
channel = "stable-24_05"

# 🔑 This fixes "Could not find run command"
run = "./deploy-and-run.sh"

entrypoint = "deploy-and-run.sh"

[deployment]
deploymentTarget = "cloudrun"
build = ["make", "clean", "&&", "make", "all"]
run = ["./bin/mersenne_prime", "-p", "127", "-v"]

[[ports]]
localPort = 5000
externalPort = 80
exposeLocalhost = true

[languages.cpp]
pattern = "**/*.{cpp,hpp,c,h,cc,cxx}"
syntax = "cpp"

[env]
CC = "gcc"
CXX = "g++"
```

### 2. Use the Universal Launcher Script
The `deploy-and-run.sh` script handles all deployment scenarios:

**Pure C++ Application:**
- Builds the binary if needed
- Runs comprehensive tests
- Starts health check server on port 5000

**Hybrid Application (React + C++):**
- Builds C++ backend
- Serves React frontend from dist/
- Provides API endpoints for C++ integration

**With Node.js Backend:**
- Integrates with existing Express server
- Maintains WebSocket connections
- Serves static files

### 3. Required Dependencies
Ensure your environment has these packages (they're already installed):
- gcc13 (C++ compiler)
- gmp (arbitrary precision arithmetic)
- fftw (Fast Fourier Transform)
- cmake, binutils, gdb, valgrind

## 🚀 Deployment Steps

### Option A: Manual Fix
1. Copy `replit-config.toml` contents to `.replit`
2. Click "Deploy" in Replit
3. The launcher script will handle building and running

### Option B: Quick Test
Run the deployment script directly:
```bash
./deploy-and-run.sh
```

### Option C: Hybrid Deployment
If you have both React and C++:
```bash
# Build frontend
npm run build

# Deploy everything
./deploy-and-run.sh
```

## 🔍 Troubleshooting

### "Build failed" errors:
```bash
cat deployment-run.log
```

### Binary not working:
```bash
./bin/mersenne_prime -p 127 -v
```

### Port issues:
The script automatically detects and runs on port 5000 (mapped to port 80 externally)

### Health check:
Visit `/health` endpoint to verify deployment status

## 🎯 What This Fixes

1. **"Could not find run command"** - Fixed with proper `run` directive
2. **Silent build failures** - Complete logging to `deployment-run.log`
3. **Binary persistence** - Proper build and execution flow
4. **Hybrid deployments** - Automatic detection of React + C++ setup
5. **Health monitoring** - Built-in health check endpoints

## 🔄 For Your Current Project

Your Mersenne Prime Calculator will:
1. Build automatically on deployment
2. Run performance tests (M127, M521)
3. Start a web server with health monitoring
4. Serve a simple web interface showing status
5. Provide `/health` endpoint for monitoring

Ready to deploy - just update your `.replit` file with the configuration from `replit-config.toml`!