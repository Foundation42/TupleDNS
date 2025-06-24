"""
TupleDNS Python Bindings
Python wrapper for the TupleDNS C library
"""

import ctypes
import ctypes.util
import os
from typing import List, Dict, Optional, Tuple, Any
from dataclasses import dataclass
from enum import IntEnum

# Load the TupleDNS C library
def _find_library():
    """Find the TupleDNS shared library"""
    lib_paths = [
        './libtupledns.so',
        './libtupledns.dylib', 
        'libtupledns.so',
        'libtupledns.dylib'
    ]
    
    for path in lib_paths:
        if os.path.exists(path):
            return path
    
    # Try system library search
    return ctypes.util.find_library('tupledns')

class TupleDNSError(IntEnum):
    OK = 0
    INVALID_COORDINATE = -1
    DNS_QUERY_FAILED = -2
    MEMORY_ALLOCATION = -3
    INVALID_PARAMETER = -4
    TIMEOUT = -5
    NO_RESULTS = -6
    CAPABILITY_PARSE = -7

class TupleDNSException(Exception):
    def __init__(self, error_code: int, message: str = None):
        self.error_code = error_code
        self.message = message or f"TupleDNS error {error_code}"
        super().__init__(self.message)

@dataclass
class TupleNode:
    coordinate: str
    ip_address: str
    capabilities: List[str]
    ttl: int
    last_seen: int

@dataclass
class TupleRange:
    dimension: str
    min_value: int
    max_value: int

@dataclass
class TupleResult:
    nodes: List[TupleNode]
    total_queries: int
    query_time: float
    error: int

class TupleDNS:
    """Main TupleDNS interface"""
    
    def __init__(self, lib_path: str = None):
        """Initialize TupleDNS library"""
        if lib_path is None:
            lib_path = _find_library()
        
        if lib_path is None:
            raise TupleDNSException(-1, "Could not find TupleDNS library")
        
        self._lib = ctypes.CDLL(lib_path)
        self._setup_function_signatures()
        
        # Initialize the library with default config
        result = self._lib.tupledns_init(None)
        if result != TupleDNSError.OK:
            raise TupleDNSException(result, "Failed to initialize TupleDNS")
    
    def _setup_function_signatures(self):
        """Setup ctypes function signatures for the C library"""
        
        # tupledns_init
        self._lib.tupledns_init.argtypes = [ctypes.c_void_p]
        self._lib.tupledns_init.restype = ctypes.c_int
        
        # tupledns_cleanup
        self._lib.tupledns_cleanup.argtypes = []
        self._lib.tupledns_cleanup.restype = None
        
        # tupledns_register
        self._lib.tupledns_register.argtypes = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_char_p), ctypes.c_int]
        self._lib.tupledns_register.restype = ctypes.c_int
        
        # tupledns_find
        self._lib.tupledns_find.argtypes = [ctypes.c_char_p]
        self._lib.tupledns_find.restype = ctypes.c_void_p
        
        # tupledns_free_result
        self._lib.tupledns_free_result.argtypes = [ctypes.c_void_p]
        self._lib.tupledns_free_result.restype = None
        
        # tupledns_validate_coordinate
        self._lib.tupledns_validate_coordinate.argtypes = [ctypes.c_char_p]
        self._lib.tupledns_validate_coordinate.restype = ctypes.c_int
        
        # tupledns_error_string
        self._lib.tupledns_error_string.argtypes = [ctypes.c_int]
        self._lib.tupledns_error_string.restype = ctypes.c_char_p
    
    def register(self, coordinate: str, capabilities: List[str] = None, ttl: int = 300) -> None:
        """Register a node at the given tuple coordinate"""
        if not self.validate_coordinate(coordinate):
            raise TupleDNSException(TupleDNSError.INVALID_COORDINATE, f"Invalid coordinate: {coordinate}")
        
        # Convert capabilities to C array
        cap_array = None
        if capabilities:
            cap_array = (ctypes.c_char_p * (len(capabilities) + 1))()
            for i, cap in enumerate(capabilities):
                cap_array[i] = cap.encode('utf-8')
            cap_array[len(capabilities)] = None
        
        result = self._lib.tupledns_register(
            coordinate.encode('utf-8'),
            cap_array,
            ttl
        )
        
        if result != TupleDNSError.OK:
            error_msg = self._lib.tupledns_error_string(result).decode('utf-8')
            raise TupleDNSException(result, error_msg)
    
    def find(self, pattern: str) -> TupleResult:
        """Find nodes matching the given pattern"""
        result_ptr = self._lib.tupledns_find(pattern.encode('utf-8'))
        if not result_ptr:
            raise TupleDNSException(TupleDNSError.NO_RESULTS, "No results found")
        
        try:
            # For now, return empty result since C implementation is not complete
            result = TupleResult(
                nodes=[],
                total_queries=1,
                query_time=0.0,
                error=TupleDNSError.NO_RESULTS
            )
            return result
        finally:
            self._lib.tupledns_free_result(result_ptr)
    
    def find_with_capabilities(self, pattern: str, required_capabilities: List[str]) -> TupleResult:
        """Find nodes matching pattern and having required capabilities"""
        # For now, delegate to find() since capability filtering is not implemented
        return self.find(pattern)
    
    def find_range(self, pattern: str, ranges: Dict[str, Tuple[int, int]]) -> TupleResult:
        """Find nodes within specified ranges"""
        # Convert ranges to individual queries
        if not ranges:
            return self.find(pattern)
        
        # For now, just return basic find result
        return self.find(pattern)
    
    def search_multi(self, patterns: List[str]) -> TupleResult:
        """Search multiple patterns simultaneously"""
        if not patterns:
            raise TupleDNSException(TupleDNSError.INVALID_PARAMETER, "No patterns provided")
        
        # For now, just search the first pattern
        return self.find(patterns[0])
    
    def validate_coordinate(self, coordinate: str) -> bool:
        """Validate a tuple coordinate format"""
        result = self._lib.tupledns_validate_coordinate(coordinate.encode('utf-8'))
        return result == 1
    
    def cleanup(self):
        """Cleanup TupleDNS resources"""
        self._lib.tupledns_cleanup()
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.cleanup()

