# TupleDNS Protocol Specification

Technical specification for the TupleDNS multidimensional discovery protocol.

## Overview

TupleDNS enables discovery of services and capabilities within multidimensional coordinate spaces using existing DNS infrastructure. This document provides the technical details for implementers.

## Coordinate Encoding

### Format
```
<value1>.<value2>.<value3>.<space-type>.tuple
```

### Encoding Rules
- Values must be valid DNS labels (RFC 1035)
- Case-insensitive
- Hyphens replace spaces and special characters
- Maximum 63 characters per label
- Total coordinate length ≤ 253 characters

### Examples
```
ambient.120.4-4.c-major.london.music.tuple
device-1.kitchen.floor-1.building-5.spatial.tuple
creative.high.collaborative.research.ai.tuple
```

## DNS Record Usage

### Address Records (A/AAAA)
Nodes register their IP addresses:
```
ambient.120.experimental.music.tuple. IN A 192.168.1.100
```

### Text Records (TXT)
Capabilities are declared in TXT records:
```
ambient.120.experimental.music.tuple. IN TXT "caps=midi-in,midi-out,real-time"
```

### Canonical Name Records (CNAME)
Multi-index registration via aliases:
```
studio-2.building-5.london.spatial.tuple. IN CNAME ambient.120.experimental.music.tuple.
```

## Discovery Protocol

### Pattern Matching
Clients expand wildcard patterns into specific DNS queries:
```
Pattern: *.120.*.music.tuple
Expands to multiple specific coordinate queries
```

### Range Queries
Systematic enumeration within dimensional bounds:
```python
# BPM range 110-130
for bpm in range(110, 131):
    query(f"ambient.{bpm}.experimental.music.tuple")
```

### Capability Filtering
Post-query filtering based on TXT record capabilities:
```
1. Query DNS for coordinate pattern matches
2. Retrieve TXT records for each match
3. Parse capability declarations
4. Filter results by required capabilities
```

## Implementation Requirements

### Coordinate Validation
- Verify DNS label syntax compliance
- Check total length limits
- Validate .tuple suffix
- Ensure minimum components (space-type + value)

### DNS Integration
- Use standard DNS resolution APIs
- Implement appropriate caching strategies
- Handle DNS errors gracefully
- Support both IPv4 and IPv6

### Capability Handling
- Parse TXT records for capability declarations
- Support comma-separated capability lists
- Implement case-insensitive capability matching
- Handle multiple TXT records per node

## Performance Considerations

### Query Optimization
- Parallel DNS query execution
- Intelligent result caching
- Query pattern optimization
- Rate limiting to prevent DNS flooding

### Scalability
- Leverage DNS hierarchy for efficient queries
- Use appropriate TTL values
- Implement distributed caching where possible
- Design coordinate spaces for query efficiency

## Security Model

### Trust Assumptions
- Capabilities are declarations, not guarantees
- Application-level verification required
- Standard DNS security mechanisms apply
- No built-in authentication beyond DNS

### Best Practices
- Use DNSSEC where available
- Implement capability verification
- Consider DNS-over-HTTPS for privacy
- Monitor for abnormal query patterns

## Error Handling

### DNS Resolution Errors
- Timeout handling
- NXDOMAIN responses
- Server failures
- Network connectivity issues

### Validation Errors
- Invalid coordinate formats
- Malformed capability declarations
- Protocol version mismatches
- Resource exhaustion

## Extensibility

### Version Negotiation
TXT records may include version information:
```
"version=1.0;caps=midi,real-time"
```

### Custom Attributes
Additional TXT record fields:
```
"caps=midi;contact=admin@example.com;description=Studio A"
```

### Coordinate Space Evolution
- Backward compatibility considerations
- Namespace coordination
- Semantic versioning for coordinate spaces

## Implementation Notes

This specification is implemented in the reference C library at [tupledns.c](../tupledns.c) and [tupledns.h](../tupledns.h).

For the complete IETF standards submission, see [RFC-TupleDNS.md](../RFC-TupleDNS.md).