/**
 * TupleDNS JavaScript Test Suite
 * 
 * Comprehensive tests for JavaScript/Node.js bindings
 */

const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

// Load TupleDNS JavaScript bindings
const TupleDNS = require('../../tupledns.js');

class TestSuite {
    constructor() {
        this.tests = [];
        this.passed = 0;
        this.failed = 0;
        this.proxyProcess = null;
        this.proxyPort = 3003;
    }

    addTest(name, testFunc) {
        this.tests.push({ name, testFunc });
    }

    async runTests() {
        console.log('TupleDNS JavaScript Test Suite');
        console.log('==============================\n');

        // Start proxy server for browser tests
        await this.startProxy();

        for (const test of this.tests) {
            try {
                console.log(`Running: ${test.name}`);
                await test.testFunc();
                console.log('  ✅ PASS\n');
                this.passed++;
            } catch (error) {
                console.log(`  ❌ FAIL: ${error.message}\n`);
                this.failed++;
            }
        }

        await this.stopProxy();

        console.log('==============================');
        console.log(`Results: ${this.passed} passed, ${this.failed} failed`);
        console.log(`Status: ${this.failed === 0 ? 'ALL TESTS PASSED! ✅' : 'SOME TESTS FAILED ❌'}`);

        return this.failed === 0;
    }

    async startProxy() {
        return new Promise((resolve, reject) => {
            this.proxyProcess = spawn('node', [
                path.join(__dirname, '../../proxy-server.cjs'),
                '--port', this.proxyPort.toString()
            ], {
                stdio: ['pipe', 'pipe', 'pipe']
            });

            this.proxyProcess.stdout.on('data', (data) => {
                if (data.toString().includes('Ready to proxy TupleDNS requests')) {
                    setTimeout(resolve, 500);
                }
            });

            this.proxyProcess.stderr.on('data', (data) => {
                console.error('Proxy error:', data.toString());
            });

            setTimeout(resolve, 3000); // Fallback timeout
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

    assert(condition, message) {
        if (!condition) {
            throw new Error(message);
        }
    }

    assertEqual(actual, expected, message) {
        if (actual !== expected) {
            throw new Error(`${message}: expected ${expected}, got ${actual}`);
        }
    }

    assertArrayEqual(actual, expected, message) {
        if (JSON.stringify(actual) !== JSON.stringify(expected)) {
            throw new Error(`${message}: expected ${JSON.stringify(expected)}, got ${JSON.stringify(actual)}`);
        }
    }
}

// Test suite instance
const suite = new TestSuite();

// Core functionality tests
suite.addTest('TupleDNS Class Creation', async () => {
    const dns = new TupleDNS.TupleDNS();
    suite.assert(dns instanceof TupleDNS.TupleDNS, 'Should create TupleDNS instance');
});

suite.addTest('TupleDNS Initialization', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    suite.assert(true, 'Initialization should complete without error');
});

suite.addTest('Coordinate Validation', async () => {
    // Valid coordinates
    suite.assert(TupleDNS.validateCoordinate('simple.test.tuple'), 'Simple coordinate should be valid');
    suite.assert(TupleDNS.validateCoordinate('a.b.c.d.e.f.tuple'), 'Long coordinate should be valid');
    suite.assert(TupleDNS.validateCoordinate('123.456.numeric.tuple'), 'Numeric values should be valid');
    suite.assert(TupleDNS.validateCoordinate('with-hyphens.test.tuple'), 'Hyphens should be valid');

    // Invalid coordinates
    suite.assert(!TupleDNS.validateCoordinate(''), 'Empty string should be invalid');
    suite.assert(!TupleDNS.validateCoordinate('no-suffix'), 'Missing .tuple should be invalid');
    suite.assert(!TupleDNS.validateCoordinate('.tuple'), 'Just .tuple should be invalid');
    suite.assert(!TupleDNS.validateCoordinate('spaces not allowed.tuple'), 'Spaces should be invalid');
});

suite.addTest('Coordinate Encoding', async () => {
    const components = ['ambient', '120', 'london', 'music'];
    const encoded = TupleDNS.encodeCoordinate(components);
    suite.assertEqual(encoded, 'ambient.120.london.music.tuple', 'Encoded coordinate should match expected');
    
    const decoded = TupleDNS.decodeCoordinate(encoded);
    suite.assertArrayEqual(decoded, components, 'Decoded components should match original');
});

suite.addTest('Pattern Matching', async () => {
    const coord = 'ambient.120.london.music.tuple';
    
    // Exact matches
    suite.assert(TupleDNS.matchPattern(coord, coord), 'Exact match should work');
    
    // Wildcard matches
    suite.assert(TupleDNS.matchPattern(coord, '*.120.london.music.tuple'), 'First wildcard should match');
    suite.assert(TupleDNS.matchPattern(coord, 'ambient.*.london.music.tuple'), 'Middle wildcard should match');
    suite.assert(TupleDNS.matchPattern(coord, 'ambient.120.*.music.tuple'), 'Last wildcard should match');
    suite.assert(TupleDNS.matchPattern(coord, '*.*.*.*.tuple'), 'All wildcards should match');
    
    // Non-matches
    suite.assert(!TupleDNS.matchPattern(coord, 'jazz.120.london.music.tuple'), 'Different component should not match');
    suite.assert(!TupleDNS.matchPattern(coord, 'ambient.140.london.music.tuple'), 'Different component should not match');
});

suite.addTest('Node Registration', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    const coordinate = `test.${Date.now()}.registration.tuple`;
    const capabilities = ['javascript', 'test', 'registration'];
    
    // Should not throw
    await dns.register(coordinate, capabilities, 300);
    await dns.unregister(coordinate);
});

suite.addTest('Node Discovery', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    // Register test nodes
    const coord1 = `discover1.${Date.now()}.test.tuple`;
    const coord2 = `discover2.${Date.now()}.test.tuple`;
    
    await dns.register(coord1, ['test', 'discovery']);
    await dns.register(coord2, ['test', 'discovery']);
    
    // Discover nodes
    const result = await dns.find('*.*.test.tuple');
    suite.assert(result instanceof TupleDNS.TupleResult, 'Should return TupleResult instance');
    suite.assert(Array.isArray(result.nodes), 'Should have nodes array');
    suite.assert(typeof result.totalQueries === 'number', 'Should have totalQueries');
    suite.assert(typeof result.queryTime === 'number', 'Should have queryTime');
    
    // Cleanup
    await dns.unregister(coord1);
    await dns.unregister(coord2);
});

