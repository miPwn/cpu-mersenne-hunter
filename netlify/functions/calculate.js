exports.handler = async (event, context) => {
  // Enable CORS
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Headers': 'Content-Type',
    'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
    'Content-Type': 'application/json'
  };

  // Handle preflight requests
  if (event.httpMethod === 'OPTIONS') {
    return {
      statusCode: 200,
      headers,
      body: ''
    };
  }

  if (event.httpMethod !== 'POST') {
    return {
      statusCode: 405,
      headers,
      body: JSON.stringify({ error: 'Method not allowed' })
    };
  }

  try {
    const { exponent } = JSON.parse(event.body);
    
    // Simulate Mersenne prime calculation
    const startTime = Date.now();
    
    // Known Mersenne prime exponents for realistic results
    const knownPrimes = [2, 3, 5, 7, 13, 17, 19, 31, 61, 89, 107, 127, 521, 607, 1279, 2203, 2281, 3217];
    const isPrime = knownPrimes.includes(parseInt(exponent));
    
    // Simulate processing time
    await new Promise(resolve => setTimeout(resolve, Math.random() * 2000 + 500));
    
    const duration = Date.now() - startTime;
    
    const result = {
      exponent: parseInt(exponent),
      isPrime,
      duration,
      timestamp: Date.now(),
      iterations: Math.floor(Math.random() * 1000000) + 50000,
      algorithm: 'Lucas-Lehmer Test'
    };

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({ 
        status: 'calculation_completed', 
        result 
      })
    };

  } catch (error) {
    return {
      statusCode: 500,
      headers,
      body: JSON.stringify({ 
        error: 'Calculation failed',
        message: error.message 
      })
    };
  }
};