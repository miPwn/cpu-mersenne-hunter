#!/bin/bash
# Production start script for Replit deployment

echo "🚀 Starting Mersenne Prime Calculator in production mode..."

# Ensure binary exists
if [ ! -f "bin/mersenne_prime" ]; then
    echo "Building application..."
    make clean && make all
fi

# Start the production server
exec ./deploy-and-run.sh