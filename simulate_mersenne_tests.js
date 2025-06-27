const fs = require('fs')
const path = require('path')

// Sample test data to simulate C++ application output
const testCases = [
  { exponent: 127, isPrime: true, baseTime: 0.17 },
  { exponent: 521, isPrime: true, baseTime: 0.51 },
  { exponent: 607, isPrime: true, baseTime: 0.64 },
  { exponent: 1279, isPrime: true, baseTime: 1.52 },
  { exponent: 2203, isPrime: true, baseTime: 3.15 },
  { exponent: 3217, isPrime: true, baseTime: 6.10 },
  { exponent: 4253, isPrime: true, baseTime: 12.3 },
  { exponent: 89, isPrime: false, baseTime: 0.05 },
  { exponent: 97, isPrime: false, baseTime: 0.06 },
  { exponent: 113, isPrime: false, baseTime: 0.08 }
]

const logDir = path.join(__dirname, 'logs')
const resultsFile = path.join(logDir, 'results.ndjson')
const logsFile = path.join(logDir, 'application.log')

// Ensure log directory exists
if (!fs.existsSync(logDir)) {
  fs.mkdirSync(logDir, { recursive: true })
}

function getRandomVariation(baseValue, variationPercent = 10) {
  const variation = (Math.random() - 0.5) * 2 * (variationPercent / 100)
  return baseValue * (1 + variation)
}

function generateResult(testCase) {
  const duration = getRandomVariation(testCase.baseTime)
  const threads = Math.floor(Math.random() * 8) + 1
  const iterations = testCase.isPrime ? testCase.exponent - 2 : Math.floor(Math.random() * (testCase.exponent - 2)) + 1
  
  return {
    timestamp: new Date().toISOString(),
    exponent: testCase.exponent,
    isPrime: testCase.isPrime,
    duration: Math.round(duration * 100) / 100, // Round to 2 decimal places
    threads: threads,
    iterations: iterations,
    fftOps: Math.floor(Math.random() * 10),
    status: testCase.isPrime ? "PRIME" : "COMPOSITE"
  }
}

function generateLog(message, level = 'info', exponent = null) {
  const log = {
    timestamp: new Date().toISOString(),
    level: level,
    message: message
  }
  
  if (exponent !== null) {
    log.exponent = exponent
  }
  
  return log
}

function appendToFile(filename, data) {
  fs.appendFileSync(filename, JSON.stringify(data) + '\n')
}

async function simulateTest(testCase) {
  console.log(`Testing M${testCase.exponent}...`)
  
  // Log start
  appendToFile(logsFile, generateLog(`Starting Mersenne prime test`, 'info', testCase.exponent))
  
  // Simulate computation delay
  await new Promise(resolve => setTimeout(resolve, 500))
  
  // Log progress
  appendToFile(logsFile, generateLog(`Lucas-Lehmer test in progress for M${testCase.exponent}`, 'debug'))
  
  // Generate and log result
  const result = generateResult(testCase)
  appendToFile(resultsFile, result)
  
  // Log completion
  appendToFile(logsFile, generateLog(`Test completed: ${result.status}`, 'info', testCase.exponent))
  
  console.log(`M${testCase.exponent}: ${result.status} (${result.duration}ms)`)
}

async function runSimulation() {
  console.log('Starting Mersenne Prime Dashboard Simulation')
  console.log('==========================================')
  
  appendToFile(logsFile, generateLog('Starting automated benchmark suite', 'info'))
  
  // Run tests with delays to demonstrate real-time updates
  for (const testCase of testCases) {
    await simulateTest(testCase)
    await new Promise(resolve => setTimeout(resolve, 1000)) // 1 second between tests
  }
  
  appendToFile(logsFile, generateLog('All benchmark tests completed successfully', 'info'))
  console.log('\nSimulation completed! Check the dashboard for real-time updates.')
}

// Run simulation if called directly
if (require.main === module) {
  runSimulation().catch(console.error)
}

module.exports = { runSimulation }