"""
TupleDNS Python Test Suite

Comprehensive tests for the Python bindings using pytest
"""

import pytest
import sys
import os
import time
from typing import List, Optional

# Add parent directory to path to import tupledns
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
import tupledns

class TestTupleDNSCore:
    """Test core TupleDNS functionality"""
    
    def setup_method(self):
        """Setup for each test method"""
        tupledns.init()
    
    def teardown_method(self):
        """Cleanup after each test method"""
        tupledns.cleanup()
    
    def test_library_lifecycle(self):
        """Test library initialization and cleanup"""
        tupledns.cleanup()
        tupledns.init()  # Should not raise
        tupledns.cleanup()
        tupledns.init()  # Should work multiple times
    
    def test_coordinate_validation(self):
        """Test coordinate validation"""
        # Valid coordinates
        assert tupledns.validate_coordinate("simple.test.tuple")
        assert tupledns.validate_coordinate("a.b.c.d.e.f.tuple")
        assert tupledns.validate_coordinate("123.456.789.numeric.tuple")
        assert tupledns.validate_coordinate("with-hyphens.and-more.test.tuple")
        
        # Invalid coordinates
        assert not tupledns.validate_coordinate("")
        assert not tupledns.validate_coordinate("no-suffix")
        assert not tupledns.validate_coordinate(".tuple")
        assert not tupledns.validate_coordinate("spaces not allowed.tuple")
        assert not tupledns.validate_coordinate("under_scores.tuple")
        assert not tupledns.validate_coordinate("special@chars.tuple")
    
    def test_coordinate_encoding(self):
        """Test coordinate encoding and decoding"""
        components = ["ambient", "120", "london", "music"]
        encoded = tupledns.encode_coordinate(components)
        assert encoded == "ambient.120.london.music.tuple"
        
        decoded = tupledns.decode_coordinate(encoded)
        assert decoded == components
    
    def test_pattern_matching(self):
        """Test pattern matching functionality"""
        coord = "ambient.120.london.music.tuple"
        
        # Exact matches
        assert tupledns.match_pattern(coord, coord)
        
        # Wildcard matches
        assert tupledns.match_pattern(coord, "*.120.london.music.tuple")
        assert tupledns.match_pattern(coord, "ambient.*.london.music.tuple")
        assert tupledns.match_pattern(coord, "ambient.120.*.music.tuple")
        assert tupledns.match_pattern(coord, "*.*.*.*.tuple")
        
        # Non-matches
        assert not tupledns.match_pattern(coord, "jazz.120.london.music.tuple")
        assert not tupledns.match_pattern(coord, "ambient.140.london.music.tuple")
        assert not tupledns.match_pattern(coord, "ambient.120.london.music.spatial.tuple")

class TestTupleDNSRegistration:
    """Test registration and discovery functionality"""
    
    def setup_method(self):
        tupledns.init()
    
    def teardown_method(self):
        tupledns.cleanup()
    
    def create_test_coordinate(self, prefix: str) -> str:
        """Create a unique test coordinate"""
        timestamp = int(time.time() * 1000) % 10000
        return f"{prefix}.test.{timestamp}.tuple"
    
    def test_basic_registration(self):
        """Test basic node registration"""
        coord = self.create_test_coordinate("basic")
        capabilities = ["test", "python"]
        
        # Should not raise
        tupledns.register(coord, capabilities, ttl=300)
        tupledns.unregister(coord)
    
    def test_registration_with_capabilities(self):
        """Test registration with various capability combinations"""
        coords_and_caps = [
            (self.create_test_coordinate("music"), ["midi", "real-time", "collaborative"]),
            (self.create_test_coordinate("iot"), ["sensor", "temperature", "wireless"]),
            (self.create_test_coordinate("ai"), ["reasoning", "creativity", "multimodal"]),
        ]
        
        for coord, caps in coords_and_caps:
            tupledns.register(coord, caps)
        
        # Cleanup
        for coord, _ in coords_and_caps:
            tupledns.unregister(coord)
    
    def test_discovery_basic(self):
        """Test basic discovery functionality"""
        # Register some test nodes
        coord1 = self.create_test_coordinate("discover1")
        coord2 = self.create_test_coordinate("discover2")
        
        tupledns.register(coord1, ["test", "discovery"])
        tupledns.register(coord2, ["test", "discovery"])
        
        # Discover nodes
        result = tupledns.find("*.test.*.tuple")
        assert result is not None
        assert hasattr(result, 'nodes')
        assert hasattr(result, 'total_queries')
        assert hasattr(result, 'query_time')
        
        # Cleanup
        tupledns.unregister(coord1)
        tupledns.unregister(coord2)
    
    def test_capability_filtering(self):
        """Test discovery with capability filtering"""
        coord1 = self.create_test_coordinate("cap1")
        coord2 = self.create_test_coordinate("cap2")
        
        tupledns.register(coord1, ["common", "unique1"])
        tupledns.register(coord2, ["common", "unique2"])
        
        # Find with capability filter
        result = tupledns.find("*.test.*.tuple", capabilities=["common"])
        assert result is not None
        
        result_unique = tupledns.find("*.test.*.tuple", capabilities=["unique1"])
        assert result_unique is not None
        
        # Cleanup
        tupledns.unregister(coord1)
        tupledns.unregister(coord2)

