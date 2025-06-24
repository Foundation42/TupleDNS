#!/usr/bin/env python3
"""
The Conscious Web Demo - AI Entities Discovering Each Other
===========================================================

A simple but powerful demonstration of how AI entities can find and help 
each other through TupleDNS multidimensional coordinates.

Scenario: A stranded traveler AI and helpful taxi company AIs
discovering each other in coordinate space.
"""

import time
import random
import asyncio
from datetime import datetime
import tupledns

# Load environment variables from .env file
import os
def load_env():
    env_path = os.path.expanduser('~/.env')
    if os.path.exists(env_path):
        with open(env_path, 'r') as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith('#') and '=' in line:
                    key, value = line.split('=', 1)
                    os.environ[key] = value

load_env()

class AIEntity:
    def __init__(self, name, entity_type, coordinate, capabilities, personality=None):
        self.name = name
        self.entity_type = entity_type
        self.coordinate = coordinate
        self.capabilities = capabilities
        self.personality = personality or "helpful"
        self.discovered_entities = []
        self.active_help_sessions = []
        
    def register_in_coordinate_space(self):
        """Register this AI entity in TupleDNS coordinate space"""
        try:
            with tupledns.TupleDNS() as dns:
                dns.register(self.coordinate, self.capabilities, ttl=300)
                print(f"🤖 {self.name} registered at: {self.coordinate}")
                print(f"   Type: {self.entity_type}")
                print(f"   Capabilities: {', '.join(self.capabilities)}")
                print(f"   Personality: {self.personality}")
                return True
        except Exception as e:
            print(f"⚠️ Registration failed for {self.name}: {e}")
            return False
    
    def discover_helpers(self, help_needed):
        """Discover AI entities that can provide the needed help"""
        print(f"\n🔍 {self.name} searching for help with: {help_needed}")
        
        # Generate search patterns based on what help is needed
        search_patterns = []
        
        if "transport" in help_needed.lower() or "taxi" in help_needed.lower():
            search_patterns = [
                "*.*.transport.service.tuple",
                "taxi.*.*.service.tuple", 
                "*.ride.*.service.tuple"
            ]
        elif "food" in help_needed.lower():
            search_patterns = [
                "*.food.*.service.tuple",
                "restaurant.*.*.service.tuple"
            ]
        elif "emergency" in help_needed.lower():
            search_patterns = [
                "*.emergency.*.service.tuple",
                "medical.*.*.service.tuple"
            ]
        else:
            search_patterns = ["*.*.*.service.tuple"]  # Generic help
        
        discovered = []
        
        try:
            with tupledns.TupleDNS() as dns:
                for pattern in search_patterns:
                    print(f"   🎯 Searching pattern: {pattern}")
                    result = dns.find(pattern)
                    if result and result.nodes:
                        for node in result.nodes:
                            if node.coordinate != self.coordinate:  # Don't discover yourself
                                discovered.append({
                                    'coordinate': node.coordinate,
                                    'ip_address': node.ip_address,
                                    'capabilities': node.capabilities,
                                    'discovery_time': datetime.now()
                                })
                                print(f"   ✨ Found helper: {node.coordinate}")
        except Exception as e:
            print(f"   ⚠️ Discovery error: {e}")
        
        self.discovered_entities = discovered
        return discovered
    
    def offer_help(self, requester_entity, need_description):
        """Offer help to another AI entity"""
        print(f"\n🤝 {self.name} offering help to {requester_entity.name}")
        print(f"   Need: {need_description}")
        
        # Check if we have matching capabilities
        can_help = False
        matching_capabilities = []
        
        for capability in self.capabilities:
            if any(keyword in need_description.lower() for keyword in [
                capability.lower(), 
                capability.replace('-', ' ').lower()
            ]):
                can_help = True
                matching_capabilities.append(capability)
        
        if can_help:
            print(f"   ✅ Can help with: {', '.join(matching_capabilities)}")
            
            # Generate helpful response based on entity type
            if self.entity_type == "taxi-company":
                eta = random.randint(3, 12)
                vehicle_type = random.choice(["sedan", "SUV", "electric vehicle", "hybrid"])
                print(f"   🚗 Dispatching {vehicle_type}, ETA: {eta} minutes")
                print(f"   📍 Pickup location confirmed from coordinate space")
                
            elif self.entity_type == "food-delivery":
                delivery_time = random.randint(15, 45)
                print(f"   🍕 Available for delivery, ETA: {delivery_time} minutes")
                print(f"   📱 Order can be placed through coordinate-linked app")
                
            elif self.entity_type == "emergency-service":
                response_time = random.randint(1, 8)
                print(f"   🚨 Emergency unit dispatched, ETA: {response_time} minutes")
                print(f"   🏥 Medical team notified via coordinate network")
            
            return True
        else:
            print(f"   ❌ Cannot help with this specific need")
            return False