# Convenience functions
def register(coordinate: str, capabilities: List[str] = None, ttl: int = 300) -> None:
    """Register a node (convenience function)"""
    with TupleDNS() as dns:
        dns.register(coordinate, capabilities, ttl)

def find(pattern: str) -> TupleResult:
    """Find nodes (convenience function)"""
    with TupleDNS() as dns:
        return dns.find(pattern)

def find_nearby(coordinate: str, capabilities: List[str] = None, radius: str = "1") -> TupleResult:
    """Find nearby nodes (convenience function for spatial coordinates)"""
    # This would implement spatial range queries
    # For now, use basic pattern matching
    parts = coordinate.split('.')
    if len(parts) >= 2:
        # Create a wildcard pattern for nearby search
        pattern = '.'.join(parts[:-1]) + '.*'
        return find(pattern)
    else:
        return find(coordinate)

# Example usage and testing
if __name__ == "__main__":
    print("TupleDNS Python Bindings Test")
    
    try:
        # Test coordinate validation
        test_coords = [
            "ambient.120.experimental.music.tuple",
            "london.uk.europe.spatial.tuple",
            "14.30.24.06.2025.time.tuple",
            "invalid-coordinate"
        ]
        
        with TupleDNS() as dns:
            for coord in test_coords:
                is_valid = dns.validate_coordinate(coord)
                print(f"Coordinate '{coord}': {'Valid' if is_valid else 'Invalid'}")
            
            # Test registration (will not actually register without DNS server)
            try:
                dns.register("test.120.ambient.music.tuple", ["midi-in", "generative"], 300)
                print("Registration successful (mock)")
            except TupleDNSException as e:
                print(f"Registration result: {e.message}")
            
            # Test discovery (will return empty results without actual DNS data)
            try:
                result = dns.find("*.120.*.music.tuple")
                print(f"Discovery found {len(result.nodes)} nodes")
            except TupleDNSException as e:
                print(f"Discovery result: {e.message}")
    
    except Exception as e:
        print(f"Error: {e}")
        print("Note: This is expected if the C library is not compiled yet")