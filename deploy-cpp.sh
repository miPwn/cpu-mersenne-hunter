#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1" | tee -a deployment.log
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1" | tee -a deployment.log
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1" | tee -a deployment.log
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" | tee -a deployment.log
}

# Create deployment log
echo "=== C++ Deployment Started: $(date) ===" > deployment.log

log_info "🏗️  C++ Mersenne Prime Calculator Deployment"
log_info "============================================="

# Step 1: Environment verification
log_info "📋 Verifying build environment..."

# Check compilers
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    log_success "G++ found: $GCC_VERSION"
else
    log_error "G++ not found!"
    exit 1
fi

if command -v make &> /dev/null; then
    MAKE_VERSION=$(make --version | head -n1)
    log_success "Make found: $MAKE_VERSION"
else
    log_error "Make not found!"
    exit 1
fi

# Check required libraries
log_info "🔍 Checking required libraries..."

check_library() {
    local lib_name="$1"
    local header_file="$2"
    
    if echo "#include <$header_file>" | g++ -x c++ -c - -o /dev/null 2>/dev/null; then
        log_success "$lib_name development headers found"
        return 0
    else
        log_warning "$lib_name development headers not found, trying pkg-config..."
        if pkg-config --exists "$lib_name" 2>/dev/null; then
            log_success "$lib_name found via pkg-config"
            return 0
        else
            log_error "$lib_name not available"
            return 1
        fi
    fi
}

# Check GMP
if check_library "gmp" "gmp.h"; then
    GMP_VERSION=$(pkg-config --modversion gmp 2>/dev/null || echo "unknown")
    log_success "GMP version: $GMP_VERSION"
fi

# Check FFTW
if check_library "fftw3" "fftw3.h"; then
    FFTW_VERSION=$(pkg-config --modversion fftw3 2>/dev/null || echo "unknown")
    log_success "FFTW3 version: $FFTW_VERSION"
fi

# Step 2: Create necessary directories
log_info "📁 Creating build directories..."
mkdir -p bin obj logs dist
log_success "Directories created: bin/, obj/, logs/, dist/"

# Step 3: Clean previous builds
log_info "🧹 Cleaning previous builds..."
if make clean 2>&1 | tee -a deployment.log; then
    log_success "Clean completed"
else
    log_warning "Clean had issues, continuing..."
fi

# Step 4: Build C++ application with detailed logging
log_info "🔨 Building C++ application..."
echo "=== Build Output ===" >> deployment.log

if make all VERBOSE=1 2>&1 | tee -a deployment.log; then
    log_success "C++ build completed successfully"
    
    # Verify binary exists and is executable
    if [ -f "bin/mersenne_prime" ]; then
        chmod +x bin/mersenne_prime
        
        # Test the binary
        log_info "🧪 Testing compiled binary..."
        if timeout 10s ./bin/mersenne_prime -p 127 -v > test_output.log 2>&1; then
            log_success "Binary test completed successfully"
            cat test_output.log >> deployment.log
        else
            log_warning "Binary test timed out or failed, but binary exists"
            if [ -f test_output.log ]; then
                cat test_output.log >> deployment.log
            fi
        fi
        
        # Get binary info
        BINARY_SIZE=$(du -h bin/mersenne_prime | cut -f1)
        log_success "Binary size: $BINARY_SIZE"
        
        # Check dependencies
        log_info "📚 Checking binary dependencies..."
        if command -v ldd &> /dev/null; then
            echo "=== Binary Dependencies ===" >> deployment.log
            ldd bin/mersenne_prime 2>&1 | tee -a deployment.log
        fi
        
    else
        log_error "Binary not found after successful build!"
        exit 1
    fi
else
    log_error "C++ build failed!"
    echo "=== Build Failure Details ===" >> deployment.log
    
    # Try to get more details about the failure
    log_info "🔍 Attempting detailed diagnosis..."
    
    # Check if Makefile exists
    if [ ! -f "Makefile" ]; then
        log_error "Makefile not found in current directory"
        ls -la >> deployment.log
    fi
    
    # Check source files
    if [ -d "src" ]; then
        log_info "Source files found:"
        ls -la src/ >> deployment.log
    else
        log_error "src/ directory not found"
    fi
    
    # Try a simple test compilation
    log_info "🧪 Testing simple compilation..."
    if [ -f "src/main.cpp" ]; then
        echo "=== Simple Compilation Test ===" >> deployment.log
        g++ -v src/main.cpp -o test_binary 2>&1 | tee -a deployment.log || true
    fi
    
    exit 1
fi

# Step 5: Create deployment info
log_info "📝 Creating deployment information..."
cat > deployment_info.json << EOF
{
  "timestamp": "$(date -Iseconds)",
  "status": "success",
  "compiler": "$GCC_VERSION",
  "binary_size": "$BINARY_SIZE",
  "build_time": "$(date)",
  "environment": {
    "gcc_version": "$(g++ --version | head -n1)",
    "make_version": "$(make --version | head -n1)",
    "gmp_available": $(check_library "gmp" "gmp.h" && echo "true" || echo "false"),
    "fftw_available": $(check_library "fftw3" "fftw3.h" && echo "true" || echo "false")
  }
}
EOF

# Step 6: Create run script for easy execution
log_info "🚀 Creating run script..."
cat > run-mersenne.sh << 'EOF'
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
EOF

chmod +x run-mersenne.sh

# Step 7: Create health check endpoint
log_info "💓 Creating health check endpoint..."
cat > health-check.sh << 'EOF'
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
EOF

chmod +x health-check.sh

# Final summary
log_success "🎉 C++ deployment completed successfully!"
echo ""
log_info "📋 Deployment Summary:"
log_info "- Binary: bin/mersenne_prime (executable)"
log_info "- Run script: ./run-mersenne.sh"
log_info "- Health check: ./health-check.sh"
log_info "- Full logs: deployment.log"
log_info "- Build info: deployment_info.json"
echo ""
log_info "🚀 Ready for production deployment!"

echo "=== Deployment Completed: $(date) ===" >> deployment.log