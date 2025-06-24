/**
 * TupleDNS JavaScript/WebAssembly Bindings
 * 
 * Browser and Node.js compatible bindings for TupleDNS
 * Enables multidimensional discovery in web applications
 */

class TupleDNSError extends Error {
    constructor(code, message) {
        super(message);
        this.name = 'TupleDNSError';
        this.code = code;
    }
}

class TupleNode {
    constructor(data) {
        this.coordinate = data.coordinate;
        this.ipAddress = data.ipAddress;
        this.capabilities = data.capabilities || [];
        this.ttl = data.ttl || 300;
        this.lastSeen = data.lastSeen || Date.now();
    }
    
    hasCapability(capability) {
        return this.capabilities.includes(capability);
    }
    
    toJSON() {
        return {
            coordinate: this.coordinate,
            ipAddress: this.ipAddress,
            capabilities: this.capabilities,
            ttl: this.ttl,
            lastSeen: this.lastSeen
        };
    }
}

class TupleResult {
    constructor(data) {
        this.nodes = (data.nodes || []).map(node => new TupleNode(node));
        this.totalQueries = data.totalQueries || 0;
        this.queryTime = data.queryTime || 0;
        this.error = data.error || 0;
    }
}

class TupleDNS {
    constructor(options = {}) {
        this.options = {
            timeout: options.timeout || 5000,
            maxConcurrent: options.maxConcurrent || 16,
            enableCaching: options.enableCaching !== false,
            proxyUrl: options.proxyUrl || null,
            wasmPath: options.wasmPath || './tupledns.wasm'
        };
        
        this.initialized = false;
        this.wasmModule = null;
        this.cache = new Map();
    }
    
    async init() {
        if (this.initialized) return;
        
        try {
            if (typeof WebAssembly !== 'undefined') {
                // Browser or Node.js with WebAssembly support
                await this.initWasm();
            } else {
                // Fallback to proxy/native implementation
                await this.initProxy();
            }
            
            this.initialized = true;
        } catch (error) {
            throw new TupleDNSError(-1, `Failed to initialize TupleDNS: ${error.message}`);
        }
    }
    
    async initWasm() {
        try {
            let wasmBytes;
            
            if (typeof fetch !== 'undefined') {
                // Browser environment
                const response = await fetch(this.options.wasmPath);
                if (!response.ok) {
                    throw new Error(`Failed to fetch WASM: ${response.statusText}`);
                }
                wasmBytes = await response.arrayBuffer();
            } else {
                // Node.js environment
                const fs = require('fs');
                wasmBytes = fs.readFileSync(this.options.wasmPath);
            }
            
            const wasmModule = await WebAssembly.instantiate(wasmBytes, {
                env: {
                    // Environment imports for C library
                    memory: new WebAssembly.Memory({ initial: 256 }),
                    __memory_base: 0,
                    __table_base: 0,
                    abort: () => { throw new Error('WASM abort'); }
                }
            });
            
            this.wasmModule = wasmModule.instance.exports;
            
            // Initialize the C library
            const result = this.wasmModule.tupledns_init(0);
            if (result !== 0) {
                throw new Error(`WASM initialization failed: ${result}`);
            }
            
        } catch (error) {
            throw new Error(`WebAssembly initialization failed: ${error.message}`);
        }
    }
    
    async initProxy() {
        // For environments without WebAssembly, use HTTP proxy
        if (!this.options.proxyUrl) {
            throw new Error('No WebAssembly support and no proxy URL provided');
        }
        
        // Test proxy connection
        try {
            const response = await this.fetchWithTimeout(`${this.options.proxyUrl}/health`, {
                method: 'GET',
                timeout: this.options.timeout
            });
            
            if (!response.ok) {
                throw new Error(`Proxy health check failed: ${response.statusText}`);
            }
        } catch (error) {
            throw new Error(`Proxy connection failed: ${error.message}`);
        }
    }
    
    validateCoordinate(coordinate) {
        if (typeof coordinate !== 'string' || coordinate.length === 0) {
            return false;
        }
        
        if (coordinate.length > 253) { // Max DNS name length
            return false;
        }
        
        if (!coordinate.endsWith('.tuple')) {
            return false;
        }
        
        // Check valid DNS characters
        const validChars = /^[a-zA-Z0-9.-]+$/;
        return validChars.test(coordinate);
    }
    