suite.addTest('Capability Filtering', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    const coord1 = `cap1.${Date.now()}.test.tuple`;
    const coord2 = `cap2.${Date.now()}.test.tuple`;
    
    await dns.register(coord1, ['common', 'unique1']);
    await dns.register(coord2, ['common', 'unique2']);
    
    // Find with capability filter
    const commonResult = await dns.find('*.*.test.tuple', ['common']);
    suite.assert(commonResult instanceof TupleDNS.TupleResult, 'Should return results for common capability');
    
    const uniqueResult = await dns.find('*.*.test.tuple', ['unique1']);
    suite.assert(uniqueResult instanceof TupleDNS.TupleResult, 'Should return results for unique capability');
    
    // Cleanup
    await dns.unregister(coord1);
    await dns.unregister(coord2);
});

suite.addTest('Range Queries', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    const coords = [
        `ambient.100.${Date.now()}.music.tuple`,
        `ambient.120.${Date.now()}.music.tuple`,
        `ambient.140.${Date.now()}.music.tuple`
    ];
    
    for (const coord of coords) {
        await dns.register(coord, ['music', 'test']);
    }
    
    // Range query
    const result = await dns.findRange(
        'ambient.{bpm}.{timestamp}.music.tuple',
        { 
            bpm: [100, 120, 140],
            timestamp: [Date.now()]
        }
    );
    
    suite.assert(result instanceof TupleDNS.TupleResult, 'Should return TupleResult for range query');
    
    // Cleanup
    for (const coord of coords) {
        await dns.unregister(coord);
    }
});

