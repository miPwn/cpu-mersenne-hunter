const express = require('express');
const path = require('path');
const { spawn } = require('child_process');

const app = express();
const PORT = process.env.PORT || 5000;

console.log('🚀 Starting Mersenne Prime Calculator Production Server');
console.log(`📍 Port: ${PORT}`);
console.log(`🌐 Expected Public URL: https://mersenne-hunter-richardpashley.replit.app/`);

// Serve static files from dist
app.use(express.static(path.join(__dirname, 'dist')));

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({
    status: 'healthy',
    timestamp: new Date().toISOString(),
    service: 'Mersenne Prime Calculator',
    port: PORT,
    environment: 'production'
  });
});

// Root endpoint with status info
app.get('/', (req, res) => {
  // Check if we have the built React app
  const indexPath = path.join(__dirname, 'dist', 'index.html');
  const fs = require('fs');
  
  if (fs.existsSync(indexPath)) {
    res.sendFile(indexPath);
  } else {
    // Fallback HTML if React build not found
    res.send(`
      <!DOCTYPE html>
      <html>
      <head>
        <title>Mersenne Prime Calculator</title>
        <style>
          body { font-family: Arial, sans-serif; margin: 40px; background: #f5f5f5; }
          .container { background: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
          .status { background: #e8f5e8; padding: 15px; border-radius: 4px; margin: 20px 0; }
          .test-link { display: inline-block; background: #007acc; color: white; padding: 10px 20px; text-decoration: none; border-radius: 4px; margin: 10px 5px 0 0; }
          .test-link:hover { background: #005a9a; }
          pre { background: #f8f8f8; padding: 15px; border-radius: 4px; overflow-x: auto; }
        </style>
      </head>
      <body>
        <div class="container">
          <h1>🔢 Mersenne Prime Calculator</h1>
          <p>High-performance C++ application for computing Mersenne primes</p>
          
          <div class="status">
            <strong>✅ Server Status: Running</strong><br>
            <strong>📅 Started:</strong> ${new Date().toISOString()}<br>
            <strong>🌐 Port:</strong> ${PORT}<br>
            <strong>🖥️ Environment:</strong> Production
          </div>
          
          <h3>Available Endpoints:</h3>
          <a href="/health" class="test-link">Health Check</a>
          <a href="/api/calculate/127" class="test-link">Test M127</a>
          
          <h3>Quick Test:</h3>
          <pre id="healthResult">Loading health status...</pre>
          
          <script>
            fetch('/health')
              .then(r => r.json())
              .then(data => {
                document.getElementById('healthResult').textContent = JSON.stringify(data, null, 2);
              })
              .catch(err => {
                document.getElementById('healthResult').textContent = 'Error: ' + err.message;
              });
          </script>
        </div>
      </body>
      </html>
    `);
  }
});

// API endpoint for calculations
app.get('/api/calculate/:exponent', (req, res) => {
  const exponent = parseInt(req.params.exponent);
  
  if (!exponent || exponent < 2) {
    return res.status(400).json({ error: 'Invalid exponent' });
  }
  
  console.log(`🧮 Computing M${exponent}`);
  const startTime = Date.now();
  const process = spawn('./bin/mersenne_prime', ['-p', exponent.toString(), '-v']);
  
  let output = '';
  let error = '';
  
  process.stdout.on('data', (data) => {
    output += data.toString();
  });
  
  process.stderr.on('data', (data) => {
    error += data.toString();
  });
  
  process.on('close', (code) => {
    const duration = Date.now() - startTime;
    const result = {
      exponent,
      output: output.trim(),
      error: error.trim(),
      duration,
      timestamp: new Date().toISOString(),
      success: code === 0
    };
    
    console.log(`✅ M${exponent} completed in ${duration}ms`);
    res.json(result);
  });
});

// Catch-all for React routing
app.get('*', (req, res) => {
  const indexPath = path.join(__dirname, 'dist', 'index.html');
  const fs = require('fs');
  
  if (fs.existsSync(indexPath)) {
    res.sendFile(indexPath);
  } else {
    res.redirect('/');
  }
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`✅ Server running on http://0.0.0.0:${PORT}`);
  console.log(`🔗 Local: http://localhost:${PORT}`);
  console.log(`🌍 Public: https://mersenne-hunter-richardpashley.replit.app/`);
  console.log(`🩺 Health: https://mersenne-hunter-richardpashley.replit.app/health`);
});