    encodeCoordinate(spaceType, values) {
        if (!spaceType || !Array.isArray(values) || values.length === 0) {
            throw new TupleDNSError(-4, 'Invalid parameters for coordinate encoding');
        }
        
        const coordinate = `${values.join('.')}.${spaceType}.tuple`;
        
        if (!this.validateCoordinate(coordinate)) {
            throw new TupleDNSError(-1, 'Generated coordinate is invalid');
        }
        
        return coordinate;
    }
    
    decodeCoordinate(coordinate) {
        if (!this.validateCoordinate(coordinate)) {
            throw new TupleDNSError(-1, 'Invalid coordinate format');
        }
        
        // Remove .tuple suffix
        const withoutSuffix = coordinate.slice(0, -6);
        const parts = withoutSuffix.split('.');
        
        if (parts.length < 2) {
            throw new TupleDNSError(-1, 'Coordinate must have at least space type');
        }
        
        const spaceType = parts.pop();
        const values = parts;
        
        return { spaceType, values };
    }
    
    matchPattern(coordinate, pattern) {
        if (!coordinate || !pattern) {
            return false;
        }
        
        const coordParts = coordinate.split('.');
        const patternParts = pattern.split('.');
        
        if (coordParts.length !== patternParts.length) {
            return false;
        }
        
        for (let i = 0; i < coordParts.length; i++) {
            if (patternParts[i] !== '*' && patternParts[i] !== coordParts[i]) {
                return false;
            }
        }
        
        return true;
    }
    
    async register(coordinate, capabilities = [], ttl = 300, ipAddress = null) {
        if (!this.initialized) {
            await this.init();
        }
        
        if (!this.validateCoordinate(coordinate)) {
            throw new TupleDNSError(-1, 'Invalid coordinate format');
        }
        
        const registrationData = {
            coordinate,
            capabilities,
            ttl,
            ipAddress: ipAddress || await this.getClientIP(),
            timestamp: Date.now()
        };
        
        if (this.wasmModule) {
            // Use WebAssembly implementation
            return await this.registerWasm(registrationData);
        } else {
            // Use proxy implementation
            return await this.registerProxy(registrationData);
        }
    }
    
    async registerWasm(data) {
        // For now, simulate registration since we need actual DNS server integration
        console.log(`WASM: Registered ${data.coordinate} with capabilities:`, data.capabilities);
        return { success: true, message: 'Registration simulated (WASM)' };
    }
    
