#!/usr/bin/env node
/**
 * TupleDNS Browser Integration Test
 * 
 * Tests the JavaScript bindings and proxy server integration
 */

const http = require('http');
const { spawn } = require('child_process');
const fs = require('fs');

class TupleDNSBrowserTest {
    constructor() {
        this.proxyProcess = null;
        this.proxyPort = 3002;
        this.baseUrl = `http://localhost:${this.proxyPort}`;
    }
    
    async startProxy() {
        return new Promise((resolve, reject) => {
            this.proxyProcess = spawn('node', ['proxy-server.cjs', '--port', this.proxyPort.toString()], {
                stdio: ['pipe', 'pipe', 'pipe']
            });
            
            this.proxyProcess.stdout.on('data', (data) => {
                const output = data.toString();
                console.log('🔧 Proxy:', output.trim());
                if (output.includes('Ready to proxy TupleDNS requests')) {
                    setTimeout(resolve, 500); // Give it a moment to fully start
                }
            });
            
            this.proxyProcess.stderr.on('data', (data) => {
                console.error('❌ Proxy Error:', data.toString().trim());
            });
            
            this.proxyProcess.on('error', reject);
            
            // Timeout fallback
            setTimeout(() => resolve(), 3000);
        });
    }
    
    async stopProxy() {
        if (this.proxyProcess) {
            this.proxyProcess.kill();
            await new Promise(resolve => {
                this.proxyProcess.on('exit', resolve);
                setTimeout(resolve, 1000);
            });
        }
    }
    
    async makeRequest(path, options = {}) {
        return new Promise((resolve, reject) => {
            const url = `${this.baseUrl}${path}`;
            const method = options.method || 'GET';
            const body = options.body ? JSON.stringify(options.body) : null;
            
            const req = http.request(url, {
                method,
                headers: {
                    'Content-Type': 'application/json',
                    ...options.headers
                }
            }, (res) => {
                let data = '';
                res.on('data', chunk => data += chunk);
                res.on('end', () => {
                    try {
                        const parsed = JSON.parse(data);
                        resolve({ status: res.statusCode, data: parsed });
                    } catch (error) {
                        resolve({ status: res.statusCode, data: data });
                    }
                });
            });
            
            req.on('error', reject);
            
            if (body) {
                req.write(body);
            }
            
            req.end();
        });
    }
    
    async testHealthCheck() {
        console.log('\n🏥 Testing Health Check...');
        const response = await this.makeRequest('/health');
        
        if (response.status === 200 && response.data.status === 'healthy') {
            console.log('✅ Health check passed');
            console.log(`   Service: ${response.data.service}`);
            console.log(`   Registrations: ${response.data.registrations}`);
            return true;
        } else {
            console.log('❌ Health check failed');
            return false;
        }
    }
    
    async testRegistration() {
        console.log('\n📝 Testing Node Registration...');
        
        const testNode = {
            coordinate: 'test.browser.123.demo.tuple',
            capabilities: ['javascript', 'browser', 'demo'],
            ipAddress: '127.0.0.1',
            ttl: 300
        };
        
        const response = await this.makeRequest('/register', {
            method: 'POST',
            body: testNode
        });
        
        if (response.status === 200 && response.data.success) {
            console.log('✅ Registration successful');
            console.log(`   Coordinate: ${response.data.coordinate}`);
            return true;
        } else {
            console.log('❌ Registration failed:', response.data);
            return false;
        }
    }
    
    async testDiscovery() {
        console.log('\n🔍 Testing Node Discovery...');
        
        // Test pattern-based discovery
        const response = await this.makeRequest('/find?pattern=*.tuple');
        
        if (response.status === 200) {
            console.log('✅ Discovery successful');
            console.log(`   Found ${response.data.nodes.length} nodes`);
            console.log(`   Query time: ${response.data.queryTime}s`);
            
            if (response.data.nodes.length > 0) {
                console.log('   Sample nodes:');
                response.data.nodes.slice(0, 2).forEach((node, i) => {
                    console.log(`     ${i+1}. ${node.coordinate}`);
                    console.log(`        Capabilities: ${node.capabilities.join(', ')}`);
                });
            }
            return true;
        } else {
            console.log('❌ Discovery failed:', response.data);
            return false;
        }
    }
    
