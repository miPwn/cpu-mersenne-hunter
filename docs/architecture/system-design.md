# System Architecture Design

## Overview

Mersenne Hunter implements a layered architecture designed for high-performance mathematical computation, research scalability, and maintainable code organization. The system is built around core mathematical algorithms with supporting infrastructure for performance monitoring, web interfaces, and extensible computation strategies.

## Architectural Principles

### 1. **Separation of Concerns**
- Mathematical algorithms isolated from I/O operations
- Performance profiling decoupled from computational logic
- Web interface separated from core computation engine

### 2. **Performance-First Design**
- Zero-cost abstractions where possible
- Memory-efficient data structures
- Cache-aware algorithm implementations
- NUMA-conscious thread management

### 3. **Research Extensibility**
- Pluggable multiplication strategies
- Configurable algorithm parameters
- Comprehensive performance instrumentation
- Academic-grade result validation

## High-Level System Architecture

```mermaid
graph TB
    subgraph "Presentation Layer"
        CLI[Command Line Interface]
        WEB[Web Dashboard]
        API[REST API]
        WS[WebSocket Interface]
    end
    
    subgraph "Application Layer"
        MAIN[Main Controller]
        CONFIG[Configuration Manager]
        LOG[Logging System]
        VALID[Result Validator]
    end
    
    subgraph "Computation Engine"
        MP[MersennePrime Controller]
        PROF[Performance Profiler]
        STRAT[Multiplication Strategy]
        SCHED[Task Scheduler]
    end
    
    subgraph "Mathematical Core"
        LL[Lucas-Lehmer Algorithm]
        FFT[FFT Multiplier]
        NTT[NTT Multiplier]
        KARAT[Karatsuba Multiplier]
        BIGINT[Big Integer Operations]
    end
    
    subgraph "System Layer"
        THREAD[Thread Management]
        MEM[Memory Manager]
        CACHE[Cache Optimizer]
        HW[Hardware Detection]
    end
    
    subgraph "Data Persistence"
        JSON[JSON Exports]
        CSV[CSV Reports]
        PERF[Performance Database]
        RESULTS[Results Archive]
    end
    
    CLI --> MAIN
    WEB --> API
    API --> MAIN
    WS --> MAIN
    
    MAIN --> MP
    MAIN --> CONFIG
    MAIN --> LOG
    MAIN --> VALID
    
    MP --> LL
    MP --> PROF
    MP --> STRAT
    MP --> SCHED
    
    LL --> STRAT
    STRAT --> FFT
    STRAT --> NTT
    STRAT --> KARAT
    STRAT --> BIGINT
    
    MP --> THREAD
    THREAD --> MEM
    MEM --> CACHE
    CACHE --> HW
    
    PROF --> JSON
    PROF --> CSV
    PROF --> PERF
    VALID --> RESULTS
    
    style LL fill:#FFE4B5
    style FFT fill:#F0E68C
    style NTT fill:#F0E68C
    style KARAT fill:#F0E68C
```

## Component Architecture

### Core Computation Engine

#### MersennePrime Class
```mermaid
classDiagram
    class MersennePrime {
        -PerformanceProfiler& profiler
        -atomic~uint64_t~ iterations_completed
        -atomic~uint64_t~ fft_operations
        -static thread_local ThreadContext
        
        +is_mersenne_prime(exponent, threads) bool
        +find_mersenne_primes_range(start, end, threads) vector~uint64_t~
        +get_iterations_completed() uint64_t
        +get_fft_operations() uint64_t
        
        -lucas_lehmer_sequential(exponent) bool
        -lucas_lehmer_parallel(exponent, threads) bool
        -mod_mersenne_optimized(result, value, exponent) void
        -square_mod_mersenne(result, value, exponent) void
    }
    
    class ThreadContext {
        +mpz_t s, temp, mersenne, temp2
        +ThreadContext()
        +~ThreadContext()
    }
    
    MersennePrime --> ThreadContext : manages
```

