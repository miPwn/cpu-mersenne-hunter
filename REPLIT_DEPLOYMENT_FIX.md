# Fix for 404 Error on Public Replit URL

## The Problem
Your Mersenne Prime Calculator is running perfectly on the internal server (localhost:5000) but the public URL `https://mersenne-hunter-richardpashley.replit.app/` returns 404. This is because Replit's public routing system isn't configured to use your application as the primary service.

## The Solution

### Step 1: Update Your .replit File
You need to manually edit your `.replit` file to add the missing `run` command. Open the `.replit` file in the Replit editor and add this line at the top level (not inside any brackets):

```toml
run = "node main.js"
```

Your `.replit` file should look like this at the beginning:
```toml
modules = ["cpp", "nodejs-20"]
run = "node main.js"

[nix]
channel = "stable-24_05"
packages = ["gcc", "gmp", "fftw", "pkg-config", "gcc13", "cmake", "binutils", "gdb", "valgrind", "python3"]
```

### Step 2: Verify the Configuration
After adding the `run` command:
1. Save the `.replit` file
2. The Replit environment should automatically restart
3. Your application should now be accessible at the public URL

### Step 3: Test the Deployment
Once configured, these URLs should work:
- **Main App**: https://mersenne-hunter-richardpashley.replit.app/
- **Health Check**: https://mersenne-hunter-richardpashley.replit.app/health
- **API Example**: https://mersenne-hunter-richardpashley.replit.app/api/calculate/127

## Current Status
- ✅ Your application is built and running locally
- ✅ C++ Mersenne prime calculator is operational
- ✅ React frontend is built and ready
- ✅ Express server is serving both frontend and API
- ❌ Public URL routing needs the `.replit` fix above

## Alternative: Use Deploy Button
Once you've updated the `.replit` file with the `run` command, you can also use Replit's "Deploy" button, which should now work without the "Could not find run command" error.

## Verification Commands
After the fix, you can test locally:
```bash
curl http://localhost:5000/health
curl http://localhost:5000/
```

Both should return successful responses (they already do locally).

The public URL will work once the `.replit` configuration is updated.