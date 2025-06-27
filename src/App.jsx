import React, { useState, useEffect } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { 
  Calculator, 
  Activity, 
  FileText, 
  BarChart3, 
  Sun, 
  Moon,
  Wifi,
  WifiOff,
  Settings
} from 'lucide-react'
import Dashboard from './components/Dashboard'
import LogViewer from './components/LogViewer'
import Charts from './components/Charts'
import { useWebSocket } from './hooks/useWebSocket'
import { ThemeProvider, useTheme } from './contexts/ThemeContext'
import { Button } from './components/ui/Button'
import { Badge } from './components/ui/Badge'
import clsx from 'clsx'

function AppContent() {
  const [activeTab, setActiveTab] = useState('dashboard')
  const [results, setResults] = useState([])
  const [logs, setLogs] = useState([])
  const { theme, toggleTheme } = useTheme()
  
  const { connected, error } = useWebSocket({
    onResult: (result) => {
      setResults(prev => [result, ...prev].slice(0, 1000))
    },
    onLog: (log) => {
      setLogs(prev => [log, ...prev].slice(0, 5000))
    }
  })

  const tabs = [
    { id: 'dashboard', name: 'Dashboard', icon: Activity, description: 'Control & monitor calculations' },
    { id: 'logs', name: 'Logs', icon: FileText, description: 'View system logs' },
    { id: 'charts', name: 'Analytics', icon: BarChart3, description: 'Performance charts' }
  ]

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-950 transition-colors duration-300">
      {/* Glass Header */}
      <header className="sticky top-0 z-50 glass-effect border-b border-gray-200/50 dark:border-gray-800/50">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex justify-between items-center h-16">
            {/* Logo & Title */}
            <motion.div 
              className="flex items-center space-x-3"
              initial={{ opacity: 0, x: -20 }}
              animate={{ opacity: 1, x: 0 }}
              transition={{ duration: 0.5 }}
            >
              <div className="relative">
                <Calculator className="h-8 w-8 text-primary-600 dark:text-primary-400" />
                <div className="absolute -top-1 -right-1 w-3 h-3 bg-primary-500 rounded-full animate-pulse"></div>
              </div>
              <div>
                <h1 className="text-xl font-bold bg-gradient-to-r from-gray-900 to-gray-600 dark:from-gray-100 dark:to-gray-400 bg-clip-text text-transparent">
                  Mersenne Prime Calculator
                </h1>
                <p className="text-sm text-gray-500 dark:text-gray-400">High-Performance Computing Dashboard</p>
              </div>
            </motion.div>
            
            {/* Status & Controls */}
            <motion.div 
              className="flex items-center space-x-4"
              initial={{ opacity: 0, x: 20 }}
              animate={{ opacity: 1, x: 0 }}
              transition={{ duration: 0.5, delay: 0.1 }}
            >
              {/* Connection Status */}
              <Badge 
                variant={connected ? 'success' : 'error'}
                icon={connected ? Wifi : WifiOff}
                className="animate-fade-in"
              >
                {connected ? 'Connected' : 'Disconnected'}
              </Badge>
              
              {/* Stats */}
              <div className="hidden sm:flex items-center space-x-3 text-sm text-gray-600 dark:text-gray-400">
                <span className="flex items-center space-x-1">
                  <div className="w-2 h-2 bg-primary-500 rounded-full"></div>
                  <span>{results.length} results</span>
                </span>
                <span className="flex items-center space-x-1">
                  <div className="w-2 h-2 bg-success-500 rounded-full"></div>
                  <span>{logs.length} logs</span>
                </span>
              </div>

              {/* Theme Toggle */}
              <Button
                variant="ghost"
                size="sm"
                onClick={toggleTheme}
                className="p-2"
                aria-label="Toggle theme"
              >
                <motion.div
                  initial={false}
                  animate={{ rotate: theme === 'dark' ? 180 : 0 }}
                  transition={{ duration: 0.3 }}
                >
                  {theme === 'dark' ? (
                    <Sun className="h-4 w-4" />
                  ) : (
                    <Moon className="h-4 w-4" />
                  )}
                </motion.div>
              </Button>
            </motion.div>
          </div>
        </div>
      </header>

      {/* Enhanced Navigation */}
      <nav className="glass-effect border-b border-gray-200/50 dark:border-gray-800/50">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
          <div className="flex space-x-8">
            {tabs.map((tab, index) => {
              const Icon = tab.icon
              const isActive = activeTab === tab.id
              
              return (
                <motion.button
                  key={tab.id}
                  onClick={() => setActiveTab(tab.id)}
                  className={clsx(
                    'relative flex items-center space-x-2 py-4 px-1 font-medium text-sm transition-all duration-200',
                    'hover:text-primary-600 dark:hover:text-primary-400',
                    isActive 
                      ? 'text-primary-600 dark:text-primary-400' 
                      : 'text-gray-500 dark:text-gray-400'
                  )}
                  initial={{ opacity: 0, y: 10 }}
                  animate={{ opacity: 1, y: 0 }}
                  transition={{ duration: 0.3, delay: index * 0.1 }}
                  whileHover={{ y: -2 }}
                  whileTap={{ y: 0 }}
                >
                  <Icon className="h-4 w-4" />
                  <span>{tab.name}</span>
                  
                  {/* Active indicator */}
                  {isActive && (
                    <motion.div
                      className="absolute bottom-0 left-0 right-0 h-0.5 bg-primary-600 dark:bg-primary-400"
                      layoutId="activeTab"
                      transition={{ duration: 0.3 }}
                    />
                  )}
                  
                  {/* Hover tooltip */}
                  <div className="absolute top-full mt-2 left-1/2 transform -translate-x-1/2 px-2 py-1 bg-gray-900 dark:bg-gray-700 text-white text-xs rounded opacity-0 pointer-events-none transition-opacity duration-200 group-hover:opacity-100 whitespace-nowrap">
                    {tab.description}
                  </div>
                </motion.button>
              )
            })}
          </div>
        </div>
      </nav>

      {/* Error Banner */}
      <AnimatePresence>
        {error && (
          <motion.div 
            className="bg-error-50 dark:bg-error-900/20 border-l-4 border-error-400 p-4"
            initial={{ opacity: 0, height: 0 }}
            animate={{ opacity: 1, height: 'auto' }}
            exit={{ opacity: 0, height: 0 }}
            transition={{ duration: 0.3 }}
          >
            <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
              <p className="text-error-700 dark:text-error-300">Connection error: {error}</p>
            </div>
          </motion.div>
        )}
      </AnimatePresence>

      {/* Main Content with Page Transitions */}
      <main className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <AnimatePresence mode="wait">
          <motion.div
            key={activeTab}
            initial={{ opacity: 0, y: 20 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: -20 }}
            transition={{ duration: 0.3 }}
            className="animate-slide-up"
          >
            {activeTab === 'dashboard' && (
              <Dashboard 
                results={results} 
                onNewResult={(result) => setResults(prev => [result, ...prev].slice(0, 1000))}
              />
            )}
            {activeTab === 'logs' && <LogViewer logs={logs} />}
            {activeTab === 'charts' && <Charts results={results} />}
          </motion.div>
        </AnimatePresence>
      </main>
    </div>
  )
}

function App() {
  return (
    <ThemeProvider>
      <AppContent />
    </ThemeProvider>
  )
}

export default App