#### Multiplication Strategy Pattern
```mermaid
classDiagram
    class MultiplicationStrategy {
        <<Strategy>>
        -FFTMultiplier fft_multiplier
        -NTTMultiplier ntt_multiplier
        -KaratsubaMultiplier karatsuba_multiplier
        
        +multiply(result, a, b, strategy) void
        +square(result, value, strategy) void
        +select_best_strategy(a, b) Strategy
        +benchmark_strategies(a, b, results) void
    }
    
    class FFTMultiplier {
        -static vector~FFTWPlanCache~ plan_cache
        +multiply(result, a, b) void
        +square(result, value) void
        +multiply_and_mod_mersenne(result, a, b, exp) void
        -convolution_fft(a, b, result) void
    }
    
    class NTTMultiplier {
        -static NTTContext ntt_context
        +multiply(result, a, b) void
        +square(result, value) void
        -ntt_transform(a, inverse) void
        -convolution_ntt(a, b, result) void
    }
    
    class KaratsubaMultiplier {
        +multiply(result, a, b) void
        +square(result, value) void
        -karatsuba_multiply_limbs(result, a, a_size, b, b_size) void
    }
    
    MultiplicationStrategy --> FFTMultiplier
    MultiplicationStrategy --> NTTMultiplier
    MultiplicationStrategy --> KaratsubaMultiplier
```

### Performance Profiling Architecture

```mermaid
graph LR
    subgraph "Profiling System"
        PROF[PerformanceProfiler Singleton]
        TIMER[Timer Management]
        MEM[Memory Tracking]
        SYS[System Information]
        SAMPLE[Sampling Thread]
    end
    
    subgraph "Data Collection"
        TIMING[Timing Data Map]
        COUNTERS[Performance Counters]
        MEMORY[Memory Statistics]
        HWINFO[Hardware Metrics]
    end
    
    subgraph "Export Formats"
        JSON_OUT[JSON Export]
        CSV_OUT[CSV Export]
        CONSOLE[Console Reports]
        WEB_API[Web API Metrics]
    end
    
    PROF --> TIMER
    PROF --> MEM
    PROF --> SYS
    PROF --> SAMPLE
    
    TIMER --> TIMING
    MEM --> MEMORY
    SYS --> HWINFO
    SAMPLE --> COUNTERS
    
    TIMING --> JSON_OUT
    MEMORY --> CSV_OUT
    HWINFO --> CONSOLE
    COUNTERS --> WEB_API
```

### Thread Management System

```mermaid
sequenceDiagram
    participant Main as Main Thread
    participant Sched as Task Scheduler
    participant Pool as Thread Pool
    participant Worker as Worker Threads
    participant Prof as Profiler
    
    Main->>Sched: Initialize with thread count
    Sched->>Pool: Create worker threads
    Pool->>Worker: Spawn N workers
    
    Main->>Sched: Submit Lucas-Lehmer task
    Sched->>Pool: Decompose into subtasks
    Pool->>Worker: Distribute FFT operations
    
    loop For each Lucas-Lehmer iteration
        Worker->>Worker: Parallel FFT computation
        Worker->>Prof: Record timing metrics
        Worker->>Pool: Return partial results
        Pool->>Sched: Aggregate results
        Sched->>Main: Return S_i value
    end
    
    Main->>Prof: Generate performance report
    Prof-->>Main: Timing and memory statistics
```

## Memory Management Architecture

### Memory Allocation Strategy

```mermaid
graph TD
    subgraph "Memory Hierarchy"
        APP[Application Request]
        ALLOC[Custom Allocator]
        POOL[Memory Pools]
        NUMA[NUMA-Aware Allocation]
        SYS[System Memory]
    end
    
    subgraph "GMP Integration"
        GMP[GMP Memory Functions]
        CUSTOM[Custom GMP Allocator]
        TRACK[Allocation Tracking]
    end
    
    subgraph "Optimization Layers"
        ALIGN[Cache Alignment]
        PREFETCH[Prefetch Hints]
        LOCALITY[Data Locality]
    end
    
    APP --> ALLOC
    ALLOC --> POOL
    POOL --> NUMA
    NUMA --> SYS
    
    ALLOC --> GMP
    GMP --> CUSTOM
    CUSTOM --> TRACK
    
    POOL --> ALIGN
    ALIGN --> PREFETCH
    PREFETCH --> LOCALITY
    
    style NUMA fill:#98FB98
    style ALIGN fill:#87CEEB
    style LOCALITY fill:#DDA0DD
```

### Cache Optimization

The system implements multiple levels of cache optimization:

1. **L1 Cache Optimization**
   - Data structure alignment to cache line boundaries
   - Loop tiling for optimal cache utilization
   - Prefetch instructions for predictable access patterns

2. **L2/L3 Cache Management**
   - Blocking algorithms for large matrix operations (FFT)
   - Cache-aware data layouts
   - Minimization of cache conflicts

3. **Memory Bandwidth Optimization**
   - NUMA-aware thread scheduling
   - Streaming optimizations for large data sets
   - Vectorized memory operations