    async testCapabilityFiltering() {
        console.log('\n🎯 Testing Capability Filtering...');
        
        const response = await this.makeRequest('/find?pattern=*.tuple&capabilities=javascript');
        
        if (response.status === 200) {
            console.log('✅ Capability filtering successful');
            console.log(`   Found ${response.data.nodes.length} nodes with 'javascript' capability`);
            return true;
        } else {
            console.log('❌ Capability filtering failed:', response.data);
            return false;
        }
    }
    
    async testJavaScriptBindings() {
        console.log('\n🌐 Testing JavaScript Bindings...');
        
        try {
            // Load and test the JavaScript bindings
            const jsCode = fs.readFileSync('tupledns.js', 'utf8');
            
            // Basic validation that the file contains expected classes
            const hasClasses = [
                'class TupleDNS',
                'class TupleNode', 
                'class TupleResult',
                'validateCoordinate',
                'encodeCoordinate',
                'matchPattern'
            ].every(pattern => jsCode.includes(pattern));
            
            if (hasClasses) {
                console.log('✅ JavaScript bindings structure valid');
                console.log('   - TupleDNS class found');
                console.log('   - TupleNode class found');
                console.log('   - Core methods found');
                return true;
            } else {
                console.log('❌ JavaScript bindings incomplete');
                return false;
            }
        } catch (error) {
            console.log('❌ JavaScript bindings test failed:', error.message);
            return false;
        }
    }
    
    async testDemoPage() {
        console.log('\n📄 Testing Demo Page...');
        
        try {
            const htmlContent = fs.readFileSync('demo.html', 'utf8');
            
            const hasFeatures = [
                'TupleDNS Browser Demo',
                'Register Node',
                'Discover Nodes',
                'Quick Examples',
                'script src="tupledns.js"'
            ].every(pattern => htmlContent.includes(pattern));
            
            if (hasFeatures) {
                console.log('✅ Demo page structure valid');
                console.log('   - Registration form found');
                console.log('   - Discovery interface found');
                console.log('   - Example scenarios included');
                console.log(`   📱 Open in browser: file://${process.cwd()}/demo.html`);
                return true;
            } else {
                console.log('❌ Demo page incomplete');
                return false;
            }
        } catch (error) {
            console.log('❌ Demo page test failed:', error.message);
            return false;
        }
    }
    
    async runAllTests() {
        console.log('🧪 TupleDNS Browser Integration Test Suite');
        console.log('===========================================');
        
        try {
            // Start proxy server
            console.log('🚀 Starting proxy server...');
            await this.startProxy();
            
            // Run tests
            const tests = [
                this.testHealthCheck(),
                this.testRegistration(),
                this.testDiscovery(),
                this.testCapabilityFiltering(),
                this.testJavaScriptBindings(),
                this.testDemoPage()
            ];
            
            const results = await Promise.all(tests);
            const passed = results.filter(r => r).length;
            const total = results.length;
            
            console.log('\n📊 Test Results');
            console.log('================');
            console.log(`✅ Passed: ${passed}/${total}`);
            console.log(`❌ Failed: ${total - passed}/${total}`);
            
            if (passed === total) {
                console.log('\n🎉 ALL TESTS PASSED!');
                console.log('🌐 TupleDNS browser integration is ready!');
                console.log('\n🚀 Next Steps:');
                console.log('   1. Start proxy: node proxy-server.cjs');
                console.log(`   2. Open demo: file://${process.cwd()}/demo.html`);
                console.log('   3. Test multidimensional discovery in browser!');
            } else {
                console.log('\n⚠️ Some tests failed - check implementation');
            }
            
        } catch (error) {
            console.error('❌ Test suite failed:', error);
        } finally {
            // Cleanup
            await this.stopProxy();
        }
    }
}

// Run tests if called directly
if (require.main === module) {
    const tester = new TupleDNSBrowserTest();
    tester.runAllTests().catch(console.error);
}

module.exports = TupleDNSBrowserTest;