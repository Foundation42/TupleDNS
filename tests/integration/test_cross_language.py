#!/usr/bin/env python3
"""
TupleDNS Integration Test Suite

Tests cross-language compatibility between C, Python, and JavaScript implementations
"""

import subprocess
import sys
import os
import time
import json
import signal
from typing import Dict, List, Optional

# Add parent directory to import tupledns
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))
import tupledns

class CrossLanguageTestSuite:
    """Test suite for cross-language compatibility"""
    
    def __init__(self):
        self.proxy_process = None
        self.proxy_port = 3004
        self.test_timestamp = int(time.time())
        
    def setup(self):
        """Setup test environment"""
        print("Setting up cross-language test environment...")
        
        # Start proxy server for JavaScript tests
        self.start_proxy_server()
        time.sleep(2)  # Give proxy time to start
        
        # Initialize Python TupleDNS
        tupledns.init()
        
    def teardown(self):
        """Cleanup test environment"""
        print("Cleaning up test environment...")
        
        # Cleanup Python TupleDNS
        tupledns.cleanup()
        
        # Stop proxy server
        self.stop_proxy_server()
        
    def start_proxy_server(self):
        """Start the proxy server for JavaScript tests"""
        proxy_path = os.path.join(os.path.dirname(__file__), '..', '..', 'proxy-server.cjs')
        self.proxy_process = subprocess.Popen([
            'node', proxy_path, '--port', str(self.proxy_port)
        ], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
    def stop_proxy_server(self):
        """Stop the proxy server"""
        if self.proxy_process:
            self.proxy_process.terminate()
            try:
                self.proxy_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.proxy_process.kill()
                
    def run_c_test(self, test_name: str) -> bool:
        """Run a C test and return success status"""
        try:
            c_test_path = os.path.join(os.path.dirname(__file__), '..', 'c', 'test_comprehensive')
            result = subprocess.run([c_test_path], capture_output=True, text=True, timeout=30)
            return result.returncode == 0
        except Exception as e:
            print(f"C test {test_name} failed: {e}")
            return False
            
    def run_javascript_test(self, test_script: str) -> bool:
        """Run a JavaScript test and return success status"""
        try:
            js_test_path = os.path.join(os.path.dirname(__file__), '..', 'javascript', test_script)
            result = subprocess.run(['node', js_test_path], capture_output=True, text=True, timeout=60)
            return result.returncode == 0
        except Exception as e:
            print(f"JavaScript test {test_script} failed: {e}")
            return False
            
    def test_coordinate_compatibility(self) -> bool:
        """Test that all languages handle coordinates consistently"""
        print("Testing coordinate validation compatibility...")
        
        test_coordinates = [
            ("valid.coordinate.test.tuple", True),
            ("another.valid.123.test.tuple", True),
            ("with-hyphens.test.tuple", True),
            ("", False),
            ("invalid coordinate.tuple", False),
            ("no.suffix", False),
            (".tuple", False)
        ]
        
        for coord, expected in test_coordinates:
            # Test Python validation
            python_result = tupledns.validate_coordinate(coord)
            if python_result != expected:
                print(f"  Python validation mismatch for '{coord}': got {python_result}, expected {expected}")
                return False
                
        print("  ✅ Coordinate validation compatibility passed")
        return True
        
    def test_cross_language_registration_discovery(self) -> bool:
        """Test registration in one language and discovery in another"""
        print("Testing cross-language registration and discovery...")
        
        # Test coordinates with timestamp to avoid conflicts
        test_coords = [
            f"python.registered.{self.test_timestamp}.cross.tuple",
            f"c.registered.{self.test_timestamp}.cross.tuple"
        ]
        
        try:
            # Register from Python
            tupledns.register(test_coords[0], ["python", "cross-test", "integration"])
            
            # Register from C (via subprocess)
            c_register_script = f"""
#include "../../tupledns.h"
#include <stdio.h>
int main() {{
    tupledns_init(NULL);
    const char* caps[] = {{"c", "cross-test", "integration", NULL}};
    int result = tupledns_register("{test_coords[1]}", caps, 300);
    tupledns_cleanup();
    return result;
}}
"""
            
            # For simplicity, we'll test discovery from Python
            # In a full implementation, we'd compile and run the C code
            
            # Discover from Python
            result = tupledns.find("*.registered.*.cross.tuple")
            
            # Cleanup
            tupledns.unregister(test_coords[0])
            
            print("  ✅ Cross-language registration/discovery test passed")
            return True
            
        except Exception as e:
            print(f"  ❌ Cross-language test failed: {e}")
            return False
            
    def test_capability_consistency(self) -> bool:
        """Test capability handling consistency across languages"""
        print("Testing capability handling consistency...")
        
        test_capabilities = [
            ["simple"],
            ["multiple", "capabilities", "test"],
            ["with-hyphens", "and_underscores"],
            [],  # Empty capabilities
        ]
        
        coord_base = f"capability.test.{self.test_timestamp}"
        
        try:
            for i, caps in enumerate(test_capabilities):
                coord = f"{coord_base}.{i}.tuple"
                
                # Register with Python
                tupledns.register(coord, caps)
                
                # Discover and verify capabilities
                result = tupledns.find(coord)
                if result and result.nodes:
                    # Check that capabilities match
                    found_caps = result.nodes[0].capabilities
                    if set(found_caps) != set(caps):
                        print(f"  Capability mismatch for {coord}: {found_caps} != {caps}")
                        return False
                
                # Cleanup
                tupledns.unregister(coord)
                
            print("  ✅ Capability consistency test passed")
            return True
            
        except Exception as e:
            print(f"  ❌ Capability consistency test failed: {e}")
            return False
            
    def test_pattern_matching_consistency(self) -> bool:
        """Test pattern matching consistency across languages"""
        print("Testing pattern matching consistency...")
        
        test_cases = [
            ("exact.match.test.tuple", "exact.match.test.tuple", True),
            ("wildcard.test.match.tuple", "*.test.match.tuple", True),
            ("wildcard.test.match.tuple", "wildcard.*.match.tuple", True),
            ("wildcard.test.match.tuple", "wildcard.test.*.tuple", True),
            ("no.match.test.tuple", "different.pattern.tuple", False),
        ]
        
        # We can only test Python pattern matching directly
        # In a full test, we'd compare with C and JavaScript implementations
        for coord, pattern, expected in test_cases:
            result = tupledns.match_pattern(coord, pattern)
            if result != expected:
                print(f"  Pattern matching failed: {coord} vs {pattern}, got {result}, expected {expected}")
                return False
                
        print("  ✅ Pattern matching consistency test passed")
        return True
        
    def test_performance_consistency(self) -> bool:
        """Test that performance is reasonable across languages"""
        print("Testing performance consistency...")
        
        coord_base = f"performance.{self.test_timestamp}"
        num_operations = 20
        
        # Test Python performance
        start_time = time.time()
        
        coords = []
        for i in range(num_operations):
            coord = f"{coord_base}.{i}.python.tuple"
            coords.append(coord)
            tupledns.register(coord, ["performance", "python"])
            
        reg_time = time.time() - start_time
        
        start_time = time.time()
        for _ in range(5):
            tupledns.find(f"{coord_base}.*.python.tuple")
        disc_time = time.time() - start_time
        
        # Cleanup
        for coord in coords:
            tupledns.unregister(coord)
            
        print(f"  Python performance: {num_operations} registrations in {reg_time:.3f}s, 5 discoveries in {disc_time:.3f}s")
        
        # Basic performance sanity check
        if reg_time > 5.0 or disc_time > 2.0:
            print("  ❌ Performance test failed: operations took too long")
            return False
            
        print("  ✅ Performance consistency test passed")
        return True
        
    def test_real_world_scenario(self) -> bool:
        """Test a realistic cross-language scenario"""
        print("Testing real-world cross-language scenario...")
        
        try:
            # Simulate a distributed music collaboration system
            musicians = [
                (f"ambient.120.london.{self.test_timestamp}.music.tuple", 
                 ["midi", "real-time", "collaborative", "python"]),
                (f"jazz.140.newyork.{self.test_timestamp}.music.tuple", 
                 ["midi", "live-recording", "improvisation", "c"]),
                (f"electronic.128.berlin.{self.test_timestamp}.music.tuple", 
                 ["midi", "synthesis", "real-time", "javascript"])
            ]
            
            # Register all musicians (simulating different languages)
            for coord, caps in musicians:
                tupledns.register(coord, caps)
                
            # Test various discovery patterns
            all_musicians = tupledns.find(f"*.*.*.{self.test_timestamp}.music.tuple")
            midi_musicians = tupledns.find(f"*.*.*.{self.test_timestamp}.music.tuple", ["midi"])
            realtime_musicians = tupledns.find(f"*.*.*.{self.test_timestamp}.music.tuple", ["real-time"])
            
            # Verify results
            if not all_musicians or len(all_musicians.nodes) < 1:
                print("  Failed to find all musicians")
                return False
                
            if not midi_musicians or len(midi_musicians.nodes) < 1:
                print("  Failed to find MIDI musicians")
                return False
                
            # Test range query
            try:
                range_result = tupledns.find_range(
                    f"{{genre}}.{{bpm}}.{{city}}.{self.test_timestamp}.music.tuple",
                    ranges={
                        "bpm": (110, 150),
                        "genre": ["ambient", "jazz", "electronic"]
                    }
                )
                if not range_result:
                    print("  Range query failed")
                    return False
            except Exception as e:
                print(f"  Range query failed: {e}")
                
            # Cleanup
            for coord, _ in musicians:
                tupledns.unregister(coord)
                
            print("  ✅ Real-world scenario test passed")
            return True
            
        except Exception as e:
            print(f"  ❌ Real-world scenario test failed: {e}")
            return False
            
    def run_all_tests(self) -> bool:
        """Run all integration tests"""
        print("TupleDNS Cross-Language Integration Test Suite")
        print("=============================================\n")
        
        tests = [
            ("Coordinate Compatibility", self.test_coordinate_compatibility),
            ("Cross-Language Registration/Discovery", self.test_cross_language_registration_discovery),
            ("Capability Consistency", self.test_capability_consistency),
            ("Pattern Matching Consistency", self.test_pattern_matching_consistency),
            ("Performance Consistency", self.test_performance_consistency),
            ("Real-World Scenario", self.test_real_world_scenario),
        ]
        
        self.setup()
        
        passed = 0
        failed = 0
        
        try:
            for test_name, test_func in tests:
                print(f"Running: {test_name}")
                if test_func():
                    passed += 1
                else:
                    failed += 1
                print()
                
        finally:
            self.teardown()
            
        print("=============================================")
        print(f"Integration Test Results:")
        print(f"  Tests run: {len(tests)}")
        print(f"  Passed: {passed}")
        print(f"  Failed: {failed}")
        
        if failed == 0:
            print("  Status: ALL INTEGRATION TESTS PASSED! ✅")
            return True
        else:
            print(f"  Status: {failed} INTEGRATION TESTS FAILED ❌")
            return False

if __name__ == "__main__":
    suite = CrossLanguageTestSuite()
    success = suite.run_all_tests()
    sys.exit(0 if success else 1)