## Web Interface Architecture

### Frontend-Backend Communication

```mermaid
sequenceDiagram
    participant UI as React Frontend
    participant API as Node.js Backend
    participant WS as WebSocket Server
    participant CPP as C++ Engine
    participant PROF as Profiler
    
    UI->>API: POST /api/calculate
    API->>CPP: Spawn mersenne_prime process
    API->>WS: Establish progress channel
    
    loop Computation Progress
        CPP->>PROF: Update performance metrics
        PROF->>WS: Stream progress data
        WS->>UI: Real-time updates
    end
    
    CPP->>API: Return computation result
    API->>UI: Final result with metadata
    
    UI->>API: GET /api/metrics
    API->>PROF: Fetch performance data
    PROF->>API: JSON metrics
    API->>UI: Performance dashboard data
```

### API Design Patterns

```mermaid
graph TB
    subgraph "REST API Endpoints"
        CALC[/api/calculate/:exponent]
        BATCH[/api/batch]
        STATUS[/api/status/:taskId]
        METRICS[/api/metrics]
        HEALTH[/health]
    end
    
    subgraph "WebSocket Channels"
        PROGRESS[/ws - Progress Updates]
        LOGS[/ws - Log Stream]
        METRICS_WS[/ws - Real-time Metrics]
    end
    
    subgraph "Data Validation"
        INPUT[Input Sanitization]
        LIMITS[Resource Limits]
        AUTH[Authentication]
    end
    
    CALC --> INPUT
    BATCH --> LIMITS
    STATUS --> AUTH
    
    PROGRESS --> CALC
    LOGS --> BATCH
    METRICS_WS --> METRICS
    
    style CALC fill:#FFB6C1
    style PROGRESS fill:#98FB98
    style INPUT fill:#F0E68C
```

## Scalability Considerations

### Horizontal Scaling

```mermaid
graph LR
    subgraph "Load Balancer"
        LB[NGINX/HAProxy]
    end
    
    subgraph "Application Tier"
        APP1[Node.js Instance 1]
        APP2[Node.js Instance 2]
        APP3[Node.js Instance N]
    end
    
    subgraph "Computation Tier"
        COMP1[C++ Engine 1]
        COMP2[C++ Engine 2]
        COMP3[C++ Engine N]
    end
    
    subgraph "Data Tier"
        REDIS[Redis Cache]
        MONGO[MongoDB Results]
        METRICS[Prometheus Metrics]
    end
    
    LB --> APP1
    LB --> APP2
    LB --> APP3
    
    APP1 --> COMP1
    APP2 --> COMP2
    APP3 --> COMP3
    
    APP1 --> REDIS
    APP2 --> REDIS
    APP3 --> REDIS
    
    COMP1 --> MONGO
    COMP2 --> MONGO
    COMP3 --> MONGO
    
    APP1 --> METRICS
    APP2 --> METRICS
    APP3 --> METRICS
```

### Vertical Scaling Optimizations

1. **CPU Scaling**
   - Automatic thread count detection
   - NUMA topology awareness
   - CPU affinity optimization

2. **Memory Scaling**
   - Streaming algorithms for memory-constrained environments
   - Disk-based intermediate storage for very large computations
   - Memory-mapped file support

3. **I/O Scaling**
   - Asynchronous result persistence
   - Batch processing capabilities
   - Compressed data formats

## Security Architecture

### Input Validation and Sanitization

```mermaid
graph TD
    A[User Input] --> B[Input Validation]
    B --> C{Valid Range?}
    C -->|No| D[Reject with Error]
    C -->|Yes| E[Resource Check]
    E --> F{Within Limits?}
    F -->|No| G[Queue for Later]
    F -->|Yes| H[Execute Computation]
    
    subgraph "Validation Rules"
        V1[Exponent Range: 2-10^8]
        V2[Thread Count: 1-CPU_COUNT]
        V3[Memory Limit Check]
        V4[Execution Time Limit]
    end
    
    B --> V1
    B --> V2
    E --> V3
    E --> V4
    
    style D fill:#FFB6C1
    style G fill:#FFE4B5
    style H fill:#90EE90
```

### Process Isolation

- Each computation runs in a separate process
- Resource limits enforced via cgroups (Linux)
- Sandboxing for untrusted computations
- Memory and CPU usage monitoring

## Performance Monitoring

### Real-time Metrics Collection

