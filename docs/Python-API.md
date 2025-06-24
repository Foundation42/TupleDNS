# TupleDNS Python API Reference

Python bindings for the TupleDNS multidimensional discovery protocol.

## Installation

```bash
# Build the C library first
make all

# Build Python bindings
make python

# Or use directly
python3 tupledns.py
```

## Quick Start

```python
import tupledns

# Initialize (optional - auto-initialized on first use)
tupledns.init()

# Register yourself in coordinate space
tupledns.register("ambient.120.london.music.tuple", 
                  capabilities=["midi", "real-time", "collaborative"])

# Discover nodes
result = tupledns.find("*.120.*.music.tuple")
print(f"Found {len(result.nodes)} compatible nodes")

# Cleanup (optional - auto-cleanup on exit)
tupledns.cleanup()
```

## Core Classes

### TupleNode
```python
class TupleNode:
    coordinate: str
    ip_address: str
    capabilities: List[str]
    ttl: int
    last_seen: float
```

### TupleResult
```python
class TupleResult:
    nodes: List[TupleNode]
    total_queries: int
    query_time: float
```

## API Functions

### tupledns.init(config_file=None)
Initialize the TupleDNS library.
- `config_file`: Optional configuration file path

### tupledns.register(coordinate, capabilities=None, ttl=300)
Register a node at the specified coordinate.
- `coordinate`: Tuple coordinate string
- `capabilities`: List of capability strings
- `ttl`: Time-to-live in seconds

### tupledns.find(pattern, capabilities=None) → TupleResult
Find nodes matching the pattern.
- `pattern`: Search pattern with wildcards
- `capabilities`: Required capabilities filter

### tupledns.find_range(pattern, ranges=None, capabilities=None) → TupleResult
Find nodes within dimensional ranges.
- `pattern`: Base pattern with {dimension} placeholders
- `ranges`: Dictionary of dimension ranges
- `capabilities`: Required capabilities filter

### tupledns.unregister(coordinate)
Remove registration for a coordinate.

### tupledns.validate_coordinate(coordinate) → bool
Validate coordinate format.

### tupledns.cleanup()
Clean up library resources.

## Examples

### Music Collaboration
```python
import tupledns

# Register a music studio
tupledns.register("ambient.120.experimental.london.music.tuple",
                  capabilities=["midi", "real-time", "generative"])

# Find collaborators within BPM range
collaborators = tupledns.find_range(
    "ambient.{bpm}.*.london.music.tuple",
    ranges={"bpm": (110, 130)},
    capabilities=["collaborative"]
)

for node in collaborators.nodes:
    print(f"🎵 {node.coordinate}")
    print(f"   Address: {node.ip_address}")
    print(f"   Capabilities: {', '.join(node.capabilities)}")
```

### IoT Device Discovery
```python
# Register IoT devices
devices = [
    ("sensor-1.kitchen.floor-1.building-5.spatial.tuple", 
     ["temperature", "humidity", "iot"]),
    ("printer-1.lab.floor-2.building-5.spatial.tuple", 
     ["3d-print", "pla", "petg"]),
]

for coord, caps in devices:
    tupledns.register(coord, caps)

# Find all sensors in building
sensors = tupledns.find("*.*.*.building-5.spatial.tuple", 
                       capabilities=["sensor"])

print(f"Found {len(sensors.nodes)} sensors in building 5")
```

### AI Agent Network
```python
# Register AI agent with personality coordinates
tupledns.register("creative.80.collaborative.research.ai.tuple",
                  capabilities=["reasoning", "creativity", "multimodal"])

# Find compatible AI agents
partners = tupledns.find_range(
    "{personality}.{freq}.*.research.ai.tuple",
    ranges={
        "personality": ["creative", "analytical", "empathetic"],
        "freq": (70, 90)
    },
    capabilities=["collaborative"]
)

for agent in partners.nodes:
    print(f"🤖 Compatible AI: {agent.coordinate}")
```

## Error Handling

```python
try:
    result = tupledns.find("invalid..pattern")
except tupledns.TupleDNSError as e:
    print(f"TupleDNS error: {e}")
except Exception as e:
    print(f"General error: {e}")
```

## Type Hints

The Python bindings include full type hints for modern Python development:

```python
from typing import List, Optional, Dict, Union
import tupledns

def discover_music_collaborators(
    genre: str, 
    bpm_range: tuple[int, int],
    required_caps: Optional[List[str]] = None
) -> List[tupledns.TupleNode]:
    
    pattern = f"{genre}.{{bpm}}.*.music.tuple"
    ranges = {"bpm": bpm_range}
    
    result = tupledns.find_range(pattern, ranges, required_caps)
    return result.nodes
```

See [tupledns.py](../tupledns.py) for the complete implementation with inline documentation.