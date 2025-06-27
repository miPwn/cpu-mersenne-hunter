import React, { useState } from 'react'
import { Play, Pause, Square, Settings, Loader2 } from 'lucide-react'

function CalculatorControls({ onCalculate }) {
  const [exponent, setExponent] = useState('127')
  const [isRunning, setIsRunning] = useState(false)
  const [isPaused, setIsPaused] = useState(false)
  const [currentJobs, setCurrentJobs] = useState([])
  const [maxThreads, setMaxThreads] = useState('8')
  const [batchMode, setBatchMode] = useState(false)
  const [rangeStart, setRangeStart] = useState('127')
  const [rangeEnd, setRangeEnd] = useState('521')

  const handleRun = async () => {
    if (isRunning) return

    setIsRunning(true)
    setIsPaused(false)

    try {
      if (batchMode) {
        const start = parseInt(rangeStart)
        const end = parseInt(rangeEnd)
        const jobs = []
        
        for (let exp = start; exp <= end; exp++) {
          if (isPrime(exp)) { // Only test prime exponents for Mersenne primes
            jobs.push(exp)
          }
        }
        
        setCurrentJobs(jobs)
        
        for (let i = 0; i < jobs.length && !isPaused; i++) {
          await calculateSingle(jobs[i])
          if (isPaused) break
        }
      } else {
        await calculateSingle(parseInt(exponent))
      }
    } finally {
      setIsRunning(false)
      setCurrentJobs([])
    }
  }

  const calculateSingle = async (exp) => {
    try {
      const response = await fetch(`/api/calculate/${exp}`)
      const result = await response.json()
      
      if (onCalculate) {
        onCalculate({
          ...result,
          isPrime: result.output.includes('is PRIME'),
          threads: parseInt(maxThreads),
          iterations: extractIterations(result.output)
        })
      }
    } catch (error) {
      console.error('Calculation error:', error)
    }
  }

  const handlePause = () => {
    setIsPaused(true)
  }

  const handleStop = () => {
    setIsRunning(false)
    setIsPaused(false)
    setCurrentJobs([])
  }

  const isPrime = (n) => {
    if (n < 2) return false
    if (n === 2) return true
    if (n % 2 === 0) return false
    for (let i = 3; i * i <= n; i += 2) {
      if (n % i === 0) return false
    }
    return true
  }

  const extractIterations = (output) => {
    const match = output.match(/Iterations completed: (\d+)/)
    return match ? parseInt(match[1]) : null
  }

  const presetExponents = [127, 521, 607, 1279, 2203, 2281, 3217, 4253, 4423, 9689]

  return (
    <div className="bg-white rounded-lg shadow p-6 space-y-6">
      <div className="flex items-center justify-between">
        <h3 className="text-lg font-semibold text-gray-900">Calculator Controls</h3>
        <div className="flex items-center space-x-2">
          {isRunning && (
            <div className="flex items-center space-x-2 text-sm text-blue-600">
              <Loader2 className="h-4 w-4 animate-spin" />
              <span>Running...</span>
            </div>
          )}
          {isPaused && (
            <div className="flex items-center space-x-2 text-sm text-orange-600">
              <Pause className="h-4 w-4" />
              <span>Paused</span>
            </div>
          )}
        </div>
      </div>

      {/* Mode Selection */}
      <div className="flex space-x-4">
        <label className="flex items-center">
          <input
            type="radio"
            checked={!batchMode}
            onChange={() => setBatchMode(false)}
            className="mr-2"
          />
          <span className="text-sm text-gray-700">Single Test</span>
        </label>
        <label className="flex items-center">
          <input
            type="radio"
            checked={batchMode}
            onChange={() => setBatchMode(true)}
            className="mr-2"
          />
          <span className="text-sm text-gray-700">Batch Range</span>
        </label>
      </div>

      {/* Input Fields */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        {!batchMode ? (
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-1">
              Exponent
            </label>
            <input
              type="number"
              value={exponent}
              onChange={(e) => setExponent(e.target.value)}
              disabled={isRunning}
              className="w-full rounded-md border border-gray-300 px-3 py-2 text-sm focus:border-blue-500 focus:ring-blue-500 disabled:opacity-50"
              placeholder="Enter prime exponent (e.g., 127)"
            />
          </div>
        ) : (
          <>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-1">
                Range Start
              </label>
              <input
                type="number"
                value={rangeStart}
                onChange={(e) => setRangeStart(e.target.value)}
                disabled={isRunning}
                className="w-full rounded-md border border-gray-300 px-3 py-2 text-sm focus:border-blue-500 focus:ring-blue-500 disabled:opacity-50"
              />
            </div>
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-1">
                Range End
              </label>
              <input
                type="number"
                value={rangeEnd}
                onChange={(e) => setRangeEnd(e.target.value)}
                disabled={isRunning}
                className="w-full rounded-md border border-gray-300 px-3 py-2 text-sm focus:border-blue-500 focus:ring-blue-500 disabled:opacity-50"
              />
            </div>
          </>
        )}

        <div>
          <label className="block text-sm font-medium text-gray-700 mb-1">
            Max Threads
          </label>
          <select
            value={maxThreads}
            onChange={(e) => setMaxThreads(e.target.value)}
            disabled={isRunning}
            className="w-full rounded-md border border-gray-300 px-3 py-2 text-sm focus:border-blue-500 focus:ring-blue-500 disabled:opacity-50"
          >
            <option value="1">1 Thread</option>
            <option value="2">2 Threads</option>
            <option value="4">4 Threads</option>
            <option value="8">8 Threads</option>
            <option value="16">16 Threads</option>
          </select>
        </div>
      </div>

      {/* Preset Buttons */}
      {!batchMode && (
        <div>
          <label className="block text-sm font-medium text-gray-700 mb-2">
            Quick Presets
          </label>
          <div className="grid grid-cols-5 gap-2">
            {presetExponents.map(exp => (
              <button
                key={exp}
                onClick={() => setExponent(exp.toString())}
                disabled={isRunning}
                className="px-3 py-1 text-xs border border-gray-300 rounded hover:bg-gray-50 disabled:opacity-50"
              >
                M{exp}
              </button>
            ))}
          </div>
        </div>
      )}

      {/* Control Buttons */}
      <div className="flex space-x-3">
        <button
          onClick={handleRun}
          disabled={isRunning}
          className="flex items-center space-x-2 px-4 py-2 bg-green-600 text-white rounded-md hover:bg-green-700 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <Play className="h-4 w-4" />
          <span>Run</span>
        </button>

        <button
          onClick={handlePause}
          disabled={!isRunning || isPaused}
          className="flex items-center space-x-2 px-4 py-2 bg-orange-600 text-white rounded-md hover:bg-orange-700 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <Pause className="h-4 w-4" />
          <span>Pause</span>
        </button>

        <button
          onClick={handleStop}
          disabled={!isRunning && !isPaused}
          className="flex items-center space-x-2 px-4 py-2 bg-red-600 text-white rounded-md hover:bg-red-700 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <Square className="h-4 w-4" />
          <span>Stop</span>
        </button>
      </div>

      {/* Progress Information */}
      {(isRunning || isPaused) && currentJobs.length > 0 && (
        <div className="bg-blue-50 rounded-md p-4">
          <div className="flex items-center justify-between text-sm">
            <span className="text-blue-800">Batch Progress</span>
            <span className="text-blue-600">
              {currentJobs.length} tests queued
            </span>
          </div>
          <div className="mt-2">
            <div className="text-xs text-blue-600">
              Testing: {currentJobs.slice(0, 5).map(exp => `M${exp}`).join(', ')}
              {currentJobs.length > 5 && ` +${currentJobs.length - 5} more`}
            </div>
          </div>
        </div>
      )}
    </div>
  )
}

export default CalculatorControls