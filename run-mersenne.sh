#!/bin/bash
echo "🔢 Mersenne Prime Calculator"
echo "=========================="

if [ ! -f "bin/mersenne_prime" ]; then
    echo "❌ Binary not found! Please run deployment first."
    exit 1
fi

# Default test cases
if [ $# -eq 0 ]; then
    echo "🧪 Running default test cases..."
    ./bin/mersenne_prime -p 127 -v
    ./bin/mersenne_prime -p 521 -v
else
    echo "🎯 Running with custom arguments: $@"
    ./bin/mersenne_prime "$@"
fi
