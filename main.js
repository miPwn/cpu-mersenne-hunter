#!/usr/bin/env node

// Main entry point for Replit - ensures proper startup
const path = require('path');
const { spawn } = require('child_process');

console.log('🔄 Starting Mersenne Prime Calculator via main.js...');
console.log('📁 Working directory:', process.cwd());
console.log('🎯 Target file:', path.resolve('./index.js'));

// Start the actual server
require('./index.js');