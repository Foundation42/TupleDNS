# TupleDNS Specification v1.0

**Multidimensional Discovery Protocol Using DNS Infrastructure**

---

## Abstract

TupleDNS enables discovery of capabilities and services within multidimensional coordinate spaces using existing DNS infrastructure. Any device, service, or entity can register itself using hierarchical tuple coordinates (spatial, temporal, musical, etc.) and be discovered by others querying similar coordinate spaces.

## 1. Core Concepts

### 1.1 Tuple Coordinates
A tuple coordinate is a hierarchical sequence of values representing a position in multidimensional space, encoded as DNS subdomains:

```
value1.value2.value3.space-type.tuple
```

**Examples:**
- Spatial: `london.uk.europe.earth.spatial.tuple`
- Musical: `ambient.120.4-4.c-major.music.tuple`
- Temporal: `14.30.24.06.2025.time.tuple`
- Network: `low.high.excellent.quality.network.tuple`

### 1.2 Namespace Structure
```
[coordinate-values].[space-type].tuple
```

- **coordinate-values**: Hierarchical tuple components (most specific to least specific)
- **space-type**: Type of coordinate space (spatial, music, time, etc.)
- **tuple**: Root domain suffix

### 1.3 Data Types
Following JSON-style conventions:
- **Numbers**: `120`, `3.14159`
- **Strings**: `ambient`, `c-major` (use hyphens for spaces)
- **Special encoding**: Reserved characters encoded as needed

## 2. Registration Protocol

### 2.1 Basic Registration
Nodes register by creating DNS A/AAAA records at their tuple coordinate:

```
ambient.120.experimental.music.tuple IN A 192.168.1.100
```

### 2.2 Capability Declaration
Capabilities declared via TXT records:

```
ambient.120.experimental.music.tuple IN TXT "caps=midi-in,midi-out,real-time,generative"
```

### 2.3 Multi-Index Registration
Same node can exist in multiple coordinate spaces via CNAME records:

```
# Primary registration
ambient.120.experimental.music.tuple IN A 192.168.1.100

# Spatial alias for same node  
studio-2.building-5.london.spatial.tuple IN CNAME ambient.120.experimental.music.tuple
```

### 2.4 Range Coverage
Nodes register at multiple coordinates to support range queries:

```
# Register at adjacent BPM values for range discovery
119.experimental.music.tuple IN A 192.168.1.100
120.experimental.music.tuple IN A 192.168.1.100  
121.experimental.music.tuple IN A 192.168.1.100
```

## 3. Discovery Protocol

### 3.1 Basic Queries
Standard DNS queries with wildcards:

```
# Find all experimental music nodes
*.experimental.music.tuple

# Find 120 BPM nodes in any genre
120.*.music.tuple
```

### 3.2 Capability Filtering
Client-side filtering based on TXT record capabilities:

```python
# Pseudo-code
nodes = dns_query("*.120.*.music.tuple")
filtered = [n for n in nodes if "real-time" in n.capabilities]
```

### 3.3 Range Queries
Client expands ranges into multiple DNS queries:

```python
# Find nodes in BPM range 110-130
queries = [f"{bpm}.ambient.music.tuple" for bpm in range(110, 131)]
results = [dns_query(q) for q in queries]
```

### 3.4 Subdomain Discovery
Use TXT records for dynamic subdomain enumeration:

```
music.tuple IN TXT "subdomains=ambient,jazz,rock,electronic"
```

## 4. API Specification

### 4.1 Core Library (C99)

```c
// Registration
int tupledns_register(const char* coordinate, const char* capabilities[], int ttl);

// Discovery  
tupledns_result* tupledns_find(const char* pattern);
tupledns_result* tupledns_find_with_caps(const char* pattern, const char* required_caps[]);

// Range queries
tupledns_result* tupledns_find_range(const char* pattern, tupledns_range ranges[]);

// Multi-index search
tupledns_result* tupledns_search_multi(const char* patterns[], int count);

// Cleanup
void tupledns_free_result(tupledns_result* result);
```

### 4.2 Python Binding

```python
import tupledns

# Registration
tupledns.register("ambient.120.london.uk.music", 
                  capabilities=["midi-in", "real-time"],
                  ttl=300)

# Basic discovery
nodes = tupledns.find("*.120.london.uk.music")

# Range queries
nodes = tupledns.find_range(
    pattern="ambient.{bpm}.london.uk.music",
    ranges={"bpm": (110, 130)}
)

# Capability filtering
nodes = tupledns.find("*.*.london.uk.music", 
                      capabilities=["midi-in"])

# Multi-dimensional search
nodes = tupledns.search([
    "ambient.*.london.uk.music",
    "*.*.london.spatial"
])
```

### 4.3 JavaScript/Web API