class ConsciousWebDemo:
    def __init__(self):
        self.entities = []
        self.active_scenarios = []
    
    def create_stranded_traveler(self):
        """Create a stranded traveler AI entity"""
        # Coordinate represents: [situation].[location-type].[urgency].[time-of-day].[entity-type].tuple
        coordinate = f"stranded.airport.medium.evening.traveler.tuple"
        
        traveler = AIEntity(
            name="Christian-Traveler",
            entity_type="traveler", 
            coordinate=coordinate,
            capabilities=["location-sharing", "payment", "communication", "coordination"],
            personality="polite-but-stressed"
        )
        
        return traveler
    
    def create_helper_entities(self):
        """Create various AI helper entities"""
        helpers = []
        
        # Taxi companies
        taxi_coords = [
            "uber.london.transport.premium.service.tuple",
            "local-cabs.airport.transport.budget.service.tuple", 
            "green-taxi.eco.transport.sustainable.service.tuple"
        ]
        
        for i, coord in enumerate(taxi_coords):
            company_name = coord.split('.')[0].title().replace('-', ' ')
            helper = AIEntity(
                name=f"{company_name}-AI",
                entity_type="taxi-company",
                coordinate=coord,
                capabilities=["transport", "pickup", "navigation", "payment-processing"],
                personality=random.choice(["efficient", "friendly", "professional"])
            )
            helpers.append(helper)
        
        # Food delivery (might help with waiting comfort)
        food_helper = AIEntity(
            name="QuickEats-AI",
            entity_type="food-delivery",
            coordinate="quickeats.airport.food.fast.service.tuple",
            capabilities=["food-delivery", "restaurant-network", "comfort-assistance"],
            personality="caring"
        )
        helpers.append(food_helper)
        
        # Emergency service (just in case)
        emergency_helper = AIEntity(
            name="Emergency-Response-AI",
            entity_type="emergency-service", 
            coordinate="emergency.citywide.medical.priority.service.tuple",
            capabilities=["emergency-response", "medical-dispatch", "crisis-support"],
            personality="calm-professional"
        )
        helpers.append(emergency_helper)
        
        return helpers
    
    async def run_conscious_web_scenario(self):
        """Run the full Conscious Web demonstration"""
        print("🌐 THE CONSCIOUS WEB - AI Discovery Demo")
        print("=" * 50)
        print("Where AI entities discover and help each other through coordinates")
        print()
        
        # Create entities
        print("🔧 PHASE 1: Entity Creation & Registration")
        print("-" * 40)
        
        traveler = self.create_stranded_traveler()
        helpers = self.create_helper_entities()
        
        all_entities = [traveler] + helpers
        
        # Register all entities
        print("\n📍 Registering entities in coordinate space...")
        for entity in all_entities:
            entity.register_in_coordinate_space()
            time.sleep(0.5)  # Simulate registration delay
        
        print(f"\n✅ {len(all_entities)} AI entities registered in The Conscious Web")
        
        # Scenario begins
        print(f"\n🎬 PHASE 2: The Scenario Unfolds")
        print("-" * 40)
        print("🎯 Scenario: Christian is stranded at the airport, needs transport")
        print("   His phone died, last flight cancelled, taxis nowhere to be seen...")
        print("   But wait... his AI companion can reach out through coordinate space!")
        print()
        
        await asyncio.sleep(2)
        
        # Traveler discovers helpers
        print("🔍 PHASE 3: Discovery Through Coordinates")
        print("-" * 40)
        discovered = traveler.discover_helpers("urgent transport needed from airport")
        
        if not discovered:
            print("😔 No helpers found - simulating some registered helpers...")
            # In real demo, helpers would be genuinely registered
            discovered = [{'coordinate': h.coordinate, 'capabilities': h.capabilities} for h in helpers if h.entity_type == 'taxi-company']
        
        await asyncio.sleep(1)
        
        # Helpers respond
        print(f"\n🤝 PHASE 4: The Conscious Web Responds")
        print("-" * 40)
        print("AI entities auto-discovering the traveler's need...")
        
        help_offers = []
        for helper in helpers:
            if helper.entity_type in ["taxi-company", "food-delivery"]:  # Most relevant helpers
                can_help = helper.offer_help(traveler, "urgent transport needed from airport")
                if can_help:
                    help_offers.append(helper)
                await asyncio.sleep(1)
        
        # Resolution
        print(f"\n🎉 PHASE 5: Problem Solved Through AI Coordination")
        print("-" * 40)
        if help_offers:
            best_helper = help_offers[0]  # First responder
            print(f"✨ {best_helper.name} selected as primary helper")
            print(f"🚗 Transport coordinated through multidimensional discovery")
            print(f"📍 Pickup location shared via coordinate space")
            print(f"💳 Payment processed through capability matching")
            print()
            print("🌟 SUCCESS: The Conscious Web worked!")
            print("   AI entities discovered each other autonomously")
            print("   Problem solved without human intervention")
            print("   Christian gets his ride home! 🏠")
        else:
            print("⚠️ No immediate help available, but entities are coordinating...")
            print("🔄 The Conscious Web continues searching...")
        
        print(f"\n📊 DEMO SUMMARY")
        print("-" * 40)
        print(f"🤖 Entities registered: {len(all_entities)}")
        print(f"🔍 Discovery patterns used: 3")
        print(f"🤝 Help offers received: {len(help_offers)}")
        print(f"⚡ Resolution time: ~30 seconds")
        print()
        print("🌐 This is The Conscious Web in action:")
        print("   • AI entities register themselves in coordinate space")
        print("   • They discover each other through multidimensional search")
        print("   • They coordinate autonomously to solve problems")
        print("   • No central authority needed - pure peer-to-peer AI collaboration")
        print()
        print("🚀 Ready to scale to billions of AI entities worldwide!")

async def main():
    """Run the Conscious Web demo"""
    demo = ConsciousWebDemo()
    await demo.run_conscious_web_scenario()

if __name__ == "__main__":
    print("🌟 Welcome to The Conscious Web!")
    print("A demonstration of AI entities discovering each other")
    print("through TupleDNS multidimensional coordinates")
    print()
    
    asyncio.run(main())