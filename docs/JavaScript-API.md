# TupleDNS JavaScript API Reference

JavaScript/Browser bindings for TupleDNS multidimensional discovery.

## Installation

### Browser Usage
```html
<!DOCTYPE html>
<html>
<head>
    <script src="tupledns.js"></script>
</head>
<body>
    <script>
        // TupleDNS classes are now available
        const dns = new TupleDNS.TupleDNS();
    </script>
</body>
</html>
```

### Node.js Usage
```javascript
const TupleDNS = require('./tupledns.js');
const dns = new TupleDNS.TupleDNS();
```

## Core Classes

### TupleDNS
Main class for TupleDNS operations.

```javascript
const dns = new TupleDNS.TupleDNS({
    proxyUrl: 'http://localhost:3000'  // Optional HTTP proxy
});

await dns.init();
```

### TupleNode
Represents a discovered node.

```javascript
class TupleNode {
    coordinate: string;
    ipAddress: string;
    capabilities: string[];
    ttl: number;
    lastSeen: number;
}
```

### TupleResult
Contains discovery results.

```javascript
class TupleResult {
    nodes: TupleNode[];
    totalQueries: number;
    queryTime: number;
}
```

## API Methods

### init(config?)
Initialize the TupleDNS client.

```javascript
await dns.init({
    timeout: 5000,
    retries: 3
});
```

### register(coordinate, capabilities?, ttl?)
Register a node at the specified coordinate.

```javascript
await dns.register(
    "ambient.120.london.music.tuple",
    ["midi", "real-time", "collaborative"],
    300
);
```

### find(pattern, capabilities?) → Promise<TupleResult>
Find nodes matching the pattern.

```javascript
const result = await dns.find("*.120.*.music.tuple");
console.log(`Found ${result.nodes.length} nodes`);
```

### findRange(pattern, ranges, capabilities?) → Promise<TupleResult>
Find nodes within dimensional ranges.

```javascript
const result = await dns.findRange(
    "ambient.{bpm}.london.music.tuple",
    { bpm: [110, 120, 130] },
    ["collaborative"]
);
```

### unregister(coordinate)
Remove registration for a coordinate.

```javascript
await dns.unregister("ambient.120.london.music.tuple");
```

## Utility Functions

### validateCoordinate(coordinate) → boolean
```javascript
const isValid = TupleDNS.validateCoordinate("ambient.120.london.music.tuple");
```

### encodeCoordinate(components) → string
```javascript
const coord = TupleDNS.encodeCoordinate(["ambient", "120", "london", "music"]);
```

### matchPattern(coordinate, pattern) → boolean
```javascript
const matches = TupleDNS.matchPattern(
    "ambient.120.london.music.tuple",
    "*.120.*.music.tuple"
);
```

## Browser Integration Examples

### Music Discovery Web App
```javascript
async function createMusicApp() {
    const dns = new TupleDNS.TupleDNS({
        proxyUrl: 'http://localhost:3000'
    });
    
    await dns.init();
    
    // Register this web app
    await dns.register("interactive.web.creative.music.tuple", 
                       ["javascript", "browser", "real-time"]);
    
    // Find music collaborators
    const musicians = await dns.find("*.*.*.music.tuple", ["collaborative"]);
    
    musicians.nodes.forEach(node => {
        console.log(`🎵 Found: ${node.coordinate}`);
        console.log(`   Capabilities: ${node.capabilities.join(', ')}`);
    });
}
```

### IoT Dashboard
```javascript
class IoTDashboard {
    constructor() {
        this.dns = new TupleDNS.TupleDNS();
    }
    
    async init() {
        await this.dns.init();
        
        // Register dashboard
        await this.dns.register("dashboard.web.building-5.iot.tuple",
                               ["visualization", "control", "web"]);
    }
    
    async discoverDevices(building) {
        const devices = await this.dns.find(`*.*.${building}.spatial.tuple`);
        return devices.nodes.map(node => ({
            name: node.coordinate.split('.')[0],
            location: node.coordinate.split('.').slice(1, -2).join('.'),
            capabilities: node.capabilities,
            address: node.ipAddress
        }));
    }
}
```

### Progressive Web App
```javascript
// Service Worker integration
if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/sw.js').then(async registration => {
        const dns = new TupleDNS.TupleDNS();
        await dns.init();
        
        // Register PWA capabilities
        await dns.register("pwa.offline.mobile.app.tuple",
                           ["service-worker", "offline", "push-notifications"]);
        
        // Discover compatible PWAs
        const pwas = await dns.find("*.offline.*.app.tuple");
        console.log(`Found ${pwas.nodes.length} offline-capable apps`);
    });
}
```

## Error Handling

```javascript
try {
    const result = await dns.find("invalid..pattern");
} catch (error) {
    if (error instanceof TupleDNS.TupleDNSError) {
        console.error('TupleDNS error:', error.message);
    } else {
        console.error('Network error:', error);
    }
}
```

## WebAssembly Support

For high-performance applications, TupleDNS includes WebAssembly bindings:

```javascript
// Load WebAssembly version
const wasmDns = await TupleDNS.loadWasm();
await wasmDns.init();

// Same API, better performance
const result = await wasmDns.find("*.music.tuple");
```

## Demo Application

See [examples/demo.html](../examples/demo.html) for a complete browser demo application.

For the complete implementation, see [tupledns.js](../tupledns.js).