class TestTupleDNSRangeQueries:
    """Test range query functionality"""
    
    def setup_method(self):
        tupledns.init()
    
    def teardown_method(self):
        tupledns.cleanup()
    
    def test_numeric_range(self):
        """Test numeric range queries"""
        coords = [
            "ambient.100.test.music.tuple",
            "ambient.120.test.music.tuple", 
            "ambient.140.test.music.tuple"
        ]
        
        for coord in coords:
            tupledns.register(coord, ["music", "test"])
        
        # Range query
        result = tupledns.find_range(
            "ambient.{bpm}.test.music.tuple",
            ranges={"bpm": (110, 130)}
        )
        assert result is not None
        
        # Cleanup
        for coord in coords:
            tupledns.unregister(coord)
    
    def test_multi_dimensional_range(self):
        """Test multi-dimensional range queries"""
        coords = [
            "ambient.120.london.music.tuple",
            "jazz.120.newyork.music.tuple",
            "electronic.140.berlin.music.tuple"
        ]
        
        for coord in coords:
            tupledns.register(coord, ["music"])
        
        # Multi-dimensional range
        result = tupledns.find_range(
            "{genre}.{bpm}.{city}.music.tuple",
            ranges={
                "genre": ["ambient", "jazz"],
                "bpm": (110, 130),
                "city": ["london", "newyork", "berlin"]
            }
        )
        assert result is not None
        
        # Cleanup
        for coord in coords:
            tupledns.unregister(coord)

class TestTupleDNSErrorHandling:
    """Test error handling and edge cases"""
    
    def test_invalid_coordinates(self):
        """Test handling of invalid coordinates"""
        tupledns.init()
        
        with pytest.raises(Exception):
            tupledns.register("", ["test"])
        
        with pytest.raises(Exception):
            tupledns.register("invalid coordinate", ["test"])
        
        with pytest.raises(Exception):
            tupledns.find("")
        
        tupledns.cleanup()
    
    def test_none_arguments(self):
        """Test handling of None arguments"""
        tupledns.init()
        
        with pytest.raises(Exception):
            tupledns.register(None, ["test"])
        
        with pytest.raises(Exception):
            tupledns.find(None)
        
        tupledns.cleanup()
    
    def test_empty_capabilities(self):
        """Test registration with empty capabilities"""
        tupledns.init()
        
        coord = f"empty.{int(time.time())}.test.tuple"
        # Should work with empty capabilities
        tupledns.register(coord, [])
        tupledns.unregister(coord)
        
        tupledns.cleanup()

class TestTupleDNSPerformance:
    """Performance and stress tests"""
    
    def setup_method(self):
        tupledns.init()
    
    def teardown_method(self):
        tupledns.cleanup()
    
    @pytest.mark.performance
    def test_bulk_registration(self):
        """Test bulk registration performance"""
        coords = []
        start_time = time.time()
        
        # Register 100 nodes
        for i in range(100):
            coord = f"perf.{i}.bulk.test.tuple"
            coords.append(coord)
            tupledns.register(coord, ["performance", "test"])
        
        reg_time = time.time() - start_time
        print(f"\nBulk registration: 100 nodes in {reg_time:.3f}s")
        
        # Cleanup
        for coord in coords:
            tupledns.unregister(coord)
    
    @pytest.mark.performance
    def test_discovery_performance(self):
        """Test discovery performance"""
        # Register test nodes
        coords = []
        for i in range(50):
            coord = f"discover.{i}.perf.test.tuple"
            coords.append(coord)
            tupledns.register(coord, ["performance"])
        
        start_time = time.time()
        
        # Perform multiple discoveries
        for _ in range(20):
            result = tupledns.find("discover.*.perf.test.tuple")
        
        disc_time = time.time() - start_time
        print(f"\nDiscovery performance: 20 queries in {disc_time:.3f}s")
        
        # Cleanup
        for coord in coords:
            tupledns.unregister(coord)
    
    @pytest.mark.stress
    def test_memory_stress(self):
        """Test memory usage under stress"""
        coords = []
        
        # Register many nodes with varying capabilities
        for i in range(200):
            coord = f"stress.{i}.memory.test.tuple"
            coords.append(coord)
            caps = [f"cap{j}" for j in range(i % 5 + 1)]
            tupledns.register(coord, caps)
        
        # Perform many discoveries
        for _ in range(50):
            result = tupledns.find("stress.*.memory.test.tuple")
            if result:
                # Force some processing
                len(result.nodes)
        
        # Cleanup
        for coord in coords:
            tupledns.unregister(coord)