    async registerProxy(data) {
        try {
            const response = await this.fetchWithTimeout(`${this.options.proxyUrl}/register`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(data),
                timeout: this.options.timeout
            });
            
            if (!response.ok) {
                throw new TupleDNSError(-2, `Registration failed: ${response.statusText}`);
            }
            
            return await response.json();
        } catch (error) {
            throw new TupleDNSError(-2, `Registration error: ${error.message}`);
        }
    }
    
    async find(pattern, capabilities = null) {
        if (!this.initialized) {
            await this.init();
        }
        
        const cacheKey = `find:${pattern}:${JSON.stringify(capabilities)}`;
        
        if (this.options.enableCaching && this.cache.has(cacheKey)) {
            const cached = this.cache.get(cacheKey);
            if (Date.now() - cached.timestamp < 30000) { // 30s cache
                return cached.result;
            }
        }
        
        let result;
        const startTime = performance.now();
        
        if (this.wasmModule) {
            result = await this.findWasm(pattern, capabilities);
        } else {
            result = await this.findProxy(pattern, capabilities);
        }
        
        result.queryTime = (performance.now() - startTime) / 1000;
        
        if (this.options.enableCaching) {
            this.cache.set(cacheKey, {
                result: result,
                timestamp: Date.now()
            });
        }
        
        return new TupleResult(result);
    }
    
    async findWasm(pattern, capabilities) {
        // Simulate discovery for now
        const mockNodes = [];
        
        // Add some mock data for demonstration
        if (pattern.includes('music')) {
            mockNodes.push({
                coordinate: 'ambient.120.experimental.music.tuple',
                ipAddress: '192.168.1.100',
                capabilities: ['midi', 'real-time', 'generative'],
                ttl: 300,
                lastSeen: Date.now()
            });
        }
        
        // Filter by capabilities if specified
        let filteredNodes = mockNodes;
        if (capabilities && capabilities.length > 0) {
            filteredNodes = mockNodes.filter(node => 
                capabilities.every(cap => node.capabilities.includes(cap))
            );
        }
        
        return {
            nodes: filteredNodes,
            totalQueries: 1,
            queryTime: 0,
            error: filteredNodes.length > 0 ? 0 : -6
        };
    }
    
    async findProxy(pattern, capabilities) {
        try {
            const queryParams = new URLSearchParams({
                pattern: pattern
            });
            
            if (capabilities && capabilities.length > 0) {
                queryParams.append('capabilities', capabilities.join(','));
            }
            
            const response = await this.fetchWithTimeout(
                `${this.options.proxyUrl}/find?${queryParams}`,
                {
                    method: 'GET',
                    timeout: this.options.timeout
                }
            );
            
            if (!response.ok) {
                throw new TupleDNSError(-2, `Discovery failed: ${response.statusText}`);
            }
            
            return await response.json();
        } catch (error) {
            throw new TupleDNSError(-2, `Discovery error: ${error.message}`);
        }
    }
    
    async findRange(pattern, ranges) {
        if (!ranges || typeof ranges !== 'object') {
            throw new TupleDNSError(-4, 'Invalid ranges parameter');
        }
        
        // Expand ranges into multiple patterns
        const patterns = this.expandRangePatterns(pattern, ranges);
        
        // Execute multiple queries in parallel
        const promises = patterns.map(p => this.find(p));
        const results = await Promise.all(promises);
        
        // Merge results and deduplicate
        const allNodes = [];
        const seenCoordinates = new Set();
        let totalQueries = 0;
        let maxQueryTime = 0;
        
        for (const result of results) {
            totalQueries += result.totalQueries;
            maxQueryTime = Math.max(maxQueryTime, result.queryTime);
            
            for (const node of result.nodes) {
                if (!seenCoordinates.has(node.coordinate)) {
                    seenCoordinates.add(node.coordinate);
                    allNodes.push(node);
                }
            }
        }
        
        return new TupleResult({
            nodes: allNodes,
            totalQueries,
            queryTime: maxQueryTime,
            error: allNodes.length > 0 ? 0 : -6
        });
    }
    
    expandRangePatterns(pattern, ranges) {
        const patterns = [pattern];
        
        for (const [dimension, range] of Object.entries(ranges)) {
            const placeholder = `{${dimension}}`;
            if (!pattern.includes(placeholder)) {
                continue;
            }
            
            const newPatterns = [];
            const [min, max] = Array.isArray(range) ? range : [range.min, range.max];
            
            for (const currentPattern of patterns) {
                for (let value = min; value <= max; value++) {
                    newPatterns.push(currentPattern.replace(placeholder, value.toString()));
                }
            }
            
            patterns.splice(0, patterns.length, ...newPatterns);
        }
        
        return patterns;
    }
    
    async searchMulti(patterns) {
        if (!Array.isArray(patterns) || patterns.length === 0) {
            throw new TupleDNSError(-4, 'Invalid patterns array');
        }
        
        // Execute all patterns in parallel
        const promises = patterns.map(pattern => this.find(pattern));
        const results = await Promise.all(promises);
        
        // Merge results
        const allNodes = [];
        const seenCoordinates = new Set();
        let totalQueries = 0;
        let maxQueryTime = 0;
        
        for (const result of results) {
            totalQueries += result.totalQueries;
            maxQueryTime = Math.max(maxQueryTime, result.queryTime);
            
            for (const node of result.nodes) {
                if (!seenCoordinates.has(node.coordinate)) {
                    seenCoordinates.add(node.coordinate);
                    allNodes.push(node);
                }
            }
        }
        
        return new TupleResult({
            nodes: allNodes,
            totalQueries,
            queryTime: maxQueryTime,
            error: allNodes.length > 0 ? 0 : -6
        });
    }
    
    async getClientIP() {
        if (typeof window !== 'undefined') {
            // Browser environment - use a public IP service
            try {
                const response = await fetch('https://api.ipify.org?format=json');
                const data = await response.json();
                return data.ip;
            } catch {
                return '127.0.0.1'; // Fallback
            }
        } else {
            // Node.js environment
            const os = require('os');
            const networkInterfaces = os.networkInterfaces();
            
            for (const interfaceName in networkInterfaces) {
                const addresses = networkInterfaces[interfaceName];
                for (const address of addresses) {
                    if (address.family === 'IPv4' && !address.internal) {
                        return address.address;
                    }
                }
            }
            
            return '127.0.0.1'; // Fallback
        }
    }
    
    async fetchWithTimeout(url, options = {}) {
        const timeout = options.timeout || this.options.timeout;
        
        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), timeout);
        
        try {
            const response = await fetch(url, {
                ...options,
                signal: controller.signal
            });
            clearTimeout(timeoutId);
            return response;
        } catch (error) {
            clearTimeout(timeoutId);
            if (error.name === 'AbortError') {
                throw new TupleDNSError(-5, 'Request timeout');
            }
            throw error;
        }
    }
    
    cleanup() {
        if (this.wasmModule && this.wasmModule.tupledns_cleanup) {
            this.wasmModule.tupledns_cleanup();
        }
        this.cache.clear();
        this.initialized = false;
    }
}