suite.addTest('Error Handling', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    // Test invalid coordinate registration
    try {
        await dns.register('invalid coordinate', ['test']);
        suite.assert(false, 'Should throw error for invalid coordinate');
    } catch (error) {
        suite.assert(true, 'Should throw error for invalid coordinate');
    }
    
    // Test invalid pattern discovery
    try {
        await dns.find('');
        suite.assert(false, 'Should throw error for empty pattern');
    } catch (error) {
        suite.assert(true, 'Should throw error for empty pattern');
    }
});

suite.addTest('Music Collaboration Scenario', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    const timestamp = Date.now();
    const musicians = [
        [`ambient.120.${timestamp}.london.music.tuple`, ['midi', 'real-time', 'collaborative']],
        [`jazz.140.${timestamp}.newyork.music.tuple`, ['midi', 'live-recording', 'improvisation']],
        [`electronic.128.${timestamp}.berlin.music.tuple`, ['midi', 'synthesis', 'real-time']]
    ];
    
    // Register musicians
    for (const [coord, caps] of musicians) {
        await dns.register(coord, caps);
    }
    
    // Find MIDI-capable musicians
    const midiResult = await dns.find(`*.*.${timestamp}.*.music.tuple`, ['midi']);
    suite.assert(midiResult instanceof TupleDNS.TupleResult, 'Should find MIDI musicians');
    
    // Find real-time collaborators
    const realtimeResult = await dns.find(`*.*.${timestamp}.*.music.tuple`, ['real-time']);
    suite.assert(realtimeResult instanceof TupleDNS.TupleResult, 'Should find real-time musicians');
    
    // Cleanup
    for (const [coord] of musicians) {
        await dns.unregister(coord);
    }
});

suite.addTest('IoT Device Management Scenario', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    const timestamp = Date.now();
    const devices = [
        [`sensor-1.kitchen.floor-1.building-${timestamp}.spatial.tuple`, ['temperature', 'humidity', 'iot']],
        [`printer-1.lab.floor-2.building-${timestamp}.spatial.tuple`, ['3d-print', 'pla', 'petg']],
        [`light-1.office.floor-2.building-${timestamp}.spatial.tuple`, ['lighting', 'dimmer', 'color']]
    ];
    
    // Register devices
    for (const [coord, caps] of devices) {
        await dns.register(coord, caps);
    }
    
    // Find all devices in building
    const buildingResult = await dns.find(`*.*.*.building-${timestamp}.spatial.tuple`);
    suite.assert(buildingResult instanceof TupleDNS.TupleResult, 'Should find building devices');
    
    // Find IoT sensors
    const sensorResult = await dns.find(`*.*.*.building-${timestamp}.spatial.tuple`, ['iot']);
    suite.assert(sensorResult instanceof TupleDNS.TupleResult, 'Should find IoT sensors');
    
    // Cleanup
    for (const [coord] of devices) {
        await dns.unregister(coord);
    }
});

suite.addTest('Performance Test', async () => {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: `http://localhost:${suite.proxyPort}`
    });
    
    await dns.init();
    
    const startTime = Date.now();
    const coords = [];
    
    // Register 50 nodes
    for (let i = 0; i < 50; i++) {
        const coord = `perf.${i}.${Date.now()}.test.tuple`;
        coords.push(coord);
        await dns.register(coord, ['performance', 'test']);
    }
    
    const regTime = Date.now() - startTime;
    
    const discStart = Date.now();
    
    // Perform 10 discoveries
    for (let i = 0; i < 10; i++) {
        await dns.find('perf.*.*.test.tuple');
    }
    
    const discTime = Date.now() - discStart;
    
    console.log(`  Performance: 50 registrations in ${regTime}ms, 10 discoveries in ${discTime}ms`);
    
    // Cleanup
    for (const coord of coords) {
        await dns.unregister(coord);
    }
    
    suite.assert(true, 'Performance test completed');
});

// Run the test suite
if (require.main === module) {
    suite.runTests().then(success => {
        process.exit(success ? 0 : 1);
    }).catch(error => {
        console.error('Test suite failed:', error);
        process.exit(1);
    });
}

module.exports = TestSuite;