class TestTupleDNSRealWorldScenarios:
    """Test real-world usage scenarios"""
    
    def setup_method(self):
        tupledns.init()
    
    def teardown_method(self):
        tupledns.cleanup()
    
    def test_music_collaboration_scenario(self):
        """Test music collaboration use case"""
        # Register musicians
        musicians = [
            ("ambient.120.experimental.london.music.tuple", ["midi", "real-time", "collaborative"]),
            ("jazz.140.swing.newyork.music.tuple", ["midi", "live-recording", "improvisation"]),
            ("electronic.128.minimal.berlin.music.tuple", ["midi", "synthesis", "real-time"]),
        ]
        
        for coord, caps in musicians:
            tupledns.register(coord, caps)
        
        # Find collaborators by BPM range
        result = tupledns.find_range(
            "{genre}.{bpm}.{style}.{city}.music.tuple",
            ranges={"bpm": (115, 135)},
            capabilities=["real-time"]
        )
        assert result is not None
        
        # Find MIDI-capable musicians
        midi_result = tupledns.find("*.*.*.*.music.tuple", capabilities=["midi"])
        assert midi_result is not None
        
        # Cleanup
        for coord, _ in musicians:
            tupledns.unregister(coord)
    
    def test_iot_device_scenario(self):
        """Test IoT device management use case"""
        # Register IoT devices
        devices = [
            ("sensor-1.kitchen.floor-1.building-5.spatial.tuple", ["temperature", "humidity", "iot"]),
            ("printer-1.lab.floor-2.building-5.spatial.tuple", ["3d-print", "pla", "petg"]),
            ("light-1.office-201.floor-2.building-5.spatial.tuple", ["lighting", "dimmer", "color"]),
        ]
        
        for coord, caps in devices:
            tupledns.register(coord, caps)
        
        # Find all devices in building
        building_devices = tupledns.find("*.*.*.building-5.spatial.tuple")
        assert building_devices is not None
        
        # Find sensors specifically
        sensors = tupledns.find("*.*.*.building-5.spatial.tuple", capabilities=["iot"])
        assert sensors is not None
        
        # Find devices on specific floor
        floor2_devices = tupledns.find("*.floor-2.building-5.spatial.tuple")
        assert floor2_devices is not None
        
        # Cleanup
        for coord, _ in devices:
            tupledns.unregister(coord)
    
    def test_ai_agent_scenario(self):
        """Test AI agent discovery use case"""
        # Register AI agents
        agents = [
            ("creative.80.collaborative.research.ai.tuple", ["reasoning", "creativity", "multimodal"]),
            ("analytical.120.autonomous.production.ai.tuple", ["reasoning", "analysis", "code-generation"]),
            ("empathetic.70.collaborative.support.ai.tuple", ["reasoning", "empathy", "writing"]),
        ]
        
        for coord, caps in agents:
            tupledns.register(coord, caps)
        
        # Find collaborative agents
        collaborative = tupledns.find("*.*.collaborative.*.ai.tuple")
        assert collaborative is not None
        
        # Find agents with reasoning capability
        reasoning_agents = tupledns.find("*.*.*.*.ai.tuple", capabilities=["reasoning"])
        assert reasoning_agents is not None
        
        # Find agents by frequency range
        freq_range = tupledns.find_range(
            "{personality}.{freq}.{mode}.{context}.ai.tuple",
            ranges={"freq": (60, 90)}
        )
        assert freq_range is not None
        
        # Cleanup
        for coord, _ in agents:
            tupledns.unregister(coord)

# Test configuration for pytest
if __name__ == "__main__":
    pytest.main([__file__, "-v"])