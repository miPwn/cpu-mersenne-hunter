#!/usr/bin/env node

const express = require('express');
const { createServer } = require('http');
const { WebSocketServer } = require('ws');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

const app = express();
const server = createServer(app);
const wss = new WebSocketServer({ server, path: '/ws' });

const PORT = process.env.PORT || 5000;
const clients = new Set();

// Serve static files from dist directory
app.use(express.static('dist'));

// WebSocket handling for real-time updates
wss.on('connection', (ws) => {
  console.log('Client connected to WebSocket');
  clients.add(ws);
  
  ws.on('close', () => {
    console.log('Client disconnected');
    clients.delete(ws);
  });
  
  ws.on('error', (error) => {
    console.error('WebSocket error:', error);
    clients.delete(ws);
  });
});

// Broadcast function
function broadcast(message) {
  const data = JSON.stringify(message);
  clients.forEach(client => {
    if (client.readyState === 1) { // WebSocket.OPEN
      try {
        client.send(data);
      } catch (error) {
        console.error('Error sending to client:', error);
        clients.delete(client);
      }
    }
  });
}

// Health check endpoint
app.get('/health', (req, res) => {
  const healthStatus = {
    status: 'healthy',
    binary: 'bin/mersenne_prime',
    timestamp: new Date().toISOString(),
    version: '1.0.0',
    server: 'Node.js Production Server'
  };
  res.json(healthStatus);
});

// API endpoint to run Mersenne prime calculations
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
    const endTime = Date.now();
    const duration = endTime - startTime;
    
    const result = {
      exponent,
      output: output.trim(),
      error: error.trim(),
      duration,
      timestamp: new Date().toISOString(),
      success: code === 0
    };
    
    // Broadcast result to all connected WebSocket clients
    broadcast({
      type: 'result',
      data: result
    });
    
    res.json(result);
  });
});

// Serve React app for all other routes
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

// Start server
server.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Mersenne Prime Calculator running on http://0.0.0.0:${PORT}`);
  console.log(`🔗 Public URL: https://mersenne-hunter-richardpashley.replit.app/`);
  console.log(`📊 Health Check: https://mersenne-hunter-richardpashley.replit.app/health`);
  console.log(`🧮 Calculator UI: https://mersenne-hunter-richardpashley.replit.app/`);
});