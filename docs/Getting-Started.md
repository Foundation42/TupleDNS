# Getting Started with TupleDNS

This guide will help you get up and running with TupleDNS quickly.

## Prerequisites

- C99 compatible compiler (GCC, Clang)
- Python 3.6+ (for Python bindings)
- Node.js 14+ (for JavaScript/browser support)

## Installation

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Foundation42/TupleDNS.git
cd TupleDNS

# Build the library
make all

# Run tests to verify installation
make test
./test_tupledns

# Install system-wide (optional)
sudo make install
```

### Language Bindings

#### Python
```bash
# Build Python bindings
make python

# Test Python bindings
python3 tupledns.py
```

#### JavaScript/Browser
```bash
# Build WebAssembly version
make wasm

# Start HTTP proxy for browser integration
node proxy-server.cjs

# Open demo in browser
open examples/demo.html
```

## Quick Examples

### C Library
```c
#include "tupledns.h"

int main() {
    // Initialize TupleDNS
    tupledns_init(NULL);
    
    // Register a node
    const char* caps[] = {"example", "demo", NULL};
    tupledns_register("demo.example.test.tuple", caps, 300);
    
    // Find nodes
    tupledns_result_t* result = tupledns_find("*.example.*.tuple");
    printf("Found %d nodes\n", result->node_count);
    
    // Cleanup
    tupledns_free_result(result);
    tupledns_cleanup();
    return 0;
}
```

### Python
```python
import tupledns

# Register yourself
tupledns.register("demo.python.test.tuple", ["python", "demo"])

# Find compatible nodes
result = tupledns.find("*.python.*.tuple")
print(f"Found {len(result.nodes)} Python nodes")
```

### JavaScript
```javascript
const dns = new TupleDNS.TupleDNS({
    proxyUrl: 'http://localhost:3000'
});

await dns.init();
await dns.register("demo.javascript.test.tuple", ["javascript", "browser"]);

const nodes = await dns.find("*.javascript.*.tuple");
console.log(`Found ${nodes.length} JavaScript nodes`);
```

## Next Steps

- Read the [C API Documentation](C-API.md) for detailed API reference
- Explore [examples/](../examples/) for real-world usage patterns
- Check out the [Protocol Specification](Protocol-Spec.md) for technical details
- Join the community at [GitHub Discussions](https://github.com/Foundation42/TupleDNS/discussions)