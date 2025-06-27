import express from 'express'
import { createServer } from 'http'
import { WebSocketServer } from 'ws'
import chokidar from 'chokidar'
import fs from 'fs'
import path from 'path'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

const app = express()
const server = createServer(app)
const wss = new WebSocketServer({ server, path: '/ws' })

const PORT = 3001
const LOG_DIR = path.join(__dirname, '..', 'logs')
const RESULTS_FILE = path.join(LOG_DIR, 'results.ndjson')
const LOGS_FILE = path.join(LOG_DIR, 'application.log')

// Ensure log directory exists
if (!fs.existsSync(LOG_DIR)) {
  fs.mkdirSync(LOG_DIR, { recursive: true })
}

// Store connected clients
const clients = new Set()

// WebSocket connection handling
wss.on('connection', (ws) => {
  console.log('Client connected')
  clients.add(ws)

  // Send existing data to new client
  sendExistingData(ws)

  ws.on('close', () => {
    console.log('Client disconnected')
    clients.delete(ws)
  })

  ws.on('error', (error) => {
    console.error('WebSocket error:', error)
    clients.delete(ws)
  })
})

// Broadcast to all connected clients
function broadcast(message) {
  const data = JSON.stringify(message)
  clients.forEach(client => {
    if (client.readyState === 1) { // WebSocket.OPEN
      try {
        client.send(data)
      } catch (error) {
        console.error('Error sending to client:', error)
        clients.delete(client)
      }
    }
  })
}

// Send existing data to newly connected client
function sendExistingData(ws) {
  try {
    // Send existing results
    if (fs.existsSync(RESULTS_FILE)) {
      const resultsContent = fs.readFileSync(RESULTS_FILE, 'utf8')
      const lines = resultsContent.trim().split('\n').filter(line => line.trim())
      
      lines.forEach(line => {
        try {
          const result = JSON.parse(line)
          ws.send(JSON.stringify({
            type: 'result',
            payload: result
          }))
        } catch (error) {
          console.error('Error parsing result line:', error)
        }
      })
    }

    // Send existing logs
    if (fs.existsSync(LOGS_FILE)) {
      const logsContent = fs.readFileSync(LOGS_FILE, 'utf8')
      const lines = logsContent.trim().split('\n').filter(line => line.trim())
      
      lines.slice(-1000).forEach(line => { // Send last 1000 log lines
        try {
          const log = JSON.parse(line)
          ws.send(JSON.stringify({
            type: 'log',
            payload: log
          }))
        } catch (error) {
          // Handle plain text logs
          ws.send(JSON.stringify({
            type: 'log',
            payload: {
              timestamp: new Date().toISOString(),
              level: 'info',
              message: line
            }
          }))
        }
      })
    }
  } catch (error) {
    console.error('Error sending existing data:', error)
  }
}

// Watch for file changes
const watchFiles = () => {
  // Watch results file
  const resultsWatcher = chokidar.watch(RESULTS_FILE, { 
    ignoreInitial: true,
    persistent: true 
  })

  resultsWatcher.on('change', () => {
    try {
      const content = fs.readFileSync(RESULTS_FILE, 'utf8')
      const lines = content.trim().split('\n').filter(line => line.trim())
      const lastLine = lines[lines.length - 1]
      
      if (lastLine) {
        const result = JSON.parse(lastLine)
        broadcast({
          type: 'result',
          payload: result
        })
      }
    } catch (error) {
      console.error('Error reading results file:', error)
    }
  })

  // Watch logs file
  const logsWatcher = chokidar.watch(LOGS_FILE, { 
    ignoreInitial: true,
    persistent: true 
  })

  logsWatcher.on('change', () => {
    try {
      const content = fs.readFileSync(LOGS_FILE, 'utf8')
      const lines = content.trim().split('\n').filter(line => line.trim())
      const lastLine = lines[lines.length - 1]
      
      if (lastLine) {
        try {
          const log = JSON.parse(lastLine)
          broadcast({
            type: 'log',
            payload: log
          })
        } catch (error) {
          // Handle plain text logs
          broadcast({
            type: 'log',
            payload: {
              timestamp: new Date().toISOString(),
              level: 'info',
              message: lastLine
            }
          })
        }
      }
    } catch (error) {
      console.error('Error reading logs file:', error)
    }
  })

  console.log('File watchers initialized')
}

// API endpoints
app.use(express.json())

app.get('/api/status', (req, res) => {
  res.json({
    status: 'running',
    clients: clients.size,
    files: {
      results: fs.existsSync(RESULTS_FILE),
      logs: fs.existsSync(LOGS_FILE)
    }
  })
})

app.get('/api/results', (req, res) => {
  try {
    if (!fs.existsSync(RESULTS_FILE)) {
      return res.json([])
    }

    const content = fs.readFileSync(RESULTS_FILE, 'utf8')
    const lines = content.trim().split('\n').filter(line => line.trim())
    const results = lines.map(line => JSON.parse(line))
    
    res.json(results)
  } catch (error) {
    console.error('Error reading results:', error)
    res.status(500).json({ error: 'Failed to read results' })
  }
})

app.get('/api/logs', (req, res) => {
  try {
    if (!fs.existsSync(LOGS_FILE)) {
      return res.json([])
    }

    const content = fs.readFileSync(LOGS_FILE, 'utf8')
    const lines = content.trim().split('\n').filter(line => line.trim())
    const logs = lines.map(line => {
      try {
        return JSON.parse(line)
      } catch (error) {
        return {
          timestamp: new Date().toISOString(),
          level: 'info',
          message: line
        }
      }
    })
    
    res.json(logs)
  } catch (error) {
    console.error('Error reading logs:', error)
    res.status(500).json({ error: 'Failed to read logs' })
  }
})

// Start server
server.listen(PORT, '0.0.0.0', () => {
  console.log(`Server running on port ${PORT}`)
  console.log(`WebSocket endpoint: ws://localhost:${PORT}/ws`)
  console.log(`Watching for files in: ${LOG_DIR}`)
  
  // Initialize file watchers
  watchFiles()
})

// Graceful shutdown
process.on('SIGINT', () => {
  console.log('\nShutting down server...')
  server.close(() => {
    console.log('Server closed')
    process.exit(0)
  })
})