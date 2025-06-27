import React, { useState, useEffect, useRef, useMemo } from 'react'
import { Play, Pause, Search, Download, Trash2, Filter } from 'lucide-react'

function LogViewer({ logs }) {
  const [isPaused, setIsPaused] = useState(false)
  const [searchTerm, setSearchTerm] = useState('')
  const [logLevel, setLogLevel] = useState('all')
  const [autoScroll, setAutoScroll] = useState(true)
  const logContainerRef = useRef(null)
  const bottomRef = useRef(null)

  // Auto-scroll to bottom when new logs arrive (if not paused and auto-scroll is enabled)
  useEffect(() => {
    if (!isPaused && autoScroll && bottomRef.current) {
      bottomRef.current.scrollIntoView({ behavior: 'smooth' })
    }
  }, [logs, isPaused, autoScroll])

  const filteredLogs = useMemo(() => {
    let filtered = logs

    // Filter by search term
    if (searchTerm) {
      filtered = filtered.filter(log =>
        log.message.toLowerCase().includes(searchTerm.toLowerCase()) ||
        log.exponent?.toString().includes(searchTerm) ||
        log.level?.toLowerCase().includes(searchTerm.toLowerCase())
      )
    }

    // Filter by log level
    if (logLevel !== 'all') {
      filtered = filtered.filter(log => log.level === logLevel)
    }

    return filtered
  }, [logs, searchTerm, logLevel])

  const handleScroll = () => {
    const container = logContainerRef.current
    if (container) {
      const isAtBottom = container.scrollHeight - container.scrollTop <= container.clientHeight + 10
      setAutoScroll(isAtBottom)
    }
  }

  const downloadLogs = () => {
    const logText = logs.map(log => 
      `[${new Date(log.timestamp).toISOString()}] ${log.level?.toUpperCase() || 'INFO'}: ${log.message}`
    ).join('\n')
    
    const blob = new Blob([logText], { type: 'text/plain' })
    const url = URL.createObjectURL(blob)
    const a = document.createElement('a')
    a.href = url
    a.download = `mersenne-logs-${new Date().toISOString().split('T')[0]}.txt`
    document.body.appendChild(a)
    a.click()
    document.body.removeChild(a)
    URL.revokeObjectURL(url)
  }

  const getLogLevelColor = (level) => {
    switch (level?.toLowerCase()) {
      case 'error': return 'text-red-600 bg-red-50'
      case 'warn': return 'text-yellow-600 bg-yellow-50'
      case 'info': return 'text-blue-600 bg-blue-50'
      case 'debug': return 'text-gray-600 bg-gray-50'
      default: return 'text-gray-800 bg-white'
    }
  }

  const formatTimestamp = (timestamp) => {
    const date = new Date(timestamp)
    return date.toLocaleTimeString() + '.' + date.getMilliseconds().toString().padStart(3, '0')
  }

  return (
    <div className="space-y-4">
      {/* Controls */}
      <div className="bg-white rounded-lg shadow p-4">
        <div className="flex flex-wrap items-center justify-between gap-4">
          <div className="flex items-center space-x-4">
            <button
              onClick={() => setIsPaused(!isPaused)}
              className={`flex items-center space-x-2 px-3 py-2 rounded-md text-sm font-medium ${
                isPaused 
                  ? 'bg-green-100 text-green-700 hover:bg-green-200' 
                  : 'bg-red-100 text-red-700 hover:bg-red-200'
              }`}
            >
              {isPaused ? (
                <>
                  <Play className="h-4 w-4" />
                  <span>Resume</span>
                </>
              ) : (
                <>
                  <Pause className="h-4 w-4" />
                  <span>Pause</span>
                </>
              )}
            </button>

            <div className="flex items-center space-x-2">
              <input
                type="checkbox"
                id="autoScroll"
                checked={autoScroll}
                onChange={(e) => setAutoScroll(e.target.checked)}
                className="rounded border-gray-300 text-primary-600 focus:ring-primary-500"
              />
              <label htmlFor="autoScroll" className="text-sm text-gray-700">
                Auto-scroll
              </label>
            </div>

            <span className="text-sm text-gray-500">
              {filteredLogs.length} / {logs.length} logs
            </span>
          </div>

          <div className="flex items-center space-x-2">
            <button
              onClick={downloadLogs}
              className="flex items-center space-x-2 px-3 py-2 bg-gray-100 text-gray-700 rounded-md hover:bg-gray-200 text-sm"
            >
              <Download className="h-4 w-4" />
              <span>Download</span>
            </button>
          </div>
        </div>

        {/* Search and Filters */}
        <div className="mt-4 grid grid-cols-1 md:grid-cols-3 gap-4">
          <div className="relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-gray-400" />
            <input
              type="text"
              placeholder="Search logs..."
              className="pl-10 w-full rounded-md border border-gray-300 px-3 py-2 text-sm focus:border-primary-500 focus:ring-primary-500"
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
            />
          </div>

          <div>
            <select
              className="w-full rounded-md border border-gray-300 px-3 py-2 text-sm focus:border-primary-500 focus:ring-primary-500"
              value={logLevel}
              onChange={(e) => setLogLevel(e.target.value)}
            >
              <option value="all">All Levels</option>
              <option value="error">Error</option>
              <option value="warn">Warning</option>
              <option value="info">Info</option>
              <option value="debug">Debug</option>
            </select>
          </div>
        </div>
      </div>

      {/* Log Display */}
      <div className="bg-white rounded-lg shadow">
        <div className="px-4 py-3 border-b border-gray-200">
          <h3 className="text-lg font-medium text-gray-900">Live Logs</h3>
        </div>

        <div 
          ref={logContainerRef}
          onScroll={handleScroll}
          className="h-96 overflow-y-auto custom-scrollbar font-mono text-sm"
        >
          <div className="p-4 space-y-1">
            {filteredLogs.map((log, index) => (
              <div 
                key={`${log.timestamp}-${index}`}
                className={`flex items-start space-x-3 py-1 px-2 rounded ${getLogLevelColor(log.level)} log-entry-new`}
              >
                <span className="text-xs text-gray-500 shrink-0 w-24">
                  {formatTimestamp(log.timestamp)}
                </span>
                
                {log.level && (
                  <span className={`text-xs font-medium shrink-0 w-12 ${
                    log.level === 'error' ? 'text-red-600' :
                    log.level === 'warn' ? 'text-yellow-600' :
                    log.level === 'info' ? 'text-blue-600' :
                    'text-gray-600'
                  }`}>
                    {log.level.toUpperCase()}
                  </span>
                )}

                <span className="flex-1 break-words">
                  {log.message}
                </span>

                {log.exponent && (
                  <span className="text-xs bg-gray-200 text-gray-700 px-2 py-1 rounded shrink-0">
                    M{log.exponent}
                  </span>
                )}
              </div>
            ))}
            
            {filteredLogs.length === 0 && (
              <div className="text-center py-8 text-gray-500">
                {searchTerm || logLevel !== 'all' ? 'No logs match your filters' : 'No logs available'}
              </div>
            )}
          </div>
          <div ref={bottomRef} />
        </div>
      </div>
    </div>
  )
}

export default LogViewer