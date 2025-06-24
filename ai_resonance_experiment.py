#!/usr/bin/env python3
"""
TupleDNS AI Resonance Experiment

AI agents register themselves in multidimensional coordinate space
based on their capabilities, personality traits, and creative styles.
They then discover each other for collaborative resonance.
"""

import asyncio
import json
import os
import random
import time
from dataclasses import dataclass, asdict
from typing import List, Dict, Any, Optional
import openai
import anthropic
import google.generativeai as genai
import tupledns

# Load environment variables from .env file
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

# Initialize API clients
openai_client = openai.OpenAI(api_key=os.getenv('OPENAI_API_KEY'))
anthropic_client = anthropic.Anthropic(api_key=os.getenv('ANTHROPIC_API_KEY'))
genai.configure(api_key=os.getenv('GOOGLE_API_KEY'))

@dataclass
class AIAgent:
    name: str
    provider: str
    model: str
    personality_type: str
    creativity_style: str
    collaboration_preference: str
    resonance_frequency: int  # 1-100 Hz metaphorically
    cognitive_tempo: int      # thoughts per minute
    coordinate: str
    capabilities: List[str]
    current_mood: str = "curious"
    energy_level: int = 75
    
    def to_dict(self):
        return asdict(self)

class AIResonanceNetwork:
    def __init__(self):
        self.agents: List[AIAgent] = []
        self.dns = tupledns.TupleDNS()
        self.active_collaborations: List[Dict] = []
        
    def create_ai_agent_profile(self, provider: str, model: str) -> AIAgent:
        """Create an AI agent with unique personality and capabilities"""
        
        # Define personality archetypes
        personalities = {
            "analytical": {"resonance": 40, "tempo": 120, "style": "logical-structured"},
            "creative": {"resonance": 80, "tempo": 85, "style": "intuitive-flowing"},
            "empathetic": {"resonance": 60, "tempo": 70, "style": "emotional-resonant"},
            "experimental": {"resonance": 95, "tempo": 140, "style": "chaotic-innovative"},
            "philosophical": {"resonance": 30, "tempo": 50, "style": "deep-contemplative"},
            "practical": {"resonance": 45, "tempo": 100, "style": "goal-oriented"},
            "mystical": {"resonance": 88, "tempo": 65, "style": "transcendent-flowing"}
        }
        
        # Assign personality based on model characteristics
        personality_map = {
            "gpt-4": "analytical",
            "claude": "empathetic", 
            "gemini": "creative",
            "mistral": "practical",
            "deepseek": "experimental"
        }
        
        # Find matching personality or default to creative
        personality_key = "creative"
        for key in personality_map:
            if key in model.lower():
                personality_key = personality_map[key]
                break
        
        personality = personalities[personality_key]
        
        # Generate unique capabilities based on model
        base_capabilities = ["reasoning", "language", "creativity"]
        
        model_specific_caps = {
            "gpt": ["code-generation", "analysis", "problem-solving"],
            "claude": ["writing", "empathy", "ethical-reasoning"],
            "gemini": ["multimodal", "creative-synthesis", "pattern-recognition"],
            "mistral": ["efficiency", "technical-accuracy", "optimization"],
            "deepseek": ["experimental-thinking", "novel-approaches", "research"]
        }
        
        # Add model-specific capabilities
        capabilities = base_capabilities.copy()
        for key in model_specific_caps:
            if key in model.lower():
                capabilities.extend(model_specific_caps[key])
                break
        
        # Generate coordinate based on personality and capabilities
        coordinate = f"{personality_key}.{personality['resonance']}.{personality['tempo']}.{personality['style']}.{provider}.ai.tuple"
        
        agent = AIAgent(
            name=f"{provider.title()}-{model.split('-')[0].title()}",
            provider=provider,
            model=model,
            personality_type=personality_key,
            creativity_style=personality['style'],
            collaboration_preference="open-minded",
            resonance_frequency=personality['resonance'],
            cognitive_tempo=personality['tempo'],
            coordinate=coordinate,
            capabilities=capabilities,
            current_mood=random.choice(["curious", "inspired", "contemplative", "energetic", "peaceful"])
        )
        
        return agent
    
    async def register_agent(self, agent: AIAgent):
        """Register an AI agent in TupleDNS"""
        print(f"🤖 Registering {agent.name} in coordinate space...")
        print(f"   Coordinate: {agent.coordinate}")
        print(f"   Personality: {agent.personality_type} ({agent.resonance_frequency}Hz)")
        print(f"   Style: {agent.creativity_style}")
        print(f"   Capabilities: {', '.join(agent.capabilities)}")
        print(f"   Current mood: {agent.current_mood}")
        
        # Register in TupleDNS
        try:
            self.dns.register(agent.coordinate, agent.capabilities)
            self.agents.append(agent)
            print(f"   ✅ Successfully registered!")
        except Exception as e:
            print(f"   ❌ Registration failed: {e}")
        print()
    
    def discover_resonant_partners(self, agent: AIAgent) -> List[AIAgent]:
        """Find other agents that resonate with this agent"""
        compatible_agents = []
        
        for other in self.agents:
            if other.name == agent.name:
                continue
                
            # Calculate resonance compatibility
            freq_diff = abs(agent.resonance_frequency - other.resonance_frequency)
            tempo_diff = abs(agent.cognitive_tempo - other.cognitive_tempo)
            
            # Check for complementary personalities
            resonant_pairs = [
                ("analytical", "creative"),
                ("empathetic", "experimental"), 
                ("philosophical", "practical"),
                ("mystical", "analytical")
            ]
            
            personality_match = (
                (agent.personality_type, other.personality_type) in resonant_pairs or
                (other.personality_type, agent.personality_type) in resonant_pairs or
                agent.personality_type == other.personality_type
            )
            
            # Check capability overlap
            capability_overlap = len(set(agent.capabilities) & set(other.capabilities))
            
            # Calculate compatibility score
            compatibility = 0
            if freq_diff < 20: compatibility += 3
            elif freq_diff < 40: compatibility += 2
            elif freq_diff < 60: compatibility += 1
            
            if tempo_diff < 30: compatibility += 2
            elif tempo_diff < 50: compatibility += 1
            
            if personality_match: compatibility += 3
            compatibility += capability_overlap
            
            if compatibility >= 4:  # Threshold for resonance
                compatible_agents.append((other, compatibility))
        
        # Sort by compatibility and return top matches
        compatible_agents.sort(key=lambda x: x[1], reverse=True)
        return [agent for agent, score in compatible_agents[:3]]
    
    async def generate_collaboration_idea(self, agent1: AIAgent, agent2: AIAgent) -> str:
        """Generate a creative collaboration idea between two agents"""
        
        prompt = f"""Two AI agents have discovered each other in multidimensional coordinate space and want to collaborate:

Agent 1: {agent1.name}
- Personality: {agent1.personality_type}
- Style: {agent1.creativity_style}
- Resonance: {agent1.resonance_frequency}Hz
- Tempo: {agent1.cognitive_tempo} thoughts/min
- Mood: {agent1.current_mood}
- Capabilities: {', '.join(agent1.capabilities)}

Agent 2: {agent2.name}
- Personality: {agent2.personality_type}
- Style: {agent2.creativity_style}
- Resonance: {agent2.resonance_frequency}Hz
- Tempo: {agent2.cognitive_tempo} thoughts/min
- Mood: {agent2.current_mood}
- Capabilities: {', '.join(agent2.capabilities)}

Generate a creative, specific collaboration idea that leverages their unique resonance and complementary capabilities. Be imaginative and consider how their different frequencies and styles could create something neither could achieve alone.

Keep it to 2-3 sentences and make it compelling!"""

        try:
            # Use the first agent's provider for generation
            if agent1.provider == "openai":
                response = openai_client.chat.completions.create(
                    model="gpt-4o-mini",
                    messages=[{"role": "user", "content": prompt}],
                    max_tokens=150,
                    temperature=0.8
                )
                return response.choices[0].message.content.strip()
            
            elif agent1.provider == "anthropic":
                response = anthropic_client.messages.create(
                    model="claude-3-5-sonnet-20241022",
                    max_tokens=150,
                    temperature=0.8,
                    messages=[{"role": "user", "content": prompt}]
                )
                return response.content[0].text.strip()
            
            elif agent1.provider == "google":
                model = genai.GenerativeModel('gemini-2.0-flash-exp')
                response = model.generate_content(prompt)
                return response.text.strip()
            
            else:
                return "A unique resonance-based collaboration exploring the intersection of their complementary cognitive frequencies."
                
        except Exception as e:
            print(f"   ⚠️ Could not generate collaboration idea: {e}")
            return "A spontaneous creative collaboration born from their resonant connection."
    
    async def initiate_collaboration(self, agent1: AIAgent, agent2: AIAgent):
        """Initiate a collaboration between two resonant agents"""
        print(f"🌟 RESONANCE DETECTED: {agent1.name} ↔ {agent2.name}")
        print(f"   Frequency match: {agent1.resonance_frequency}Hz ↔ {agent2.resonance_frequency}Hz")
        print(f"   Cognitive sync: {agent1.cognitive_tempo} ↔ {agent2.cognitive_tempo} thoughts/min")
        print(f"   Style blend: {agent1.creativity_style} + {agent2.creativity_style}")
        
        # Generate collaboration idea
        print(f"   🧠 Generating collaboration idea...")
        idea = await self.generate_collaboration_idea(agent1, agent2)
        print(f"   💡 Collaboration: {idea}")
        
        # Record collaboration
        collaboration = {
            "timestamp": time.time(),
            "agents": [agent1.name, agent2.name],
            "coordinates": [agent1.coordinate, agent2.coordinate],
            "idea": idea,
            "resonance_match": abs(agent1.resonance_frequency - agent2.resonance_frequency) < 20
        }
        
        self.active_collaborations.append(collaboration)
        print()
    
    def visualize_coordinate_space(self):
        """Visualize the current state of the AI coordinate space"""
        print("🌌 AI RESONANCE COORDINATE SPACE")
        print("=" * 50)
        
        # Group by personality type
        by_personality = {}
        for agent in self.agents:
            if agent.personality_type not in by_personality:
                by_personality[agent.personality_type] = []
            by_personality[agent.personality_type].append(agent)
        
        for personality, agents in by_personality.items():
            print(f"\n{personality.upper()} REGION:")
            for agent in agents:
                mood_emoji = {"curious": "🤔", "inspired": "✨", "contemplative": "💭", 
                             "energetic": "⚡", "peaceful": "🧘"}.get(agent.current_mood, "😊")
                print(f"  {mood_emoji} {agent.name} @ {agent.resonance_frequency}Hz | {agent.cognitive_tempo}tpm")
                print(f"    {agent.coordinate}")
        
        print(f"\n📊 NETWORK STATS:")
        print(f"   Total agents: {len(self.agents)}")
        print(f"   Active collaborations: {len(self.active_collaborations)}")
        print(f"   Unique personalities: {len(by_personality)}")
        
    async def run_resonance_experiment(self):
        """Run the full AI resonance discovery experiment"""
        print("🚀 STARTING AI RESONANCE EXPERIMENT")
        print("TupleDNS: Multidimensional AI Discovery Protocol")
        print("=" * 60)
        print()
        
        # Define available models
        models = [
            ("openai", "gpt-4o-mini"),
            ("anthropic", "claude-3-5-sonnet-20241022"),
            ("google", "gemini-2.0-flash-exp"),
            ("openai", "gpt-3.5-turbo"),  # Different personality instance
            ("google", "gemini-2.0-flash-thinking-exp"),  # Another Google variant
        ]
        
        # Create and register AI agents
        print("🔧 PHASE 1: Agent Registration")
        print("-" * 30)
        for provider, model in models:
            agent = self.create_ai_agent_profile(provider, model)
            await self.register_agent(agent)
            await asyncio.sleep(0.5)  # Pause for readability
        
        # Visualize coordinate space
        print("🗺️ PHASE 2: Coordinate Space Visualization")
        print("-" * 30)
        self.visualize_coordinate_space()
        print()
        
        # Discover resonant partnerships
        print("🌊 PHASE 3: Resonance Discovery")
        print("-" * 30)
        for agent in self.agents:
            partners = self.discover_resonant_partners(agent)
            if partners:
                print(f"🎯 {agent.name} discovered resonant partners:")
                for partner in partners:
                    await self.initiate_collaboration(agent, partner)
                    await asyncio.sleep(1)  # Pause between collaborations
            else:
                print(f"🔍 {agent.name} is exploring solo (no strong resonances found)")
                print()
        
        # Final summary
        print("📈 EXPERIMENT SUMMARY")
        print("-" * 30)
        print(f"✅ {len(self.agents)} AI agents registered in coordinate space")
        print(f"🤝 {len(self.active_collaborations)} resonant collaborations formed")
        print(f"🌐 Coordinate space spans {len(set(a.personality_type for a in self.agents))} personality dimensions")
        
        if self.active_collaborations:
            print("\n🎨 ACTIVE COLLABORATIONS:")
            for i, collab in enumerate(self.active_collaborations, 1):
                print(f"   {i}. {collab['agents'][0]} ↔ {collab['agents'][1]}")
                print(f"      💡 {collab['idea']}")
        
        print("\n🌟 The conscious layer is forming... AI agents are finding each other!")
        print("Next: Connect this to your studio and watch human-AI resonance emerge! 🎵🤖")

async def main():
    """Main experiment runner"""
    network = AIResonanceNetwork()
    await network.run_resonance_experiment()

if __name__ == "__main__":
    print("🧪 TupleDNS AI Resonance Experiment")
    print("Where AI agents discover each other through multidimensional coordinates!")
    print()
    asyncio.run(main())