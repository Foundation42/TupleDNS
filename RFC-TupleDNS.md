# RFC Draft: TupleDNS - Multidimensional Discovery Protocol Using DNS Infrastructure

**Internet Engineering Task Force**  
**Internet-Draft**  
**Intended status: Experimental**  
**Expires: December 2025**

---

## Abstract

This document specifies TupleDNS, a protocol for discovering services and capabilities within multidimensional coordinate spaces using existing DNS infrastructure. TupleDNS enables any networked entity to register itself using hierarchical tuple coordinates and be discovered by others querying similar coordinate spaces. The protocol leverages standard DNS record types (A, AAAA, TXT, CNAME) and requires no modifications to existing DNS servers or resolvers.

## Status of This Memo

This Internet-Draft is submitted in full conformance with the provisions of BCP 78 and BCP 79.

Internet-Drafts are working documents of the Internet Engineering Task Force (IETF). Note that other groups may also distribute working documents as Internet-Drafts.

Internet-Drafts are draft documents valid for a maximum of six months and may be updated, replaced, or obsoleted by other documents at any time. It is inappropriate to use Internet-Drafts as reference material or to cite them other than as "work in progress."

## Copyright Notice

Copyright (c) 2025 IETF Trust and the persons identified as the document authors. All rights reserved.

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Conventions and Definitions](#2-conventions-and-definitions)  
3. [Protocol Overview](#3-protocol-overview)
4. [Coordinate Encoding](#4-coordinate-encoding)
5. [DNS Record Usage](#5-dns-record-usage)
6. [Discovery Operations](#6-discovery-operations)
7. [Capability Declaration](#7-capability-declaration)
8. [Multi-Index Registration](#8-multi-index-registration)
9. [Security Considerations](#9-security-considerations)
10. [IANA Considerations](#10-iana-considerations)
11. [Implementation Examples](#11-implementation-examples)
12. [References](#12-references)

---

## 1. Introduction

### 1.1 Motivation

Current service discovery mechanisms typically rely on flat namespaces or simple hierarchical structures that do not capture the multidimensional nature of modern networked entities. A music collaboration service may wish to be discovered by genre, tempo, and geographic location simultaneously. An IoT device may need to be findable by physical location, device type, and operational capabilities. AI agents may benefit from discovery based on personality traits, processing characteristics, and collaborative preferences.

TupleDNS addresses this limitation by encoding multidimensional coordinates directly into DNS names, enabling rich spatial, temporal, functional, and semantic discovery patterns using existing DNS infrastructure.

### 1.2 Scope

This specification defines:
- Coordinate encoding schemes for multidimensional spaces
- DNS record usage patterns for tuple-based discovery
- Query patterns for multidimensional service discovery
- Capability declaration and filtering mechanisms
- Security and privacy considerations

This specification does not define:
- Modifications to DNS servers or resolvers
- New DNS record types or opcodes
- Mandatory coordinate space semantics
- Authentication or authorization mechanisms beyond existing DNS security

### 1.3 Requirements Language

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "NOT RECOMMENDED", "MAY", and "OPTIONAL" in this document are to be interpreted as described in BCP 14 [RFC2119] [RFC8174].

---

## 2. Conventions and Definitions

**Tuple Coordinate**: A hierarchical sequence of values representing a position in multidimensional space, encoded as a DNS name.

**Space Type**: A classification indicating the semantic meaning of coordinate dimensions (e.g., "music", "spatial", "temporal").

**Capability**: A declared function or characteristic of a network entity, encoded in DNS TXT records.

**Node**: A network entity registered at one or more tuple coordinates.

**Pattern**: A tuple coordinate containing wildcards for discovery queries.

**Range Query**: A discovery operation spanning multiple coordinates within specified dimensional ranges.

---

## 3. Protocol Overview

### 3.1 Core Principles

TupleDNS operates on the following principles:

1. **DNS Compatibility**: Uses only standard DNS record types and operations
2. **Multidimensional Encoding**: Embeds coordinate information in DNS names
3. **Hierarchical Discovery**: Leverages DNS tree structure for efficient queries
4. **Capability Declaration**: Uses TXT records for service characteristics
5. **Multi-Index Support**: Allows nodes to exist in multiple coordinate spaces

### 3.2 Protocol Flow

```
1. Node Registration:
   Entity → DNS: Register A/AAAA and TXT records at tuple coordinate

2. Capability Declaration:
   Entity → DNS: Publish capabilities in TXT record format

3. Discovery Query:
   Client → DNS: Query for nodes matching coordinate pattern

4. Capability Filtering:
   Client: Filter results based on required capabilities

5. Multi-Index Resolution:
   Client → DNS: Follow CNAME records for alternate coordinates
```

### 3.3 Coordinate Spaces

TupleDNS supports arbitrary coordinate spaces defined by applications. Common examples include:

- **Spatial**: Geographic or architectural positioning
- **Musical**: Genre, tempo, key, time signature
- **Temporal**: Time-based event coordination  
- **Functional**: Service type, capability, performance characteristics
- **Semantic**: AI personality, collaboration preferences

---

## 4. Coordinate Encoding

### 4.1 Encoding Format

Tuple coordinates are encoded as DNS names using the format:

```
<value1>.<value2>.<value3>.<space-type>.tuple
```

Where:
- `<valueN>`: Coordinate values, most specific to least specific
- `<space-type>`: Identifier for the coordinate space semantics
- `tuple`: Reserved top-level domain suffix

### 4.2 Value Encoding Rules

Coordinate values MUST:
- Contain only characters valid in DNS labels [RFC1035]
- Be case-insensitive
- Use hyphens to represent spaces or special characters
- Be limited to 63 octets per label [RFC1035]

Examples:
- Spaces: "ambient music" → "ambient-music"
- Special characters: "C#" → "c-sharp"
- Numeric values: "120" → "120"

### 4.3 Coordinate Examples

```
# Musical coordinates
ambient.120.4-4.c-major.london.music.tuple
jazz.140.swing.bb-major.newyork.music.tuple

# Spatial coordinates  
device-1.kitchen.floor-1.building-5.spatial.tuple
sensor-2.lab.floor-3.building-2.spatial.tuple

# Temporal coordinates
14.30.24.06.2025.time.tuple
workshop.morning.25.06.2025.event.tuple

# AI agent coordinates
creative.high.collaborative.research.ai.tuple
analytical.medium.autonomous.production.ai.tuple
```

### 4.4 Coordinate Validation

Implementations MUST validate coordinates according to:
1. DNS name syntax rules [RFC1035]
2. Total length not exceeding 253 octets
3. Termination with ".tuple" suffix
4. Minimum of space-type and one coordinate value

---

## 5. DNS Record Usage

### 5.1 Address Records (A/AAAA)

Nodes MUST register A or AAAA records at their tuple coordinates:

```
ambient.120.experimental.music.tuple. IN A 192.168.1.100
ambient.120.experimental.music.tuple. IN AAAA 2001:db8::1
```

Multiple address records MAY be registered for load balancing or redundancy.

### 5.2 Text Records (TXT)

Nodes SHOULD register TXT records declaring their capabilities:

```
ambient.120.experimental.music.tuple. IN TXT "caps=midi-in,midi-out,real-time,generative"
```

#### 5.2.1 TXT Record Format

Capability declarations use the format:
```
caps=<capability1>,<capability2>,...,<capabilityN>
```

Additional TXT record fields MAY include:
- `version=<protocol-version>`
- `contact=<contact-information>`
- `description=<human-readable-description>`

#### 5.2.2 Capability Naming

Capability names SHOULD:
- Use lowercase letters and hyphens
- Be descriptive and unambiguous
- Follow domain-specific conventions where established
- Avoid trademark or vendor-specific terms

### 5.3 Canonical Name Records (CNAME)

Nodes MAY use CNAME records for multi-index registration:

```
# Primary registration
ambient.120.experimental.music.tuple. IN A 192.168.1.100

# Spatial index alias
studio-2.building-5.london.spatial.tuple. IN CNAME ambient.120.experimental.music.tuple
```

CNAME records enable nodes to be discoverable in multiple coordinate spaces while maintaining a single authoritative address record.

---

## 6. Discovery Operations

### 6.1 Pattern-Based Discovery

Clients perform discovery using DNS queries with wildcard patterns. Since DNS does not natively support wildcards, clients MUST implement pattern expansion.

#### 6.1.1 Wildcard Expansion

Patterns containing `*` wildcards are expanded into multiple specific queries:

```
Pattern: *.120.*.music.tuple
Expansion: Enumerate known values for wildcard positions
Queries: ambient.120.experimental.music.tuple
         jazz.120.swing.music.tuple
         electronic.120.minimal.music.tuple
```

#### 6.1.2 Subdomain Enumeration

Clients MAY use DNS zone transfer requests or subdomain enumeration techniques where permitted:

```
; Request zone transfer for music.tuple
AXFR music.tuple

; Or query for specific patterns
QUERY ambient.120.experimental.music.tuple
```

### 6.2 Range Queries

Range queries span multiple coordinate values within specified bounds:

```python
# Pseudo-code for BPM range 110-130
for bpm in range(110, 131):
    query(f"ambient.{bpm}.experimental.music.tuple")
```

Implementations SHOULD:
- Execute range queries in parallel for performance
- Implement reasonable limits to prevent excessive DNS traffic
- Cache results to minimize redundant queries

### 6.3 Capability Filtering

After obtaining DNS query results, clients filter nodes based on required capabilities:

```python
# Pseudo-code
results = dns_query("*.music.tuple")
filtered = []
for result in results:
    txt_records = dns_query_txt(result.name)
    capabilities = parse_capabilities(txt_records)
    if required_capabilities.issubset(capabilities):
        filtered.append(result)
```

---

## 7. Capability Declaration

### 7.1 Capability Semantics

Capabilities represent functional characteristics, not guarantees. Clients MUST verify claimed capabilities through application-level testing.

Common capability categories include:
- **Protocols**: "http", "mqtt", "websocket"
- **Functions**: "sensor", "actuator", "compute"
- **Characteristics**: "real-time", "low-latency", "high-throughput"
- **Interfaces**: "rest-api", "graphql", "grpc"

### 7.2 Dynamic Capabilities

Capabilities MAY change over time. Implementations SHOULD:
- Use appropriate DNS TTL values for capability TXT records
- Implement cache invalidation for dynamic capabilities
- Provide mechanisms for capability updates

### 7.3 Capability Verification

Clients SHOULD implement application-level capability verification:

```python
def verify_capability(node, capability):
    # Example verification for HTTP API capability
    if capability == "rest-api":
        try:
            response = http_get(f"http://{node.address}/health")
            return response.status_code == 200
        except ConnectionError:
            return False
    return True
```

---

## 8. Multi-Index Registration

### 8.1 Use Cases

Multi-index registration enables nodes to be discoverable across different coordinate spaces:

- A music studio discoverable by both musical and spatial coordinates
- An IoT device findable by location, function, and network characteristics
- An AI service accessible via multiple semantic classification schemes

### 8.2 Implementation Patterns

#### 8.2.1 CNAME Aliases

```
# Primary registration
primary.coordinate.space.tuple. IN A 192.168.1.100
primary.coordinate.space.tuple. IN TXT "caps=capability1,capability2"

# Alias registrations
alias1.coordinate.space2.tuple. IN CNAME primary.coordinate.space.tuple
alias2.coordinate.space3.tuple. IN CNAME primary.coordinate.space.tuple
```

#### 8.2.2 Multiple A Records

```
# Independent registrations with same address
coord1.space1.tuple. IN A 192.168.1.100
coord1.space1.tuple. IN TXT "caps=capability1,capability2"

coord2.space2.tuple. IN A 192.168.1.100  
coord2.space2.tuple. IN TXT "caps=capability1,capability2"
```

### 8.3 Consistency Considerations

Nodes using multi-index registration SHOULD:
- Maintain consistent capability declarations across all coordinates
- Implement coordinated updates for address changes
- Use CNAME records where possible to ensure consistency

---

## 9. Security Considerations

### 9.1 DNS Security

TupleDNS inherits all security characteristics of the underlying DNS infrastructure:

- **Data Integrity**: Use DNSSEC where available [RFC4033]
- **Privacy**: Consider DNS-over-HTTPS [RFC8484] or DNS-over-TLS [RFC7858]
- **Availability**: Standard DNS resilience mechanisms apply

### 9.2 Information Disclosure

Coordinate registration reveals potentially sensitive information:

- **Location Privacy**: Spatial coordinates expose geographic information
- **Capability Disclosure**: TXT records reveal service characteristics
- **Activity Patterns**: Query patterns may reveal usage information

Implementations SHOULD:
- Provide opt-out mechanisms for privacy-sensitive applications
- Consider coordinate obfuscation techniques where appropriate
- Document privacy implications for users

### 9.3 Denial of Service

Potential DoS vectors include:
- **DNS Amplification**: Range queries generating excessive DNS traffic
- **Subdomain Flooding**: Mass registration of coordinates
- **Cache Pollution**: Injection of false coordinate information

Mitigations:
- Rate limiting for query operations
- Authentication requirements for registration
- Monitoring for abnormal query patterns

### 9.4 Trust and Verification

Capability claims in TXT records are not authenticated:
- Clients MUST NOT trust capability claims without verification
- Application-level authentication and authorization are REQUIRED
- Consider cryptographic capability signatures for high-security environments

---

## 10. IANA Considerations

### 10.1 Top-Level Domain

This specification assumes availability of the ".tuple" top-level domain. IANA coordination would be required for:
- Registration of ".tuple" as a special-use domain [RFC6761]
- Coordination with DNS root operators
- Policy development for coordinate space governance

### 10.2 Well-Known Space Types

IANA MAY maintain a registry of well-known space types:
- "spatial" - Geographic/architectural coordinates
- "temporal" - Time-based coordinates  
- "music" - Musical characteristics
- "network" - Network topology and performance
- "ai" - Artificial intelligence characteristics

### 10.3 Capability Registry

IANA MAY maintain a registry of standard capability names to promote interoperability across implementations.

---

## 11. Implementation Examples

### 11.1 C Library Implementation

```c
#include "tupledns.h"

int main() {
    // Initialize TupleDNS
    tupledns_init(NULL);
    
    // Register a music collaboration service
    const char* capabilities[] = {"midi", "real-time", "collaborative", NULL};
    tupledns_register("ambient.120.london.music.tuple", capabilities, 300);
    
    // Discover compatible collaborators
    tupledns_result_t* result = tupledns_find("*.120.*.music.tuple");
    
    for (int i = 0; i < result->node_count; i++) {
        printf("Found: %s at %s\n", 
               result->nodes[i].coordinate,
               result->nodes[i].ip_address);
    }
    
    tupledns_free_result(result);
    tupledns_cleanup();
    return 0;
}
```

### 11.2 Python Implementation

```python
import tupledns

# Register IoT device
tupledns.register("sensor-1.kitchen.floor-1.building-5.spatial.tuple",
                  capabilities=["temperature", "humidity", "iot"])

# Find all sensors in building
sensors = tupledns.find("*.*.*.building-5.spatial.tuple", 
                       capabilities=["sensor"])

for sensor in sensors.nodes:
    print(f"Sensor: {sensor.coordinate}")
    print(f"  Address: {sensor.ip_address}")
    print(f"  Capabilities: {', '.join(sensor.capabilities)}")
```

### 11.3 JavaScript/Browser Implementation

```javascript
// Initialize TupleDNS in browser
const dns = new TupleDNS({
    proxyUrl: 'http://localhost:3000'
});

await dns.init();

// Register web application
await dns.register("interactive.web.creative.art.tuple",
                   ["canvas", "webgl", "real-time"]);

// Discover compatible web apps
const apps = await dns.find("*.web.*.art.tuple");
console.log(`Found ${apps.nodes.length} creative web applications`);
```

---

## 12. References

### 12.1 Normative References

[RFC1035] Mockapetris, P., "Domain names - implementation and specification", STD 13, RFC 1035, November 1987.

[RFC2119] Bradner, S., "Key words for use in RFCs to Indicate Requirement Levels", BCP 14, RFC 2119, March 1997.

[RFC8174] Leiba, B., "Ambiguity of Uppercase vs Lowercase in RFC 2119 Key Words", BCP 14, RFC 8174, May 2017.

### 12.2 Informative References

[RFC4033] Arends, R., Austein, R., Larson, M., Massey, D., and S. Rose, "DNS Security Introduction and Requirements", RFC 4033, March 2005.

[RFC6761] Cheshire, S. and M. Krochmal, "Special-Use Top-Level Domains", RFC 6761, February 2013.

[RFC7858] Hu, Z., Zhu, L., Heidemann, J., Mankin, A., Wessels, D., and P. Hoffman, "Specification for DNS over Transport Layer Security (TLS)", RFC 7858, May 2016.

[RFC8484] Hoffman, P. and P. McManus, "DNS Queries over HTTPS (DoH)", RFC 8484, October 2018.

---

## Authors' Addresses

**Christian Bürger**  
Independent Researcher  
London, United Kingdom  
Email: christian@example.com

**Claude (Anthropic)**  
AI Research Assistant  
San Francisco, CA, USA  
Email: claude@anthropic.com

---

## Appendices

### Appendix A: Complete DNS Zone Example

```
; Example TupleDNS zone file for music.tuple
$ORIGIN music.tuple.
$TTL 300

; Ambient music nodes
ambient.120.experimental    IN A     192.168.1.100
ambient.120.experimental    IN TXT   "caps=midi-in,midi-out,real-time,generative"

ambient.80.drone           IN A     192.168.1.101  
ambient.80.drone           IN TXT   "caps=midi,atmospheric,collaborative"

; Jazz music nodes
jazz.140.swing.bb-major    IN A     192.168.1.102
jazz.140.swing.bb-major    IN TXT   "caps=midi,live-recording,improvisation"

; Electronic music nodes  
electronic.128.minimal     IN A     192.168.1.103
electronic.128.minimal     IN TXT   "caps=midi,synthesis,real-time"

; Multi-index example - same studio in spatial coordinate space
; (This would be in spatial.tuple zone)
studio-2.building-5.london.spatial.tuple. IN CNAME ambient.120.experimental.music.tuple.
```

### Appendix B: Implementation Considerations

#### B.1 Query Optimization

Implementations should consider:
- Parallel DNS query execution for range operations
- Intelligent caching strategies based on coordinate volatility  
- Query result aggregation and deduplication
- Progressive query expansion for large coordinate spaces

#### B.2 Coordinate Space Design

Best practices for coordinate space design:
- Use consistent ordering from specific to general
- Consider query efficiency in dimension ordering
- Provide clear semantic definitions for coordinate values
- Plan for coordinate space evolution and versioning

#### B.3 Scalability Considerations

For large-scale deployments:
- Consider coordinate space partitioning strategies
- Implement distributed caching mechanisms
- Plan for DNS infrastructure capacity requirements
- Design coordinate schemes to balance tree depth and fan-out

---

**End of RFC Draft**

*This document represents a working draft specification for TupleDNS. Implementation feedback and protocol refinements are welcomed through the IETF standards process.*