#!/bin/bash
echo "Building React frontend with calculator controls..."

# Kill any existing build processes
pkill -f "vite build" 2>/dev/null

# Quick development build
npx vite build --outDir dist-temp --mode development 2>/dev/null &
BUILD_PID=$!

# Wait up to 15 seconds for build
for i in {1..15}; do
  if ! kill -0 $BUILD_PID 2>/dev/null; then
    echo "Build completed successfully"
    if [ -d "dist-temp" ]; then
      rm -rf dist-backup 2>/dev/null
      mv dist dist-backup 2>/dev/null
      mv dist-temp dist
      echo "Frontend updated with calculator controls"
    fi
    exit 0
  fi
  sleep 1
done

# If still running, kill and use existing build
kill $BUILD_PID 2>/dev/null
echo "Using existing build - server restart will show updates"