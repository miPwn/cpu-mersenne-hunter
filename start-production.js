const express = require('express');
const path = require('path');
const fs = require('fs');
const { WebSocketServer } = require('ws');
const chokidar = require('chokidar');

const app = express();
const PORT = process.env.PORT || 5000;

// Serve static files from dist directory
app.use(express.static(path.join(__dirname, 'dist')));

// Serve any other routes with index.html (SPA fallback)
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'dist', 'index.html'));
});

const server = app.listen(PORT, '0.0.0.0', () => {
  console.log(`🚀 Production server running on http://0.0.0.0:${PORT}`);
});

// WebSocket server for real-time data (if needed)
const wss = new WebSocketServer({ server, path: '/ws' });

let clients = new Set();

wss.on('connection', (ws) => {
  console.log('Client connected to production WebSocket');
  clients.add(ws);
  
  ws.on('close', () => {
    clients.delete(ws);
  });
});

// Watch for C++ output files and broadcast updates
const logsDir = path.join(__dirname, 'logs');
if (fs.existsSync(logsDir)) {
  const watcher = chokidar.watch(logsDir);
  
  watcher.on('change', (filepath) => {
    console.log(`File changed: ${filepath}`);
    // Broadcast to all connected clients
    clients.forEach(client => {
      if (client.readyState === 1) { // WebSocket.OPEN
        client.send(JSON.stringify({ type: 'file_changed', file: filepath }));
      }
    });
  });
}

process.on('SIGTERM', () => {
  console.log('Shutting down production server...');
  server.close();
});
