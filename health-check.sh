#!/bin/bash
# Health check script for deployment monitoring

echo "Content-Type: application/json"
echo ""

if [ -f "bin/mersenne_prime" ] && [ -x "bin/mersenne_prime" ]; then
    # Try a quick test
    if timeout 5s ./bin/mersenne_prime -p 127 -v > /dev/null 2>&1; then
        echo '{"status": "healthy", "binary": "operational", "timestamp": "'$(date -Iminutes)'"}'
        exit 0
    else
        echo '{"status": "degraded", "binary": "timeout", "timestamp": "'$(date -Iminutes)'"}'
        exit 1
    fi
else
    echo '{"status": "unhealthy", "binary": "missing", "timestamp": "'$(date -Iminutes)'"}'
    exit 1
fi
