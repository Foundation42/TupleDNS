# TupleDNS

**Multidimensional Discovery Protocol Using DNS Infrastructure**

TupleDNS enables discovery of capabilities and services within multidimensional coordinate spaces using existing DNS infrastructure. Any device, service, or entity can register itself using hierarchical tuple coordinates (spatial, temporal, musical, etc.) and be discovered by others querying similar coordinate spaces.

## 🌟 Key Features

- **Universal Coordinate System**: Organize anything in multidimensional space
- **DNS-Based**: Leverages existing DNS infrastructure - no new servers needed
- **Portable C99**: Runs on everything from microcontrollers to supercomputers
- **Multiple Language Bindings**: Python, JavaScript, and more
- **Zero Dependencies**: Only standard DNS resolution libraries
- **Range Queries**: Find nodes within coordinate ranges
- **Capability Matching**: Discover services by their advertised capabilities

## 🚀 Quick Start

### Building the Library

```bash
# Build static and shared libraries
make all

# Run tests
make test

# Build examples
make examples

# Build Python bindings
make python
```

### Basic Usage (C)

```c
#include "tupledns.h"

int main() {
    // Initialize TupleDNS
    tupledns_init(NULL);
    
    // Register a service
    const char* caps[] = {"midi-in", "real-time", NULL};
    tupledns_register("ambient.120.london.music.tuple", caps, 300);
    
    // Discover services
    tupledns_result_t* result = tupledns_find("*.120.*.music.tuple");
    printf("Found %d nodes\n", result->node_count);
    
    // Cleanup
    tupledns_free_result(result);
    tupledns_cleanup();
    return 0;
}
```

### Basic Usage (Python)

```python
import tupledns

# Register a node
tupledns.register("ambient.120.london.music.tuple", 
                  capabilities=["midi-in", "real-time"])

# Find nearby collaborators
result = tupledns.find("*.120.*.music.tuple")
print(f"Found {len(result.nodes)} potential collaborators")

# Find devices with specific capabilities
devices = tupledns.find("*.*.london.*.spatial", 
                       capabilities=["iot"])
```

## 📋 Coordinate System

TupleDNS uses hierarchical coordinates encoded as DNS names:

```
[value1].[value2].[value3].[space-type].tuple
```

### Examples

**Musical Coordinates:**
```
ambient.120.4-4.c-major.music.tuple
jazz.140.4-4.bb-major.music.tuple
```

**Spatial Coordinates:**
```
kitchen.floor-1.building-5.spatial.tuple
lab.floor-2.building-5.spatial.tuple
```

**Temporal Coordinates:**
```
14.30.24.06.2025.time.tuple
```

**IoT Device Coordinates:**
```
coffee-maker.kitchen.home.device.tuple
printer-1.lab.office.device.tuple
```

## 🔍 Discovery Patterns

### Wildcard Queries
```c
// All music at 120 BPM
tupledns_find("*.120.*.music.tuple");

// All devices in building 5
tupledns_find("*.*.building-5.spatial.tuple");

// All IoT devices
tupledns_find("*.*.*.device.tuple");
```

### Capability-Based Discovery
```c
const char* required[] = {"real-time", "collaborative", NULL};
tupledns_find_with_caps("*.music.tuple", required);
```

### Range Queries
```python
# Find collaborators within BPM range
result = tupledns.find_range(
    "ambient.{bpm}.london.music.tuple",
    ranges={"bpm": (110, 130)}
)
```

## 📡 DNS Integration

TupleDNS uses standard DNS records:

**A/AAAA Records** for node addresses:
```
ambient.120.experimental.music.tuple IN A 192.168.1.100
```

**TXT Records** for capabilities:
```
ambient.120.experimental.music.tuple IN TXT "caps=midi-in,midi-out,real-time"
```

**CNAME Records** for multi-index registration:
```
# Same node in multiple coordinate systems
studio-2.building-5.london.spatial.tuple IN CNAME ambient.120.experimental.music.tuple
```

## 🎵 Use Cases

### Music Collaboration
```python
# Register a studio
tupledns.register("ambient.120.experimental.london.music", 
                  capabilities=["midi", "real-time", "collaborative"])

# Find compatible collaborators
collaborators = tupledns.find_range(
    "ambient.{bpm}.*.*.music",
    ranges={"bpm": (100, 140)},
    capabilities=["collaborative"]
)
```

### IoT Device Discovery
```python
# Find all devices in kitchen
devices = tupledns.find("*.kitchen.home.device")

# Find 3D printers in building
printers = tupledns.find("*.*.building-5.spatial", 
                        capabilities=["3d-print"])
```

### Spatial Services
```python
# Register a service location
tupledns.register("printer.lab.building-5.spatial",
                  capabilities=["3d-print", "pla", "petg"])

# Find services within area
nearby = tupledns.find("*.lab.building-5.spatial")
```

## 🛠️ API Reference