```mermaid
graph TB
    subgraph "Metrics Sources"
        CPU[CPU Usage]
        MEM[Memory Usage]
        CACHE[Cache Performance]
        THREAD[Thread Utilization]
        ALGO[Algorithm Progress]
    end
    
    subgraph "Collection Layer"
        AGENT[Metrics Agent]
        BUFFER[Ring Buffer]
        SAMPLE[Sampling Controller]
    end
    
    subgraph "Processing Layer"
        AGG[Aggregation Engine]
        FILTER[Filtering Rules]
        ALERT[Alert Manager]
    end
    
    subgraph "Storage & Export"
        TSDB[Time Series DB]
        JSON_EXP[JSON Export]
        GRAFANA[Grafana Dashboard]
        API_EXP[API Export]
    end
    
    CPU --> AGENT
    MEM --> AGENT
    CACHE --> AGENT
    THREAD --> AGENT
    ALGO --> AGENT
    
    AGENT --> BUFFER
    BUFFER --> SAMPLE
    SAMPLE --> AGG
    
    AGG --> FILTER
    FILTER --> ALERT
    
    AGG --> TSDB
    AGG --> JSON_EXP
    TSDB --> GRAFANA
    JSON_EXP --> API_EXP
```

## Quality Assurance Architecture

### Testing Strategy

```mermaid
graph TB
    subgraph "Test Types"
        UNIT[Unit Tests]
        INTEGRATION[Integration Tests]
        PERFORMANCE[Performance Tests]
        ACCURACY[Mathematical Accuracy]
        STRESS[Stress Tests]
    end
    
    subgraph "Test Infrastructure"
        FRAMEWORK[Google Test/Catch2]
        CI[GitHub Actions]
        COVERAGE[Code Coverage]
        BENCH[Benchmark Suite]
    end
    
    subgraph "Validation Methods"
        CROSS[Cross-Validation]
        KNOWN[Known Results Comparison]
        PROBABILISTIC[Probabilistic Verification]
        INDEPENDENT[Independent Implementation]
    end
    
    UNIT --> FRAMEWORK
    INTEGRATION --> CI
    PERFORMANCE --> BENCH
    ACCURACY --> CROSS
    STRESS --> COVERAGE
    
    CROSS --> KNOWN
    KNOWN --> PROBABILISTIC
    PROBABILISTIC --> INDEPENDENT
    
    style ACCURACY fill:#90EE90
    style CROSS fill:#87CEEB
    style KNOWN fill:#DDA0DD
```

## Deployment Architecture

### Container Strategy

```mermaid
graph TB
    subgraph "Base Images"
        UBUNTU[Ubuntu 20.04 LTS]
        DEPS[Dependencies Layer]
        BUILD[Build Tools Layer]
    end
    
    subgraph "Application Layers"
        CPP_BUILD[C++ Build Layer]
        NODE_BUILD[Node.js Build Layer]
        STATIC[Static Assets Layer]
    end
    
    subgraph "Runtime Containers"
        CPP_RT[C++ Runtime]
        NODE_RT[Node.js Runtime]
        NGINX_RT[NGINX Proxy]
    end
    
    subgraph "Orchestration"
        COMPOSE[Docker Compose]
        K8S[Kubernetes (Optional)]
        MONITORING[Monitoring Stack]
    end
    
    UBUNTU --> DEPS
    DEPS --> BUILD
    BUILD --> CPP_BUILD
    BUILD --> NODE_BUILD
    BUILD --> STATIC
    
    CPP_BUILD --> CPP_RT
    NODE_BUILD --> NODE_RT
    STATIC --> NGINX_RT
    
    CPP_RT --> COMPOSE
    NODE_RT --> COMPOSE
    NGINX_RT --> COMPOSE
    COMPOSE --> K8S
    K8S --> MONITORING
    
    style CPP_RT fill:#98FB98
    style NODE_RT fill:#87CEEB
    style NGINX_RT fill:#DDA0DD
```

## Conclusion

This architecture provides a robust foundation for research-grade Mersenne prime computation with:

- **High Performance**: Optimized mathematical algorithms with parallel execution
- **Scalability**: Horizontal and vertical scaling capabilities
- **Maintainability**: Clean separation of concerns and modular design
- **Research Integration**: Comprehensive instrumentation and validation
- **Production Ready**: Security, monitoring, and deployment considerations

The modular design allows for independent optimization and enhancement of each component while maintaining system cohesion and performance characteristics suitable for academic research and large-scale computational mathematics.

---

*This architecture documentation provides the technical foundation for understanding and extending the Mersenne Hunter computational platform.*