import React, { useMemo } from 'react'
import { BarChart, Bar, XAxis, YAxis, CartesianGrid, Tooltip, Legend, ResponsiveContainer, LineChart, Line, ScatterChart, Scatter, PieChart, Pie, Cell } from 'recharts'

function Charts({ results }) {
  const chartData = useMemo(() => {
    // Performance over time
    const timeData = results
      .slice(0, 50) // Last 50 results
      .reverse()
      .map((result, index) => ({
        index: index + 1,
        exponent: result.exponent,
        duration: result.duration,
        isPrime: result.isPrime,
        timestamp: new Date(result.timestamp).toLocaleTimeString()
      }))

    // Duration vs Exponent scatter plot
    const scatterData = results.map(result => ({
      exponent: result.exponent,
      duration: result.duration,
      isPrime: result.isPrime
    }))

    // Binned duration distribution
    const durationBins = {}
    results.forEach(result => {
      const bin = Math.floor(Math.log10(result.duration * 1000)) // Log scale in microseconds
      const label = bin < 0 ? '<1μs' : 
                   bin === 0 ? '1-10μs' :
                   bin === 1 ? '10-100μs' :
                   bin === 2 ? '0.1-1ms' :
                   bin === 3 ? '1-10ms' :
                   bin === 4 ? '10-100ms' :
                   bin === 5 ? '0.1-1s' : '>1s'
      
      durationBins[label] = (durationBins[label] || 0) + 1
    })

    const durationDistribution = Object.entries(durationBins).map(([range, count]) => ({
      range,
      count
    }))

    // Prime vs Composite ratio
    const primeCount = results.filter(r => r.isPrime).length
    const compositeCount = results.length - primeCount
    const primeRatio = [
      { name: 'Prime', value: primeCount, color: '#10b981' },
      { name: 'Composite', value: compositeCount, color: '#ef4444' }
    ]

    // Thread utilization
    const threadData = {}
    results.forEach(result => {
      const threads = result.threads || 1
      threadData[threads] = (threadData[threads] || 0) + 1
    })

    const threadDistribution = Object.entries(threadData).map(([threads, count]) => ({
      threads: parseInt(threads),
      count
    })).sort((a, b) => a.threads - b.threads)

    return {
      timeData,
      scatterData,
      durationDistribution,
      primeRatio,
      threadDistribution
    }
  }, [results])

  const formatDuration = (ms) => {
    if (ms < 1) return `${(ms * 1000).toFixed(1)}μs`
    if (ms < 1000) return `${ms.toFixed(2)}ms`
    return `${(ms / 1000).toFixed(2)}s`
  }

  const CustomTooltip = ({ active, payload, label }) => {
    if (active && payload && payload.length) {
      return (
        <div className="bg-white p-3 border border-gray-200 rounded-lg shadow-lg">
          <p className="font-medium">{`Test ${label}`}</p>
          {payload.map((entry, index) => (
            <p key={index} style={{ color: entry.color }}>
              {`${entry.dataKey}: ${entry.dataKey === 'duration' ? formatDuration(entry.value) : entry.value}`}
            </p>
          ))}
        </div>
      )
    }
    return null
  }

  const ScatterTooltip = ({ active, payload }) => {
    if (active && payload && payload.length) {
      const data = payload[0].payload
      return (
        <div className="bg-white p-3 border border-gray-200 rounded-lg shadow-lg">
          <p className="font-medium">{`M${data.exponent}`}</p>
          <p style={{ color: payload[0].color }}>
            Duration: {formatDuration(data.duration)}
          </p>
          <p>{data.isPrime ? 'Prime' : 'Composite'}</p>
        </div>
      )
    }
    return null
  }

  if (results.length === 0) {
    return (
      <div className="bg-white rounded-lg shadow p-8 text-center">
        <p className="text-gray-500">No data available for charts</p>
      </div>
    )
  }

  return (
    <div className="space-y-6">
      {/* Performance Over Time */}
      <div className="bg-white rounded-lg shadow p-6">
        <h3 className="text-lg font-medium text-gray-900 mb-4">Performance Over Time</h3>
        <ResponsiveContainer width="100%" height={300}>
          <LineChart data={chartData.timeData}>
            <CartesianGrid strokeDasharray="3 3" />
            <XAxis dataKey="index" />
            <YAxis tickFormatter={formatDuration} />
            <Tooltip content={<CustomTooltip />} />
            <Legend />
            <Line 
              type="monotone" 
              dataKey="duration" 
              stroke="#3b82f6" 
              strokeWidth={2}
              dot={{ fill: '#3b82f6', strokeWidth: 2, r: 4 }}
            />
          </LineChart>
        </ResponsiveContainer>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Duration vs Exponent Scatter */}
        <div className="bg-white rounded-lg shadow p-6">
          <h3 className="text-lg font-medium text-gray-900 mb-4">Duration vs Exponent</h3>
          <ResponsiveContainer width="100%" height={300}>
            <ScatterChart data={chartData.scatterData}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="exponent" name="Exponent" />
              <YAxis dataKey="duration" name="Duration" tickFormatter={formatDuration} />
              <Tooltip content={<ScatterTooltip />} />
              <Scatter 
                name="Tests" 
                dataKey="duration" 
                fill={(entry) => entry.isPrime ? '#10b981' : '#ef4444'}
              >
                {chartData.scatterData.map((entry, index) => (
                  <Cell key={`cell-${index}`} fill={entry.isPrime ? '#10b981' : '#ef4444'} />
                ))}
              </Scatter>
            </ScatterChart>
          </ResponsiveContainer>
        </div>

        {/* Prime vs Composite Ratio */}
        <div className="bg-white rounded-lg shadow p-6">
          <h3 className="text-lg font-medium text-gray-900 mb-4">Prime vs Composite</h3>
          <ResponsiveContainer width="100%" height={300}>
            <PieChart>
              <Pie
                data={chartData.primeRatio}
                cx="50%"
                cy="50%"
                labelLine={false}
                label={({ name, value, percent }) => `${name}: ${value} (${(percent * 100).toFixed(1)}%)`}
                outerRadius={80}
                fill="#8884d8"
                dataKey="value"
              >
                {chartData.primeRatio.map((entry, index) => (
                  <Cell key={`cell-${index}`} fill={entry.color} />
                ))}
              </Pie>
              <Tooltip />
            </PieChart>
          </ResponsiveContainer>
        </div>
      </div>

      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Duration Distribution */}
        <div className="bg-white rounded-lg shadow p-6">
          <h3 className="text-lg font-medium text-gray-900 mb-4">Duration Distribution</h3>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={chartData.durationDistribution}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="range" />
              <YAxis />
              <Tooltip />
              <Bar dataKey="count" fill="#8b5cf6" />
            </BarChart>
          </ResponsiveContainer>
        </div>

        {/* Thread Utilization */}
        <div className="bg-white rounded-lg shadow p-6">
          <h3 className="text-lg font-medium text-gray-900 mb-4">Thread Utilization</h3>
          <ResponsiveContainer width="100%" height={300}>
            <BarChart data={chartData.threadDistribution}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="threads" />
              <YAxis />
              <Tooltip />
              <Bar dataKey="count" fill="#f59e0b" />
            </BarChart>
          </ResponsiveContainer>
        </div>
      </div>
    </div>
  )
}

export default Charts