### Core Functions

```c
// Initialization
int tupledns_init(const tupledns_config_t* config);
void tupledns_cleanup(void);

// Registration
int tupledns_register(const char* coordinate, const char* capabilities[], int ttl);
int tupledns_unregister(const char* coordinate);

// Discovery
tupledns_result_t* tupledns_find(const char* pattern);
tupledns_result_t* tupledns_find_with_caps(const char* pattern, const char* required_caps[]);
tupledns_result_t* tupledns_find_range(const char* pattern, const tupledns_range_t ranges[], int range_count);

// Utilities
int tupledns_validate_coordinate(const char* coordinate);
char* tupledns_encode_coordinate(const char* space_type, const char* values[], int value_count);
int tupledns_match_pattern(const char* coordinate, const char* pattern);

// Memory management
void tupledns_free_result(tupledns_result_t* result);
```

### Data Structures

```c
typedef struct {
    char* coordinate;           // Full tuple coordinate
    char* ip_address;          // IPv4/IPv6 address
    char** capabilities;       // Array of capability strings
    int capability_count;      // Number of capabilities
    int ttl;                   // Time to live
    time_t last_seen;          // Last discovery time
} tupledns_node_t;

typedef struct {
    tupledns_node_t* nodes;    // Array of discovered nodes
    int node_count;            // Number of nodes found
    int total_queries;         // DNS queries performed
    double query_time;         // Total query time (seconds)
    tupledns_error_t error;    // Error code if any
} tupledns_result_t;
```

## 🌐 Language Bindings

### Python
```bash
make python
```

```python
import tupledns

with tupledns.TupleDNS() as dns:
    dns.register("test.coordinate.tuple", ["capability1", "capability2"])
    result = dns.find("*.coordinate.tuple")
```

### JavaScript/WebAssembly
```bash
make wasm
```

```javascript
// In browser or Node.js
const nodes = await TupleDNS.find("*.music.tuple");
```

## 🔧 Building and Installation

### Dependencies
- C99 compatible compiler (GCC, Clang)
- Standard DNS resolution libraries (included in most systems)

### Build Options
```bash
make all          # Build everything
make shared       # Build shared library only
make static       # Build static library only
make test         # Build and run tests
make examples     # Build example programs
make python       # Build Python bindings
make wasm         # Build WebAssembly version
make install      # Install system-wide
```

### Installation
```bash
make install PREFIX=/usr/local
```

## 📚 Examples

See the `examples/` directory for complete working examples:

- **example_basic.c** - Basic library usage and API demonstration
- **example_spatial.c** - IoT device discovery in building layouts
- **example_music.c** - Music collaboration and creative matching

Run examples:
```bash
make examples
./example_basic
./example_spatial
./example_music
```

## 🧪 Testing

Comprehensive test suite covering all functionality:

```bash
make test
./test_tupledns
```

## 🚀 Advanced Features

### Multi-Index Registration
Register the same node in multiple coordinate systems:

```c
// Register in both musical and spatial spaces
tupledns_register("ambient.120.experimental.music.tuple", caps, 300);
tupledns_register("studio-2.building-5.london.spatial.tuple", caps, 300);
// Link them with DNS CNAME records
```

### Range Queries
Find nodes within coordinate ranges:

```c
tupledns_range_t ranges[] = {
    {"bpm", 110, 130},
    {"floor", 1, 3}
};
tupledns_result_t* result = tupledns_find_range("*.{bpm}.{floor}.music.tuple", ranges, 2);
```

### Capability Filtering
Discover services by their advertised capabilities:

```c
const char* required[] = {"real-time", "low-latency", NULL};
tupledns_result_t* result = tupledns_find_with_caps("*.music.tuple", required);
```

## 🔒 Security Considerations

- Use DNSSEC when available for authenticity
- Capabilities are declarative, not guaranteed - implement application-level verification
- Consider privacy implications of spatial indexing
- Validate all input to prevent injection attacks

## 🤝 Contributing

We welcome contributions! Please see:

1. **Issues**: Report bugs or request features
2. **Pull Requests**: Submit improvements
3. **Documentation**: Help improve docs and examples
4. **Testing**: Add test cases for new functionality

### Development Setup
```bash
git clone <repository>
cd tupledns
make all
make test
```

## 📄 License

MIT License - see LICENSE file for details.

## 🌍 Vision

TupleDNS aims to create a universal coordinate system for the digital world, enabling:

- **Global Discovery**: Any device, service, or entity can be discovered
- **Spatial Computing**: Location-aware applications and services  
- **Creative Collaboration**: Artists and creators finding compatible partners
- **IoT Organization**: Hierarchical organization of connected devices
- **Distributed Systems**: Service discovery for microservices and cloud apps

*"Turning the entire planet into a queryable multidimensional space, one DNS record at a time."*

---

For more information, examples, and detailed API documentation, visit our [documentation](docs/) or run the included examples.