```javascript
// Registration (via proxy service for web clients)
await TupleDNS.register("ambient.120.studio.music", {
    capabilities: ["midi-in", "web-audio"],
    ttl: 300
});

// Discovery
const nodes = await TupleDNS.find("*.120.*.music");

// Range queries
const nodes = await TupleDNS.findRange(
    "ambient.{bpm}.studio.music", 
    { bpm: [110, 130] }
);
```

## 5. Data Structures

### 5.1 TupleNode
```c
typedef struct {
    char* coordinate;           // Full tuple coordinate
    char* ip_address;          // IPv4/IPv6 address  
    char** capabilities;       // Array of capability strings
    int capability_count;      // Number of capabilities
    int ttl;                   // Time to live
    time_t last_seen;          // Last discovery time
} tupledns_node;
```

### 5.2 Query Result
```c
typedef struct {
    tupledns_node* nodes;      // Array of discovered nodes
    int node_count;            // Number of nodes found
    int total_queries;         // DNS queries performed
    double query_time;         // Total query time (seconds)
} tupledns_result;
```

## 6. Implementation Guidelines

### 6.1 Core Library Requirements
- **Portable C99**: No platform-specific dependencies
- **Zero external dependencies**: Only standard DNS resolution
- **Async support**: Non-blocking DNS queries
- **Memory efficient**: Configurable memory limits
- **Thread-safe**: Safe for concurrent use

### 6.2 DNS Query Optimization
- **Parallel queries**: Execute range queries concurrently
- **Caching**: Respect DNS TTL values
- **Deduplication**: Merge identical nodes from multiple queries
- **Timeout handling**: Configurable query timeouts

### 6.3 Web Integration
- **WebAssembly build**: Compile core C library to WASM
- **Proxy service**: HTTP API for CORS-restricted environments
- **JavaScript native**: Pure JS implementation for simple cases
- **REST wrapper**: HTTP endpoints for language-agnostic access

## 7. Usage Examples

### 7.1 IoT Device Discovery
```python
# Smart coffee maker announces capabilities
tupledns.register("coffee-maker.kitchen.home.device", 
                  capabilities=["brew", "schedule", "iot"])

# Find all IoT devices in kitchen
devices = tupledns.find("*.kitchen.home.device", 
                        capabilities=["iot"])
```

### 7.2 Creative Collaboration
```python
# Music studio registration
tupledns.register("ambient.120.experimental.london.music",
                  capabilities=["midi", "real-time", "collaborative"])

# Find collaborators in similar musical space
collaborators = tupledns.find_range(
    "ambient.{bpm}.*.london.music",
    ranges={"bpm": (100, 140)},
    capabilities=["collaborative"]
)
```

### 7.3 Spatial Services
```python
# 3D printer registration
tupledns.register("printer.lab.building-5.spatial",
                  capabilities=["3d-print", "pla", "petg"])

# Find 3D printers in building
printers = tupledns.find("*.lab.building-5.spatial",
                         capabilities=["3d-print"])
```

## 8. Security Considerations

### 8.1 DNS Security
- Use DNSSEC where available for authenticity
- Validate TXT record formats to prevent injection
- Rate limit queries to prevent DoS

### 8.2 Capability Verification
- Capabilities are declarative, not guaranteed
- Implement application-level verification
- Use encryption for sensitive communications

### 8.3 Privacy
- Coordinate registration reveals location/capabilities
- Consider privacy implications of spatial indexing
- Implement opt-out mechanisms

## 9. Extension Points

### 9.1 Rich Type Descriptors (Future)
```
ambient.120.experimental.music.tuple IN TXT "schema={genre:string,bpm:number,key:string}"
```

### 9.2 Dynamic Coordinate Systems
- Runtime coordinate system definitions
- Cross-space coordinate transformations  
- Hierarchical space embeddings

### 9.3 Semantic Capabilities
- Ontology-based capability matching
- Fuzzy capability search
- Machine learning enhanced discovery

## 10. Implementation Roadmap

### Phase 1: Core Library
- ✅ C99 core implementation
- ✅ Basic registration/discovery
- ✅ Python bindings
- ✅ Simple range queries

### Phase 2: Web Integration  
- ✅ JavaScript bindings
- ✅ WebAssembly build
- ✅ Proxy service for CORS
- ✅ REST API wrapper

### Phase 3: Advanced Features
- ✅ Multi-index support
- ✅ Capability filtering
- ✅ Query optimization
- ✅ Rich error handling

### Phase 4: Ecosystem
- ✅ Language bindings (Rust, Go, etc.)
- ✅ Framework integrations
- ✅ Developer tools
- ✅ Community examples

---

## License
Open source under MIT License - maximum adoption and contribution

## Contributing
Community-driven development with focus on simplicity, portability, and universal accessibility.

---

*"Turning the entire planet into a queryable multidimensional space, one DNS record at a time."*