import React, { useState, useEffect } from 'react'
import { Calculator, Activity, FileText, BarChart3 } from 'lucide-react'
import Dashboard from './components/Dashboard'
import LogViewer from './components/LogViewer'
import Charts from './components/Charts'
import { useWebSocket } from './hooks/useWebSocket'

function App() {
  const [activeTab, setActiveTab] = useState('dashboard')
  const [results, setResults] = useState([])
  const [logs, setLogs] = useState([])
  
  const { connected, error } = useWebSocket({
    onResult: (result) => {
      setResults(prev => [result, ...prev].slice(0, 1000)) // Keep last 1000 results
    },
    onLog: (log) => {
      setLogs(prev => [log, ...prev].slice(0, 5000)) // Keep last 5000 logs
    }
  })

  const tabs = [
    { id: 'dashboard', name: 'Dashboard', icon: Activity },
    { id: 'logs', name: 'Logs', icon: FileText },
    { id: 'charts', name: 'Charts', icon: BarChart3 }
  ]

  return (
    <div className="min-h-screen bg-gray-50">
      {/* Header */}
      <header className="bg-white shadow-sm border-b border-gray-200">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center h-16">
            <div className="flex items-center space-x-3">
              <Calculator className="h-8 w-8 text-primary-600" />
              <div>
                <h1 className="text-xl font-semibold text-gray-900">
                  Mersenne Prime Calculator
                </h1>
                <p className="text-sm text-gray-500">High Performance Dashboard</p>
              </div>
            </div>
            
            <div className="flex items-center space-x-4">
              <div className={`flex items-center space-x-2 px-3 py-1 rounded-full text-sm ${
                connected 
                  ? 'bg-green-100 text-green-800' 
                  : 'bg-red-100 text-red-800'
              }`}>
                <div className={`w-2 h-2 rounded-full ${
                  connected ? 'bg-green-500' : 'bg-red-500'
                }`} />
                <span>{connected ? 'Connected' : 'Disconnected'}</span>
              </div>
              
              <div className="text-sm text-gray-600">
                {results.length} results • {logs.length} logs
              </div>
            </div>
          </div>
        </div>
      </header>

      {/* Navigation Tabs */}
      <nav className="bg-white border-b border-gray-200">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex space-x-8">
            {tabs.map((tab) => {
              const Icon = tab.icon
              return (
                <button
                  key={tab.id}
                  onClick={() => setActiveTab(tab.id)}
                  className={`flex items-center space-x-2 py-4 px-1 border-b-2 font-medium text-sm transition-colors ${
                    activeTab === tab.id
                      ? 'border-primary-500 text-primary-600'
                      : 'border-transparent text-gray-500 hover:text-gray-700 hover:border-gray-300'
                  }`}
                >
                  <Icon className="h-4 w-4" />
                  <span>{tab.name}</span>
                </button>
              )
            })}
          </div>
        </div>
      </nav>

      {/* Error Banner */}
      {error && (
        <div className="bg-red-50 border-l-4 border-red-400 p-4">
          <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
            <p className="text-red-700">Connection error: {error}</p>
          </div>
        </div>
      )}

      {/* Main Content */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        {activeTab === 'dashboard' && (
          <Dashboard 
            results={results} 
            onNewResult={(result) => setResults(prev => [result, ...prev].slice(0, 1000))}
          />
        )}
        {activeTab === 'logs' && <LogViewer logs={logs} />}
        {activeTab === 'charts' && <Charts results={results} />}
      </main>
    </div>
  )
}

export default App