#!/usr/bin/env node
/**
 * TupleDNS HTTP Proxy Server
 * 
 * Provides HTTP API for TupleDNS operations in CORS-restricted environments
 * Enables web browsers to access TupleDNS functionality
 */

const http = require('http');
const url = require('url');
const dns = require('dns');
const { promisify } = require('util');

// Promisify DNS functions
const resolveA = promisify(dns.resolve4);
const resolveTxt = promisify(dns.resolveTxt);

class TupleDNSProxy {
    constructor(options = {}) {
        this.port = options.port || 3000;
        this.host = options.host || 'localhost';
        this.corsOrigins = options.corsOrigins || ['*'];
        this.cache = new Map();
        this.cacheTTL = options.cacheTTL || 30000; // 30 seconds
        
        // In-memory registry for demo purposes
        this.registry = new Map();
    }
    
    start() {
        const server = http.createServer((req, res) => {
            this.handleRequest(req, res);
        });
        
        server.listen(this.port, this.host, () => {
            console.log(`🌐 TupleDNS Proxy Server running at http://${this.host}:${this.port}`);
            console.log(`📡 CORS origins: ${this.corsOrigins.join(', ')}`);
            console.log(`🎯 Ready to proxy TupleDNS requests!`);
        });
        
        return server;
    }
    
    async handleRequest(req, res) {
        // Enable CORS
        this.setCorsHeaders(res);
        
        // Handle preflight requests
        if (req.method === 'OPTIONS') {
            res.writeHead(200);
            res.end();
            return;
        }
        
        const parsedUrl = url.parse(req.url, true);
        const path = parsedUrl.pathname;
        const query = parsedUrl.query;
        
        try {
            switch (path) {
                case '/health':
                    await this.handleHealth(req, res);
                    break;
                case '/register':
                    await this.handleRegister(req, res);
                    break;
                case '/find':
                    await this.handleFind(req, res, query);
                    break;
                case '/unregister':
                    await this.handleUnregister(req, res);
                    break;
                case '/status':
                    await this.handleStatus(req, res);
                    break;
                default:
                    this.sendError(res, 404, 'Endpoint not found');
            }
        } catch (error) {
            console.error('Request error:', error);
            this.sendError(res, 500, error.message);
        }
    }
    
