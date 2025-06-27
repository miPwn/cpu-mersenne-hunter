#!/bin/bash
set -e

echo "🚀 Mersenne Prime Calculator - Deployment & Launch"
echo "=================================================="

# Ensure we have a deployment log
exec > >(tee -a deployment-run.log) 2>&1

echo "[$(date)] Starting deployment and run process..."

# Step 1: Check if binary exists, build if needed
if [ ! -f "bin/mersenne_prime" ]; then
    echo "🔨 Binary not found, building..."
    
    # Create directories
    mkdir -p bin obj logs
    
    # Clean and build
    if make clean && make all; then
        echo "✅ Build completed successfully"
    else
        echo "❌ Build failed, check logs"
        exit 1
    fi
else
    echo "✅ Binary found: bin/mersenne_prime"
fi

# Step 2: Verify binary is executable
if [ ! -x "bin/mersenne_prime" ]; then
    echo "🔧 Making binary executable..."
    chmod +x bin/mersenne_prime
fi

# Step 3: Test the binary quickly
echo "🧪 Testing binary..."
if timeout 5s ./bin/mersenne_prime -p 127 -v > /dev/null 2>&1; then
    echo "✅ Binary test successful"
else
    echo "⚠️  Binary test failed or timed out, but continuing..."
fi

# Step 4: Check for hybrid deployment (React + C++)
if [ -f "package.json" ] && [ -f "dist/index.html" ]; then
    echo "🌐 Hybrid deployment detected (React + C++)"
    
    # Start the web server
    if [ -f "start-production.js" ]; then
        echo "🖥️  Starting production web server..."
        exec node start-production.js
    else
        echo "🖥️  Starting simple web server..."
        exec node -e "
        const express = require('express');
        const path = require('path');
        const app = express();
        const PORT = process.env.PORT || 5000;
        
        app.use(express.static('dist'));
        app.get('*', (req, res) => res.sendFile(path.join(__dirname, 'dist/index.html')));
        app.get('/health', (req, res) => res.json({status: 'healthy', timestamp: new Date().toISOString()}));
        
        app.listen(PORT, '0.0.0.0', () => {
          console.log(\`🚀 Server running on http://0.0.0.0:\${PORT}\`);
        });
        "
    fi
    
elif [ -f "server.py" ] || [ -f "app.py" ]; then
    echo "🐍 Python web server detected"
    exec python3 server.py || python3 app.py
    
elif [ -f "server.js" ] || [ -f "index.js" ]; then
    echo "🟢 Node.js server detected"
    exec node server.js || node index.js
    
else
    echo "🔢 Pure C++ application mode"
    echo "🏃 Running Mersenne Prime Calculator..."
    
    # For pure C++ deployment, we need to keep the process running
    # Run tests and then start a simple HTTP server for health checks
    echo "Running comprehensive tests..."
    ./bin/mersenne_prime -p 127 -v
    ./bin/mersenne_prime -p 521 -v
    
    echo "🖥️  Starting health check server..."
    # Create a simple Python server for health checks
    exec python3 -c "
import http.server
import socketserver
import json
import subprocess
import os
from datetime import datetime

class HealthHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/health':
            try:
                # Test the binary
                result = subprocess.run(['./bin/mersenne_prime', '-p', '127', '-v'], 
                                      capture_output=True, timeout=5)
                status = 'healthy' if result.returncode == 0 else 'degraded'
            except:
                status = 'unhealthy'
            
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            response = {
                'status': status,
                'binary': 'bin/mersenne_prime',
                'timestamp': datetime.now().isoformat(),
                'version': '1.0.0'
            }
            self.wfile.write(json.dumps(response).encode())
        elif self.path == '/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.end_headers()
            html = '''
            <!DOCTYPE html>
            <html>
            <head><title>Mersenne Prime Calculator</title></head>
            <body>
                <h1>🔢 Mersenne Prime Calculator</h1>
                <p>High-performance C++ application for computing Mersenne primes</p>
                <p><a href=\"/health\">Health Check</a></p>
                <pre id=\"log\"></pre>
                <script>
                fetch('/health').then(r=>r.json()).then(d=>
                    document.getElementById('log').textContent = JSON.stringify(d, null, 2)
                );
                </script>
            </body>
            </html>
            '''
            self.wfile.write(html.encode())
        else:
            super().do_GET()

PORT = int(os.environ.get('PORT', 5000))
# Handle port conflicts gracefully
for port_attempt in [PORT, PORT+1, PORT+2, 8000, 8080]:
    try:
        with socketserver.TCPServer(('0.0.0.0', port_attempt), HealthHandler) as httpd:
            PORT = port_attempt
            break
    except OSError:
        continue
else:
    print('Unable to find available port, exiting...')
    exit(1)

with socketserver.TCPServer(('0.0.0.0', PORT), HealthHandler) as httpd:
    print(f'🚀 Mersenne Prime Calculator server running on http://0.0.0.0:{PORT}')
    print('📊 Health endpoint: /health')
    httpd.serve_forever()
"
fi