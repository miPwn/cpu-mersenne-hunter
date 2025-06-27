import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  server: {
    host: '0.0.0.0',
    port: 5000,
    // Fix EPIPE errors in Replit by using separate HMR port
    hmr: {
      port: 5001,
      clientPort: 5001,
      host: '0.0.0.0'
    },
    proxy: {
      '/api': 'http://localhost:3001',
      '/ws': {
        target: 'ws://localhost:3001',
        ws: true,
        changeOrigin: true
      }
    }
  },
  build: {
    outDir: 'dist',
    sourcemap: false,
    // Optimize for deployment
    rollupOptions: {
      output: {
        manualChunks: undefined
      }
    }
  },
  preview: {
    host: '0.0.0.0',
    port: 5000
  }
})