    setCorsHeaders(res) {
        res.setHeader('Access-Control-Allow-Origin', '*');
        res.setHeader('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
        res.setHeader('Access-Control-Allow-Headers', 'Content-Type, Authorization');
    }
    
    async handleHealth(req, res) {
        this.sendJSON(res, 200, {
            status: 'healthy',
            service: 'TupleDNS Proxy',
            version: '1.0.0',
            timestamp: new Date().toISOString(),
            uptime: process.uptime(),
            registrations: this.registry.size
        });
    }
    
    async handleRegister(req, res) {
        const data = await this.parseRequestBody(req);
        
        if (!data.coordinate) {
            return this.sendError(res, 400, 'Missing coordinate');
        }
        
        if (!this.validateCoordinate(data.coordinate)) {
            return this.sendError(res, 400, 'Invalid coordinate format');
        }
        
        // Store in memory registry (in production, this would update DNS)
        this.registry.set(data.coordinate, {
            coordinate: data.coordinate,
            ipAddress: data.ipAddress || this.getClientIP(req),
            capabilities: data.capabilities || [],
            ttl: data.ttl || 300,
            registeredAt: Date.now()
        });
        
        console.log(`📝 Registered: ${data.coordinate} with capabilities: ${data.capabilities?.join(', ')}`);
        
        this.sendJSON(res, 200, {
            success: true,
            message: 'Registration successful',
            coordinate: data.coordinate
        });
    }
    
    async handleFind(req, res, query) {
        const pattern = query.pattern;
        const capabilities = query.capabilities ? query.capabilities.split(',') : null;
        
        if (!pattern) {
            return this.sendError(res, 400, 'Missing pattern parameter');
        }
        
        console.log(`🔍 Finding nodes: pattern=${pattern}, capabilities=${capabilities?.join(',') || 'any'}`);
        
        const startTime = Date.now();
        
        // Search in memory registry
        const nodes = [];
        for (const [coord, nodeData] of this.registry.entries()) {
            if (this.matchPattern(coord, pattern)) {
                // Filter by capabilities if specified
                if (!capabilities || this.hasAllCapabilities(nodeData.capabilities, capabilities)) {
                    nodes.push({
                        coordinate: nodeData.coordinate,
                        ipAddress: nodeData.ipAddress,
                        capabilities: nodeData.capabilities,
                        ttl: nodeData.ttl,
                        lastSeen: nodeData.registeredAt
                    });
                }
            }
        }
        
        // Try actual DNS resolution for demonstration
        try {
            await this.tryDNSResolution(pattern);
        } catch (error) {
            // DNS resolution failed, which is expected without actual TupleDNS records
            console.log(`📡 DNS resolution for ${pattern}: ${error.message}`);
        }
        
        const queryTime = (Date.now() - startTime) / 1000;
        
        console.log(`✅ Found ${nodes.length} nodes in ${queryTime}s`);
        
        this.sendJSON(res, 200, {
            nodes: nodes,
            totalQueries: 1,
            queryTime: queryTime,
            error: nodes.length > 0 ? 0 : -6
        });
    }
    
    async handleUnregister(req, res) {
        const data = await this.parseRequestBody(req);
        
        if (!data.coordinate) {
            return this.sendError(res, 400, 'Missing coordinate');
        }
        
        if (this.registry.has(data.coordinate)) {
            this.registry.delete(data.coordinate);
            console.log(`🗑️ Unregistered: ${data.coordinate}`);
            
            this.sendJSON(res, 200, {
                success: true,
                message: 'Unregistration successful'
            });
        } else {
            this.sendError(res, 404, 'Coordinate not found');
        }
    }
    
    async handleStatus(req, res) {
        const registrations = Array.from(this.registry.entries()).map(([coord, data]) => ({
            coordinate: coord,
            capabilities: data.capabilities,
            registeredAt: new Date(data.registeredAt).toISOString(),
            ttl: data.ttl
        }));
        
        this.sendJSON(res, 200, {
            totalRegistrations: this.registry.size,
            registrations: registrations,
            cacheSize: this.cache.size,
            server: {
                uptime: process.uptime(),
                memory: process.memoryUsage(),
                timestamp: new Date().toISOString()
            }
        });
    }
    
    async tryDNSResolution(pattern) {
        // Try to resolve the pattern as a hostname
        // This would work if actual TupleDNS records existed
        const hostname = pattern.replace(/\*/g, 'test');
        
        try {
            const addresses = await resolveA(hostname);
            console.log(`📍 DNS A record for ${hostname}:`, addresses);
            return addresses;
        } catch (error) {
            // Try TXT records for capabilities
            try {
                const txtRecords = await resolveTxt(hostname);
                console.log(`📄 DNS TXT records for ${hostname}:`, txtRecords);
                return txtRecords;
            } catch (txtError) {
                throw new Error(`No DNS records found for ${hostname}`);
            }
        }
    }
    
    validateCoordinate(coordinate) {
        if (typeof coordinate !== 'string' || coordinate.length === 0) {
            return false;
        }
        
        if (coordinate.length > 253) {
            return false;
        }
        
        if (!coordinate.endsWith('.tuple')) {
            return false;
        }
        
        const validChars = /^[a-zA-Z0-9.-]+$/;
        return validChars.test(coordinate);
    }
    
    matchPattern(coordinate, pattern) {
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
    
    hasAllCapabilities(nodeCapabilities, requiredCapabilities) {
        return requiredCapabilities.every(cap => nodeCapabilities.includes(cap));
    }
    
    getClientIP(req) {
        return req.headers['x-forwarded-for'] || 
               req.connection.remoteAddress || 
               req.socket.remoteAddress ||
               '127.0.0.1';
    }
    
    async parseRequestBody(req) {
        return new Promise((resolve, reject) => {
            let body = '';
            req.on('data', chunk => {
                body += chunk.toString();
            });
            req.on('end', () => {
                try {
                    resolve(body ? JSON.parse(body) : {});
                } catch (error) {
                    reject(new Error('Invalid JSON'));
                }
            });
        });
    }
    
    sendJSON(res, statusCode, data) {
        res.writeHead(statusCode, { 'Content-Type': 'application/json' });
        res.end(JSON.stringify(data, null, 2));
    }
    
    sendError(res, statusCode, message) {
        this.sendJSON(res, statusCode, {
            error: true,
            message: message,
            timestamp: new Date().toISOString()
        });
    }
}

// CLI interface
if (require.main === module) {
    const args = process.argv.slice(2);
    const options = {};
    
    // Parse command line arguments
    for (let i = 0; i < args.length; i++) {
        switch (args[i]) {
            case '--port':
            case '-p':
                options.port = parseInt(args[++i]);
                break;
            case '--host':
            case '-h':
                options.host = args[++i];
                break;
            case '--help':
                console.log(`
TupleDNS HTTP Proxy Server

Usage: node proxy-server.js [options]

Options:
  -p, --port <port>    Server port (default: 3000)
  -h, --host <host>    Server host (default: localhost)
  --help               Show this help message

Examples:
  node proxy-server.js
  node proxy-server.js --port 8080 --host 0.0.0.0

API Endpoints:
  GET  /health         - Health check
  POST /register       - Register a coordinate
  GET  /find           - Find nodes by pattern
  POST /unregister     - Unregister a coordinate
  GET  /status         - Server status and registrations
`);
                process.exit(0);
                break;
        }
    }
    
    const proxy = new TupleDNSProxy(options);
    proxy.start();
    
    // Graceful shutdown
    process.on('SIGINT', () => {
        console.log('\n🛑 Shutting down TupleDNS Proxy Server...');
        process.exit(0);
    });
    
    // Example registrations for testing
    setTimeout(() => {
        console.log('\n🧪 Adding example registrations for testing...');
        
        const examples = [
            {
                coordinate: 'ambient.120.experimental.music.tuple',
                capabilities: ['midi', 'real-time', 'generative'],
                ipAddress: '192.168.1.100'
            },
            {
                coordinate: 'jazz.140.4-4.bb-major.music.tuple',
                capabilities: ['midi', 'live-recording', 'improvisation'],
                ipAddress: '192.168.1.101'
            },
            {
                coordinate: 'coffee-maker.kitchen.floor-1.building-5.spatial.tuple',
                capabilities: ['brew', 'schedule', 'iot'],
                ipAddress: '192.168.1.50'
            }
        ];
        
        const proxy_instance = proxy;
        examples.forEach(example => {
            proxy_instance.registry.set(example.coordinate, {
                ...example,
                ttl: 300,
                registeredAt: Date.now()
            });
            console.log(`📝 Example: ${example.coordinate}`);
        });
        
        console.log(`✅ ${examples.length} example registrations added`);
        console.log('🔍 Try: GET /find?pattern=*.music.tuple');
        console.log('🔍 Try: GET /find?pattern=*.kitchen.*.spatial.tuple');
    }, 1000);
}

module.exports = TupleDNSProxy;