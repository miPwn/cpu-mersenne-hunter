# Netlify Deployment Guide - Mersenne Prime Calculator

## Quick Deployment Steps

### Option 1: Git Repository Deployment (Recommended)

1. **Create a new repository** on GitHub/GitLab with these files:
   ```
   mersenne-prime-calculator/
   ├── dist/
   │   ├── index.html (your modern UI)
   │   └── assets/ (if any)
   ├── netlify/
   │   └── functions/
   │       └── calculate.js (serverless function)
   ├── netlify.toml (configuration)
   └── README.md
   ```

2. **Connect to Netlify**:
   - Go to [netlify.com](https://netlify.com)
   - Click "New site from Git"
   - Connect your repository
   - Build settings will be automatically detected from `netlify.toml`

3. **Deploy**: Netlify will automatically build and deploy your site

### Option 2: Direct File Upload

1. **Zip the dist folder** with:
   - `dist/index.html`
   - `netlify.toml`
   - `netlify/functions/calculate.js`

2. **Manual deploy on Netlify**:
   - Go to [netlify.com](https://netlify.com)
   - Drag and drop your zip file
   - Netlify will deploy instantly

## Features Included

✅ **Modern Dark Theme UI**: Professional glass effects and smooth animations  
✅ **Serverless Backend**: Netlify Functions for Mersenne prime calculations  
✅ **Responsive Design**: Works perfectly on mobile, tablet, and desktop  
✅ **Interactive Controls**: Calculator with preset Mersenne prime buttons  
✅ **Real-time Results**: Live calculation results and logging  
✅ **Theme Toggle**: Light/dark mode switching  
✅ **Professional Typography**: Inter font with gradient effects  

## Technical Details

- **Frontend**: Static HTML with Tailwind CSS and vanilla JavaScript
- **Backend**: Netlify Functions (serverless)
- **Database**: Client-side storage for results and logs
- **Performance**: Optimized for fast loading and smooth animations

## Known Mersenne Primes Supported

The calculator includes preset buttons for known Mersenne primes:
- M127, M521, M607, M1279, M2203, M3217

## API Endpoint

Once deployed, your calculator will have a serverless API at:
`https://your-site-name.netlify.app/.netlify/functions/calculate`

## Customization

You can customize the theme colors, add more preset values, or modify the calculation algorithm in the Netlify function.

Your modern Mersenne Prime Calculator is ready for production deployment on Netlify!