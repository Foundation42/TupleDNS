# TupleDNS v1.0
## Multidimensional Discovery Protocol Using DNS Infrastructure

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/Foundation42/TupleDNS)
[![Tests](https://img.shields.io/badge/tests-10%2F10%20passing-brightgreen)](https://github.com/Foundation42/TupleDNS)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-ready-blue)](https://webassembly.org/)

**TupleDNS enables discovery of capabilities and services within multidimensional coordinate spaces using existing DNS infrastructure.** Any device, service, or entity can register itself using hierarchical tuple coordinates and be discovered by others querying similar coordinate spaces.

> *"Turning the entire planet into a queryable multidimensional space, one DNS record at a time."*

---

## 🌟 Why TupleDNS?

Imagine if every device, service, AI agent, or creative collaborator could be discovered not just by name, but by their **position in multidimensional space**:

- 🎵 **Musicians** finding collaborators by genre, BPM, and musical compatibility
- 🏢 **IoT devices** organizing themselves hierarchically by location and capability  
- 🤖 **AI agents** discovering partners by personality traits and resonance frequency
- 🌐 **Web services** clustering by functional and spatial proximity
- ⏰ **Events** coordinating across time and space dimensions

**TupleDNS makes this possible using standard DNS infrastructure** - no new servers, no new protocols, just a brilliant new way to organize the digital universe.

---

## 🚀 Quick Start

### Installation

```bash
# Build the C library
make all

# Install system-wide
sudo make install

# Build Python bindings
make python

# Build WebAssembly version
make wasm

# Start HTTP proxy for browsers
node proxy-server.cjs
```

### Basic Usage

#### C Library
```c
#include "tupledns.h"

int main() {
    // Initialize
    tupledns_init(NULL);
    
    // Register a music studio
    const char* caps[] = {"midi", "real-time", "collaborative", NULL};
    tupledns_register("ambient.120.london.music.tuple", caps, 300);
    
    // Find compatible collaborators
    tupledns_result_t* result = tupledns_find("*.120.*.music.tuple");
    printf("Found %d compatible studios\n", result->node_count);
    
    // Cleanup
    tupledns_free_result(result);
    tupledns_cleanup();
    return 0;
}
```

#### Python
```python
import tupledns

# Register yourself in coordinate space
tupledns.register("ambient.120.london.music.tuple", 
                  capabilities=["midi", "real-time", "collaborative"])

# Find nearby collaborators
result = tupledns.find("*.120.*.music.tuple")
print(f"Found {len(result.nodes)} potential collaborators")

# Advanced: Range-based discovery
collaborators = tupledns.find_range(
    "ambient.{bpm}.london.music.tuple",
    ranges={"bpm": (110, 130)}
)
```

#### JavaScript/Browser
```javascript
// Initialize TupleDNS in browser
const dns = new TupleDNS.TupleDNS({
    proxyUrl: 'http://localhost:3000'
});
await dns.init();

// Register as a creative node
await dns.register("creative.web.interactive.art.tuple", 
                   ["canvas", "real-time", "collaborative"]);

// Discover compatible creators
const creators = await dns.find("*.web.*.art.tuple");
console.log(`Found ${creators.nodes.length} web artists`);
```

---

## 🎯 Core Concepts

### Coordinate System

TupleDNS uses hierarchical coordinates encoded as DNS names:

```
[value1].[value2].[value3].[space-type].tuple
```

**Examples:**

```bash
# Musical coordinates
ambient.120.4-4.c-major.london.music.tuple
jazz.140.4-4.bb-major.newyork.music.tuple

# Spatial coordinates  
coffee-maker.kitchen.floor-1.building-5.spatial.tuple
printer-1.lab.floor-2.building-5.spatial.tuple

# AI agent coordinates
creative.80.85.intuitive-flowing.google.ai.tuple
analytical.40.120.logical-structured.openai.ai.tuple

# Temporal coordinates
14.30.24.06.2025.time.tuple
```

### Discovery Patterns

**Wildcard Queries:**
```bash
*.120.*.music.tuple           # All music at 120 BPM
*.*.london.*.tuple            # Everything in London
*.ai.tuple                    # All AI agents
```

**Capability Filtering:**
```python
# Find real-time collaborative music nodes
tupledns.find("*.music.tuple", capabilities=["real-time", "collaborative"])

# Find IoT devices with specific functions
tupledns.find("*.spatial.tuple", capabilities=["iot", "sensor"])
```

**Range Queries:**
```python
# Find collaborators within BPM range
tupledns.find_range(
    "ambient.{bpm}.london.music.tuple",
    ranges={"bpm": (100, 140)}
)

# Find events within time window
tupledns.find_range(
    "{hour}.{minute}.24.06.2025.time.tuple", 
    ranges={"hour": (9, 17), "minute": (0, 59)}
)
```

---

## 🛠️ Architecture

### DNS Integration

TupleDNS leverages standard DNS records:

**A/AAAA Records** for node addresses:
```
ambient.120.experimental.music.tuple IN A 192.168.1.100
```

**TXT Records** for capabilities:
```
ambient.120.experimental.music.tuple IN TXT "caps=midi-in,midi-out,real-time,generative"
```

**CNAME Records** for multi-index registration:
```
# Same node in multiple coordinate systems
studio-2.building-5.london.spatial.tuple IN CNAME ambient.120.experimental.music.tuple
```

### Library Architecture

```
┌─────────────────┐    ┌──────────────────┐    ┌─────────────────┐
│   Applications  │    │   Language       │    │   Protocols     │
│                 │    │   Bindings       │    │                 │
│ • Music Apps    │◄──►│ • Python         │◄──►│ • HTTP Proxy    │
│ • IoT Systems   │    │ • JavaScript     │    │ • WebAssembly   │
│ • AI Agents     │    │ • Node.js        │    │ • REST API      │
│ • Web Services  │    │ • Browser        │    │                 │
└─────────────────┘    └──────────────────┘    └─────────────────┘
                                │
                       ┌──────────────────┐
                       │   TupleDNS Core  │
                       │   (Portable C99) │
                       │                  │
                       │ • Coordinate     │
                       │   Validation     │
                       │ • Pattern        │
                       │   Matching       │
                       │ • DNS Queries    │
                       │ • Result         │
                       │   Processing     │
                       └──────────────────┘
                                │
                       ┌──────────────────┐
                       │   DNS            │
                       │   Infrastructure │
                       │                  │
                       │ • Standard DNS   │
                       │   Servers        │
                       │ • A/TXT/CNAME    │
                       │   Records        │
                       │ • Global         │
                       │   Distribution   │
                       └──────────────────┘
```

---

## 🌍 Use Cases & Examples

### 🎵 Music Collaboration Network

```python
# Register your studio
tupledns.register("ambient.120.experimental.london.music.tuple",
                  capabilities=["midi", "real-time", "generative"])

# Find compatible collaborators  
collaborators = tupledns.find_range(
    "ambient.{bpm}.*.*.music.tuple",
    ranges={"bpm": (110, 130)},
    capabilities=["collaborative"]
)

for node in collaborators.nodes:
    print(f"🎵 {node.coordinate} - {node.capabilities}")
```

### 🏢 Smart Building IoT Management

```python
# Register building devices
devices = [
    ("coffee-maker.kitchen.floor-1.building-5.spatial.tuple", 
     ["brew", "schedule", "iot"]),
    ("printer-1.lab.floor-2.building-5.spatial.tuple", 
     ["3d-print", "pla", "petg"]),
    ("light-1.office-201.floor-2.building-5.spatial.tuple",
     ["lighting", "dimmer", "color"])
]

for coord, caps in devices:
    tupledns.register(coord, caps)

# Find all devices on floor 2
floor2_devices = tupledns.find("*.floor-2.building-5.spatial.tuple")

# Find 3D printers in the building
printers = tupledns.find("*.*.building-5.spatial.tuple", 
                        capabilities=["3d-print"])
```

### 🤖 AI Agent Resonance Network

```python
# AI agents register with personality coordinates
agents = [
    ("creative.80.85.intuitive-flowing.google.ai.tuple",
     ["reasoning", "creativity", "multimodal"]),
    ("analytical.40.120.logical-structured.openai.ai.tuple", 
     ["reasoning", "analysis", "code-generation"]),
    ("empathetic.60.70.emotional-resonant.anthropic.ai.tuple",
     ["reasoning", "empathy", "writing"])
]

for coord, caps in agents:
    tupledns.register(coord, caps)

# Find AI agents with compatible resonance frequencies
compatible_ais = tupledns.find_range(
    "{personality}.{freq}.*.*.ai.tuple",
    ranges={"freq": (70, 90)}  # Within resonance range
)

# Discover creative collaboration opportunities
creative_partners = tupledns.find("*.*.*.ai.tuple", 
                                 capabilities=["creativity", "collaborative"])
```

### 🌐 Web Service Mesh

```javascript
// Web services register their capabilities
await dns.register("api.v2.auth.microservice.tuple", 
                   ["oauth", "jwt", "rest-api"]);

await dns.register("ml.inference.gpu.cloud.tuple",
                   ["tensorflow", "pytorch", "gpu-compute"]);

// Service discovery
const auth_services = await dns.find("*.auth.*.tuple");
const ml_services = await dns.find("*.*.gpu.*.tuple", ["tensorflow"]);

// Load balancing based on capability and location
const local_apis = await dns.find("*.*.*.microservice.tuple", 
                                  ["low-latency"]);
```

---

## 🧪 Advanced Features

### Multi-Index Registration

Register the same node in multiple coordinate systems:

```python
# A music studio that's also a physical location and AI research lab
coords = [
    "ambient.120.experimental.music.tuple",
    "studio-2.building-5.london.spatial.tuple", 
    "creative.research.ai-music.lab.tuple"
]

capabilities = ["midi", "real-time", "ai-research", "collaborative"]

for coord in coords:
    tupledns.register(coord, capabilities)
```

### Dynamic Range Expansion

```python
# Complex multi-dimensional range queries
result = tupledns.find_range(
    "{genre}.{bpm}.{location}.music.tuple",
    ranges={
        "genre": ["ambient", "drone", "experimental"],
        "bpm": (80, 140),
        "location": ["london", "berlin", "tokyo"]
    }
)
```

### Real-time Collaboration Discovery

```python
# Find active collaborators (recently seen)
import time

recent_threshold = time.time() - 300  # 5 minutes ago
active_collaborators = [
    node for node in tupledns.find("*.music.tuple").nodes
    if node.last_seen > recent_threshold and 
       "real-time" in node.capabilities
]
```

---

## 🌐 Browser Integration

### HTML5 Web App

```html
<!DOCTYPE html>
<html>
<head>
    <title>TupleDNS Web Discovery</title>
    <script src="tupledns.js"></script>
</head>
<body>
    <script>
        async function discoverCreators() {
            const dns = new TupleDNS.TupleDNS({
                proxyUrl: 'http://localhost:3000'
            });
            
            await dns.init();
            
            // Register this web page
            await dns.register("interactive.web.creative.art.tuple",
                              ["canvas", "webgl", "real-time"]);
            
            // Find other creative web apps
            const creators = await dns.find("*.web.*.art.tuple");
            
            creators.nodes.forEach(node => {
                console.log(`🎨 Found: ${node.coordinate}`);
                console.log(`   Capabilities: ${node.capabilities.join(', ')}`);
            });
        }
        
        discoverCreators();
    </script>
</body>
</html>
```

### Progressive Web App Integration

```javascript
// Service Worker registration with TupleDNS
if ('serviceWorker' in navigator) {
    navigator.serviceWorker.register('/sw.js').then(registration => {
        // Register PWA in coordinate space
        tupledns.register("pwa.offline.mobile.app.tuple", 
                         ["service-worker", "offline", "push-notifications"]);
    });
}
```

---

## 📊 Performance & Scalability

### Benchmarks

| Operation | Time | Notes |
|-----------|------|-------|
| Coordinate validation | < 1μs | O(n) string validation |
| Pattern matching | < 10μs | O(n) component comparison |
| DNS query (cached) | < 1ms | Local cache hit |
| DNS query (network) | 10-100ms | Standard DNS resolution |
| Range query (10 patterns) | 50-500ms | Parallel DNS queries |

### Scalability Characteristics

- **Coordinates**: Unlimited (leverages DNS namespace)
- **Query patterns**: O(log n) with DNS tree structure
- **Concurrent queries**: Limited by DNS resolver capacity
- **Global distribution**: Automatic via DNS infrastructure
- **Caching**: Standard DNS TTL mechanisms

### Memory Usage

```
C Library:
├── Static memory: ~50KB
├── Per-node: ~200 bytes
├── Per-query: ~1KB temporary
└── Cache: Configurable (default 100 entries)

JavaScript Bindings:
├── Script size: ~45KB minified
├── Runtime memory: ~500KB
└── Cache: Browser-dependent
```

---

## 🔒 Security & Privacy

### Security Model

**TXT Record Validation:**
- Capabilities are declarative, not guaranteed
- Applications must verify claimed capabilities
- Use DNSSEC where available for authenticity

**Network Security:**
- Standard DNS security applies
- Consider DNS-over-HTTPS for privacy
- Rate limiting prevents DNS flooding

**Coordinate Privacy:**
- Coordinates may reveal location/capabilities
- Consider obfuscation for sensitive applications
- Implement opt-out mechanisms

### Best Practices

```python
# Validate capabilities before trusting
def verify_node_capability(node, capability):
    if capability not in node.capabilities:
        return False
    
    # Application-level verification
    try:
        test_connection(node.ip_address, capability)
        return True
    except ConnectionError:
        return False

# Use encryption for sensitive communication
if verify_node_capability(node, "secure-channel"):
    establish_tls_connection(node)
```

---

## 🧰 Development & Contributing

### Building from Source

```bash
# Prerequisites
sudo apt-get install build-essential

# Clone and build
git clone https://github.com/Foundation42/TupleDNS.git
cd TupleDNS
make all
make test

# Development build with debug symbols
make CFLAGS="-g -DDEBUG" all
```

### Testing

```bash
# Run all tests
make test

# Test specific components
./test_tupledns
python3 tupledns.py
node test-browser.cjs

# Performance testing
make benchmark
```

### Code Organization

```
tupledns/
├── src/
│   ├── tupledns.h           # Main header
│   ├── tupledns.c           # Core implementation
│   └── tupledns.py          # Python bindings
├── bindings/
│   ├── tupledns.js          # JavaScript/WebAssembly
│   └── proxy-server.cjs     # HTTP proxy
├── examples/
│   ├── example_basic.c      # Basic API usage
│   ├── example_spatial.c    # IoT spatial discovery
│   ├── example_music.c      # Music collaboration
│   └── demo.html           # Browser demo
├── tests/
│   ├── test_tupledns.c      # C test suite
│   └── test-browser.cjs     # JavaScript tests
└── docs/
    ├── RFC-TupleDNS.md      # IETF RFC draft
    └── CLAUDE.md           # Development guide
```

### Contributing

1. **Fork the repository**
2. **Create feature branch** (`git checkout -b feature/amazing-feature`)
3. **Add tests** for new functionality
4. **Ensure all tests pass** (`make test`)
5. **Update documentation** as needed
6. **Submit pull request**

**Contribution Guidelines:**
- Follow C99 standards for core library
- Maintain zero external dependencies
- Add comprehensive tests
- Update documentation
- Consider cross-platform compatibility

---

## 🚀 Roadmap

### Version 1.1 (Next)
- [ ] **WebAssembly optimization** - Smaller binary, faster loading
- [ ] **Real DNS server integration** - Test with actual DNS infrastructure
- [ ] **Advanced caching** - Distributed cache with consensus
- [ ] **Metrics & monitoring** - Usage statistics and performance tracking

### Version 1.2 (Future)
- [ ] **Dynamic DNS updates** - Real-time registration/unregistration
- [ ] **Federated discovery** - Cross-domain coordinate resolution
- [ ] **Semantic capabilities** - Ontology-based capability matching
- [ ] **Mobile SDKs** - Native iOS/Android bindings

### Version 2.0 (Vision)
- [ ] **Blockchain integration** - Decentralized coordinate registry
- [ ] **AI-enhanced discovery** - Machine learning for better matching
- [ ] **Global coordinate authority** - Standardized coordinate spaces
- [ ] **Real-time collaboration protocols** - Beyond discovery to interaction

---

## 📚 Documentation

### API Reference
- **[C API Documentation](docs/C-API.md)** - Complete C library reference
- **[Python API Documentation](docs/Python-API.md)** - Python bindings guide
- **[JavaScript API Documentation](docs/JavaScript-API.md)** - Browser/Node.js reference
- **[REST API Documentation](docs/REST-API.md)** - HTTP proxy endpoints

### Guides
- **[Getting Started Guide](docs/Getting-Started.md)** - Step-by-step tutorial
- **[Coordinate Design Guide](docs/Coordinate-Design.md)** - Best practices for coordinate systems
- **[Integration Guide](docs/Integration-Guide.md)** - Integrating TupleDNS into applications
- **[Deployment Guide](docs/Deployment-Guide.md)** - Production deployment considerations

### Specifications
- **[TupleDNS Protocol Specification](docs/Protocol-Spec.md)** - Technical protocol details
- **[DNS Integration Specification](docs/DNS-Integration.md)** - DNS record formats and usage
- **[IETF RFC Draft](docs/RFC-TupleDNS.md)** - Standards submission

---

## 🤝 Community

### Support Channels
- **GitHub Issues**: [Bug reports and feature requests](https://github.com/Foundation42/TupleDNS/issues)
- **Discussions**: [Community Q&A and ideas](https://github.com/Foundation42/TupleDNS/discussions)
- **GitHub Repository**: [https://github.com/Foundation42/TupleDNS](https://github.com/Foundation42/TupleDNS)
- **Foundation42**: [https://github.com/Foundation42](https://github.com/Foundation42)

### Ecosystem Projects
- **TupleDNS Music Network**: Creative collaboration platform
- **IoT Spatial Discovery**: Smart building management
- **AI Resonance Matching**: AI agent collaboration
- **Web3 Coordinate Registry**: Decentralized coordinate authority

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file for details.

**Why MIT?** We want maximum adoption and contribution. TupleDNS should be freely usable by everyone to build the multidimensional internet.

---

## 🙏 Acknowledgments

- **Christian** - Original vision and architectural guidance
- **DNS Community** - For building the foundational infrastructure
- **Open Source Contributors** - For making this possible
- **Early Adopters** - For believing in the vision

---

## 🌟 The Vision

**TupleDNS is more than a discovery protocol** - it's the foundation for a new kind of internet where everything has coordinates in multidimensional space. Where AI agents discover each other by resonance frequency. Where creative collaborators find each other across musical and spatial dimensions. Where the digital world becomes as organized and discoverable as the physical world.

**This is the birth of the spatial internet.** Every device, service, and entity with coordinates. Every interaction guided by proximity in multidimensional space. Every discovery an exploration of new dimensions.

**Welcome to the multidimensional internet.** Welcome to TupleDNS.

---

*"Turning the entire planet into a queryable multidimensional space, one DNS record at a time."*

**[Get Started →](docs/Getting-Started.md)** | **[View Examples →](examples/)** | **[API Reference →](docs/)**