import React, { useState } from 'react'
import { motion } from 'framer-motion'
import { Play, Pause, Square, Calculator, Zap } from 'lucide-react'
import { Button } from './ui/Button'
import { Card, CardHeader, CardContent, CardTitle } from './ui/Card'
import { Badge } from './ui/Badge'

export default function CalculatorControls({ onCalculate }) {
  const [exponent, setExponent] = useState('127')
  const [isRunning, setIsRunning] = useState(false)
  const [algorithm, setAlgorithm] = useState('lucas-lehmer')

  const handleStart = async () => {
    if (!exponent || isNaN(exponent)) return
    
    setIsRunning(true)
    
    try {
      const response = await fetch('/api/calculate', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ exponent: parseInt(exponent), algorithm }),
      })
      
      if (response.ok) {
        const result = await response.json()
        if (onCalculate) {
          onCalculate(result)
        }
      }
    } catch (error) {
      console.error('Calculation error:', error)
    }
    
    // Simulate calculation time
    setTimeout(() => {
      setIsRunning(false)
    }, 3000)
  }

  const handleStop = () => {
    setIsRunning(false)
  }

  const presetExponents = [
    { value: '127', label: 'M127', known: true },
    { value: '521', label: 'M521', known: true },
    { value: '607', label: 'M607', known: true },
    { value: '1279', label: 'M1279', known: true },
    { value: '2203', label: 'M2203', known: true },
    { value: '3217', label: 'M3217', known: true },
  ]

  return (
    <Card className="mb-6">
      <CardHeader>
        <CardTitle className="flex items-center space-x-2">
          <Calculator className="h-5 w-5 text-primary-600 dark:text-primary-400" />
          <span>Mersenne Prime Calculator</span>
          <Badge variant="primary" className="ml-auto">
            High Performance
          </Badge>
        </CardTitle>
      </CardHeader>
      
      <CardContent className="space-y-6">
        {/* Input Section */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          <div className="space-y-4">
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Exponent (p)
              </label>
              <input
                type="number"
                value={exponent}
                onChange={(e) => setExponent(e.target.value)}
                placeholder="Enter exponent..."
                className="input-field w-full"
                disabled={isRunning}
                min="2"
                max="100000000"
              />
              <p className="text-xs text-gray-500 dark:text-gray-400 mt-1">
                Calculate 2^p - 1 where p is prime
              </p>
            </div>

            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Algorithm
              </label>
              <select
                value={algorithm}
                onChange={(e) => setAlgorithm(e.target.value)}
                className="input-field w-full"
                disabled={isRunning}
              >
                <option value="lucas-lehmer">Lucas-Lehmer Test</option>
                <option value="lucas-lehmer-fft">Lucas-Lehmer + FFT</option>
                <option value="parallel">Parallel Lucas-Lehmer</option>
              </select>
            </div>
          </div>

          <div className="space-y-4">
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                Known Mersenne Primes
              </label>
              <div className="grid grid-cols-3 gap-2">
                {presetExponents.map((preset) => (
                  <Button
                    key={preset.value}
                    variant="secondary"
                    size="sm"
                    onClick={() => setExponent(preset.value)}
                    disabled={isRunning}
                    className="text-xs"
                  >
                    {preset.label}
                  </Button>
                ))}
              </div>
            </div>
          </div>
        </div>

        {/* Control Buttons */}
        <div className="flex items-center justify-between pt-4 border-t border-gray-200 dark:border-gray-700">
          <div className="flex items-center space-x-3">
            <motion.div
              whileHover={{ scale: 1.05 }}
              whileTap={{ scale: 0.95 }}
            >
              <Button
                onClick={handleStart}
                disabled={isRunning || !exponent}
                variant="primary"
                size="lg"
                className="flex items-center space-x-2"
              >
                {isRunning ? (
                  <>
                    <motion.div
                      animate={{ rotate: 360 }}
                      transition={{ duration: 1, repeat: Infinity, ease: "linear" }}
                    >
                      <Zap className="h-4 w-4" />
                    </motion.div>
                    <span>Computing...</span>
                  </>
                ) : (
                  <>
                    <Play className="h-4 w-4" />
                    <span>Start Calculation</span>
                  </>
                )}
              </Button>
            </motion.div>

            {isRunning && (
              <motion.div
                initial={{ opacity: 0, scale: 0.8 }}
                animate={{ opacity: 1, scale: 1 }}
                exit={{ opacity: 0, scale: 0.8 }}
              >
                <Button
                  onClick={handleStop}
                  variant="error"
                  size="lg"
                  className="flex items-center space-x-2"
                >
                  <Square className="h-4 w-4" />
                  <span>Stop</span>
                </Button>
              </motion.div>
            )}
          </div>

          <div className="text-sm text-gray-500 dark:text-gray-400">
            {exponent && !isNaN(exponent) && (
              <span>Testing: 2^{exponent} - 1</span>
            )}
          </div>
        </div>

        {/* Progress Indicator */}
        {isRunning && (
          <motion.div
            initial={{ opacity: 0, height: 0 }}
            animate={{ opacity: 1, height: 'auto' }}
            exit={{ opacity: 0, height: 0 }}
            className="bg-gray-50 dark:bg-gray-800 rounded-lg p-4"
          >
            <div className="flex items-center space-x-3">
              <div className="w-4 h-4 bg-primary-600 dark:bg-primary-400 rounded-full animate-pulse"></div>
              <span className="text-sm text-gray-600 dark:text-gray-300">
                Running Lucas-Lehmer primality test for M{exponent}...
              </span>
            </div>
            <div className="w-full bg-gray-200 dark:bg-gray-700 rounded-full h-2 mt-3">
              <motion.div
                className="bg-primary-600 dark:bg-primary-400 h-2 rounded-full"
                initial={{ width: 0 }}
                animate={{ width: '100%' }}
                transition={{ duration: 3, ease: "easeInOut" }}
              />
            </div>
          </motion.div>
        )}
      </CardContent>
    </Card>
  )
}