// Convenience functions
async function register(coordinate, capabilities = [], ttl = 300) {
    const dns = new TupleDNS();
    await dns.init();
    return dns.register(coordinate, capabilities, ttl);
}

async function find(pattern, capabilities = null) {
    const dns = new TupleDNS();
    await dns.init();
    return dns.find(pattern, capabilities);
}

async function findNearby(coordinate, capabilities = null, radius = 1) {
    const dns = new TupleDNS();
    const decoded = dns.decodeCoordinate(coordinate);
    
    // Create a pattern for nearby search
    const values = decoded.values.slice();
    values[values.length - 1] = '*'; // Make the most specific part a wildcard
    const pattern = dns.encodeCoordinate(decoded.spaceType, values);
    
    return dns.find(pattern, capabilities);
}

// Export for different environments
if (typeof module !== 'undefined' && module.exports) {
    // Node.js
    module.exports = {
        TupleDNS,
        TupleNode,
        TupleResult,
        TupleDNSError,
        register,
        find,
        findNearby
    };
} else if (typeof window !== 'undefined') {
    // Browser
    window.TupleDNS = {
        TupleDNS,
        TupleNode,
        TupleResult,
        TupleDNSError,
        register,
        find,
        findNearby
    };
}

// Example usage and testing
if (typeof window !== 'undefined' && window.location.pathname.includes('test')) {
    // Browser test mode
    (async () => {
        console.log('🧪 TupleDNS Browser Test');
        
        try {
            const dns = new TupleDNS();
            await dns.init();
            
            // Test coordinate validation
            console.log('✅ Coordinate validation:', dns.validateCoordinate('test.music.tuple'));
            console.log('❌ Invalid coordinate:', dns.validateCoordinate('invalid'));
            
            // Test coordinate encoding/decoding
            const coord = dns.encodeCoordinate('music', ['ambient', '120', 'experimental']);
            console.log('📍 Encoded coordinate:', coord);
            
            const decoded = dns.decodeCoordinate(coord);
            console.log('🔍 Decoded:', decoded);
            
            // Test pattern matching
            console.log('🎯 Pattern match:', dns.matchPattern(coord, '*.120.*.music.tuple'));
            
            // Test registration (mock)
            await dns.register(coord, ['midi', 'real-time']);
            console.log('📝 Registration completed');
            
            // Test discovery (mock)
            const result = await dns.find('*.music.tuple');
            console.log('🔍 Discovery result:', result);
            
            console.log('🎉 All tests completed!');
            
        } catch (error) {
            console.error('❌ Test failed:', error);
        }
    })();
}