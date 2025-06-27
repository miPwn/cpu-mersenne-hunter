const express = require('express');
const path = require('path');
const { spawn } = require('child_process');

const app = express();
const PORT = process.env.PORT || 5000;

// Serve static files from dist
app.use(express.static(path.join(__dirname, 'dist')));

// Health check endpoint
app.get('/health', (req, res) => {
  res.json({
    status: 'healthy',
    timestamp: new Date().toISOString(),
    service: 'Mersenne Prime Calculator'
  });
});

// API endpoint for calculations
app.get('/api/calculate/:exponent', (req, res) => {
  const exponent = parseInt(req.params.exponent);
  
  if (!exponent || exponent < 2) {
    return res.status(400).json({ error: 'Invalid exponent' });
  }
  
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
    res.json({
      exponent,
      output: output.trim(),
      error: error.trim(),
      duration,
      timestamp: new Date().toISOString(),
      success: code === 0
    });
  });
});

// Serve React app for all other routes
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`Server running on port ${PORT}`);
  console.log(`Local: http://localhost:${PORT}`);
  console.log(`Public: https://mersenne-hunter-richardpashley.replit.app/`);
});