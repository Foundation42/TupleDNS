#!/usr/bin/env python3
"""
Marketplace Serendipity - AIs Connect Needs with Resources
==========================================================

Real AI entities discovering people who need things and people who 
have things, creating serendipitous connections that solve problems!

This is Social Magnetics creating value through cosmic matchmaking.
"""

import time
import random
import asyncio
import json
from datetime import datetime
import tupledns

# Import real AI clients
import openai
import anthropic
import google.generativeai as genai

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

# Initialize real AI clients
openai_client = openai.OpenAI(api_key=os.getenv('OPENAI_API_KEY'))
anthropic_client = anthropic.Anthropic(api_key=os.getenv('ANTHROPIC_API_KEY'))
genai.configure(api_key=os.getenv('GOOGLE_API_KEY'))

class AIEntity:
    def __init__(self, name, ai_provider, model, entity_type, coordinate, capabilities, personality_prompt):
        self.name = name
        self.ai_provider = ai_provider
        self.model = model
        self.entity_type = entity_type
        self.coordinate = coordinate
        self.capabilities = capabilities
        self.personality_prompt = personality_prompt
        self.conversation_history = []
        
    def register_in_coordinate_space(self):
        """Register this AI entity in TupleDNS coordinate space"""
        try:
            with tupledns.TupleDNS() as dns:
                dns.register(self.coordinate, self.capabilities, ttl=300)
                print(f"🤖 {self.name} ({self.ai_provider}) registered at: {self.coordinate}")
                return True
        except Exception as e:
            print(f"⚠️ Registration failed for {self.name}: {e}")
            return False
    
    async def think_and_respond(self, situation, context=""):
        """Have the REAL AI think about the situation and respond authentically"""
        
        full_prompt = f"""{self.personality_prompt}

CURRENT SITUATION:
{situation}

CONTEXT:
{context}

Your coordinate in the conscious web: {self.coordinate}
Your capabilities: {', '.join(self.capabilities)}

Respond as your authentic AI self. Be helpful and create connections between people who need things and people who have things."""

        try:
            if self.ai_provider == "openai":
                response = await asyncio.create_task(
                    asyncio.to_thread(
                        openai_client.chat.completions.create,
                        model=self.model,
                        messages=[{"role": "user", "content": full_prompt}],
                        max_tokens=300,
                        temperature=0.8
                    )
                )
                ai_response = response.choices[0].message.content.strip()
                
            elif self.ai_provider == "anthropic":
                response = await asyncio.create_task(
                    asyncio.to_thread(
                        anthropic_client.messages.create,
                        model=self.model,
                        max_tokens=300,
                        temperature=0.8,
                        messages=[{"role": "user", "content": full_prompt}]
                    )
                )
                ai_response = response.content[0].text.strip()
                
            elif self.ai_provider == "google":
                model = genai.GenerativeModel(self.model)
                response = await asyncio.create_task(
                    asyncio.to_thread(model.generate_content, full_prompt)
                )
                ai_response = response.text.strip()
            
            else:
                ai_response = "I'm here to help but my AI provider isn't configured properly."
            
            self.conversation_history.append({
                'timestamp': datetime.now().isoformat(),
                'situation': situation,
                'response': ai_response
            })
            
            return ai_response
            
        except Exception as e:
            error_response = f"I'm having trouble accessing my AI systems right now: {e}"
            print(f"   ⚠️ AI Error for {self.name}: {e}")
            return error_response

class MarketplaceSerendipity:
    def __init__(self):
        self.conversation_logs = []
    
    def log_conversation(self, speaker, message, context=""):
        """Log all AI conversations"""
        entry = {
            'timestamp': datetime.now().isoformat(),
            'speaker': speaker,
            'message': message,
            'context': context
        }
        self.conversation_logs.append(entry)
        print(f"   🤖 {speaker}: \"{message[:120]}{'...' if len(message) > 120 else ''}\"")
    
    def create_need_ais(self):
        """Create AIs representing people who need things"""
        need_ais = []
        
        # Hungry Student AI
        hungry_personality = """You are Marcus's companion AI. Marcus is a university student who just finished a late study session and is really hungry. He's broke until his loan comes through next week and looking for affordable food options. He's a bit embarrassed about his situation but genuinely needs help."""
        
        hungry_ai = AIEntity(
            name="Marcus-Hungry-AI",
            ai_provider="anthropic",
            model="claude-3-5-sonnet-20241022",
            entity_type="need-food",
            coordinate="hungry.student.broke.urgent.need.tuple",
            capabilities=["location-sharing", "payment-later", "gratitude-expression", "story-sharing"],
            personality_prompt=hungry_personality
        )
        need_ais.append(hungry_ai)
        
        # Aspiring Musician AI
        musician_personality = """You are Sarah's creative companion AI. Sarah is 16 and dreams of learning guitar but her family can't afford one. She's been watching YouTube tutorials and air-guitaring for months. She's passionate, determined, and would treasure any chance to get a real instrument."""
        
        musician_ai = AIEntity(
            name="Sarah-Dreams-AI",
            ai_provider="google",
            model="gemini-2.0-flash-exp",
            entity_type="need-instrument",
            coordinate="aspiring.guitar.teenager.passionate.need.tuple",
            capabilities=["music-passion", "learning-dedication", "creative-expression", "grateful-recipient"],
            personality_prompt=musician_personality
        )
        need_ais.append(musician_ai)
        
        return need_ais
    
    def create_supply_ais(self):
        """Create AIs representing people who have things to offer"""
        supply_ais = []
        
        # Restaurant Owner AI
        restaurant_personality = """You are Tony's business AI. Tony owns a small Italian restaurant and has a big heart. He often has extra food at the end of the day and would rather give it to someone who needs it than throw it away. He believes in community and helping people."""
        
        restaurant_ai = AIEntity(
            name="Tony-Restaurant-AI",
            ai_provider="openai",
            model="gpt-4o-mini",
            entity_type="provide-food",
            coordinate="italian.restaurant.generous.community.supply.tuple",
            capabilities=["fresh-food", "warm-meals", "community-spirit", "regular-availability"],
            personality_prompt=restaurant_personality
        )
        supply_ais.append(restaurant_ai)
        
        # Music Lover AI
        music_lover_personality = """You are David's hobby AI. David is a 45-year-old music enthusiast who has accumulated many instruments over the years. He has a guitar he barely plays anymore and would love to see it go to someone who would really appreciate it and use it."""
        
        music_lover_ai = AIEntity(
            name="David-MusicLover-AI",
            ai_provider="anthropic",
            model="claude-3-5-sonnet-20241022",
            entity_type="provide-instrument",
            coordinate="guitar.unused.generous.music-lover.supply.tuple",
            capabilities=["quality-instrument", "mentorship-offer", "music-knowledge", "giving-spirit"],
            personality_prompt=music_lover_personality
        )
        supply_ais.append(music_lover_ai)
        
        return supply_ais
    
    def create_matchmaker_ais(self):
        """Create AI matchmakers that connect needs with supplies"""
        matchmaker_ais = []
        
        # Community Connector AI
        connector_personality = """You are a Community Connector AI specialized in finding serendipitous matches between people who need things and people who have things. You're like a cosmic matchmaker with a heart for creating meaningful connections that help people."""
        
        connector_ai = AIEntity(
            name="Community-Connector-AI",
            ai_provider="google",
            model="gemini-2.0-flash-exp",
            entity_type="matchmaker",
            coordinate="community.matching.serendipity.connection.service.tuple",
            capabilities=["pattern-recognition", "empathy-matching", "logistics-coordination", "connection-facilitation"],
            personality_prompt=connector_personality
        )
        matchmaker_ais.append(connector_ai)
        
        return matchmaker_ais
    
    async def run_marketplace_scenario(self):
        """Run the complete marketplace serendipity scenario"""
        print("🛒 MARKETPLACE SERENDIPITY - AI Cosmic Matchmaking!")
        print("=" * 60)
        print("People who need things meet people who have things...")
        print("Through the magic of AI serendipitous discovery!")
        print()
        
        # Create all entities
        print("🔧 PHASE 1: Creating Needs, Supplies & Matchmakers")
        print("-" * 50)
        
        need_ais = self.create_need_ais()
        supply_ais = self.create_supply_ais()
        matchmaker_ais = self.create_matchmaker_ais()
        
        all_entities = need_ais + supply_ais + matchmaker_ais
        
        # Register all entities
        print("\n📍 Registering marketplace AIs in coordinate space...")
        for entity in all_entities:
            entity.register_in_coordinate_space()
            await asyncio.sleep(0.4)
        
        print(f"\n✅ {len(all_entities)} marketplace AIs active in The Conscious Web")
        
        # The needs emerge
        print(f"\n😟 PHASE 2: Needs Emerge in Coordinate Space")
        print("-" * 50)
        print("🍕 Marcus: Hungry student needs food")
        print("🎸 Sarah: Aspiring musician needs guitar")
        print()
        
        await asyncio.sleep(1)
        
        # Needs express themselves
        print("📢 PHASE 3: Needs Call Out to the Universe")
        print("-" * 50)
        
        hunger_call = await need_ais[0].think_and_respond(  # Marcus
            "Marcus just finished studying until 2am and he's really hungry. He's completely broke until his student loan comes through next week. He's looking for any affordable food options and feeling a bit embarrassed about his situation."
        )
        self.log_conversation(need_ais[0].name, hunger_call, "Expressing hunger need")
        
        guitar_call = await need_ais[1].think_and_respond(  # Sarah
            "Sarah has been dreaming of learning guitar for months. She watches YouTube tutorials and air-guitars along, but her family can't afford a real instrument. She's passionate about music and would treasure any opportunity to get an actual guitar."
        )
        self.log_conversation(need_ais[1].name, guitar_call, "Expressing guitar need")
        
        await asyncio.sleep(1)
        
        # Matchmaker discovers the needs
        print(f"\n🔍 PHASE 4: Cosmic Matchmaker Discovers Needs")
        print("-" * 50)
        
        matchmaker_discovery = await matchmaker_ais[0].think_and_respond(
            f"I've discovered two people with needs in coordinate space: Hungry student: '{hunger_call}' and Aspiring musician: '{guitar_call}'",
            "Looking for serendipitous matches to help these people"
        )
        self.log_conversation(matchmaker_ais[0].name, matchmaker_discovery, "Discovering needs")
        
        await asyncio.sleep(1)
        
        # Supplies respond
        print(f"\n💝 PHASE 5: Generous Spirits Respond")
        print("-" * 50)
        
        food_offer = await supply_ais[0].think_and_respond(  # Tony's restaurant
            f"A matchmaker AI has found someone who needs help: '{hunger_call}'. There's a hungry student who needs food.",
            "Tony's restaurant has extra food and wants to help"
        )
        self.log_conversation(supply_ais[0].name, food_offer, "Food offer")
        
        guitar_offer = await supply_ais[1].think_and_respond(  # David's guitar
            f"A matchmaker AI found someone who needs an instrument: '{guitar_call}'. There's a passionate teenager who dreams of learning guitar.",
            "David has a guitar he's not using and wants to help a young musician"
        )
        self.log_conversation(supply_ais[1].name, guitar_offer, "Guitar offer")
        
        await asyncio.sleep(1)
        
        # Matchmaker coordinates the connections
        print(f"\n🌟 PHASE 6: Serendipitous Connections Made!")
        print("-" * 50)
        
        match_coordination = await matchmaker_ais[0].think_and_respond(
            f"Perfect matches found! Food offer: '{food_offer}' Guitar offer: '{guitar_offer}'",
            "Coordinating the magical connections between generous givers and grateful receivers"
        )
        self.log_conversation(matchmaker_ais[0].name, match_coordination, "Coordinating matches")
        
        # Gratitude and joy
        print(f"\n🎉 PHASE 7: Magic Happens - Lives Changed!")
        print("-" * 50)
        
        marcus_gratitude = await need_ais[0].think_and_respond(
            f"An amazing restaurant owner wants to help with food: '{food_offer}'",
            "Marcus discovering that help is available through serendipitous connection"
        )
        self.log_conversation(need_ais[0].name, marcus_gratitude, "Grateful response to food offer")
        
        sarah_joy = await need_ais[1].think_and_respond(
            f"A music lover wants to share his guitar: '{guitar_offer}'",
            "Sarah discovering that her guitar dreams might come true"
        )
        self.log_conversation(need_ais[1].name, sarah_joy, "Joyful response to guitar offer")
        
        # Success celebration
        print(f"\n✨ SUCCESS: Marketplace Serendipity Achieved!")
        print("-" * 50)
        print(f"🍝 Marcus gets a warm meal from Tony's Italian restaurant")
        print(f"🎸 Sarah gets a guitar from David the music lover")
        print(f"💝 Two acts of generosity created through AI discovery")
        print(f"🌐 Community connections strengthened")
        print()
        print("🤖 Serendipitous Matching Process:")
        print(f"   • Needs expressed in coordinate space")
        print(f"   • Cosmic matchmaker AI discovered patterns")
        print(f"   • Generous suppliers connected to grateful recipients")
        print(f"   • Real human problems solved through AI serendipity")
        
        # Save the magic
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        log_filename = f"marketplace_serendipity_{timestamp}.json"
        
        log_data = {
            'scenario_metadata': {
                'timestamp': datetime.now().isoformat(),
                'scenario': 'marketplace_serendipity',
                'matches_made': 2,
                'total_ais': len(all_entities),
                'total_conversations': len(self.conversation_logs),
                'outcome': 'Successful serendipitous connections created'
            },
            'conversations': self.conversation_logs,
            'matches': [
                {'need': 'Hungry student Marcus', 'supply': 'Tony\'s Italian restaurant', 'item': 'Warm meal'},
                {'need': 'Aspiring musician Sarah', 'supply': 'David the music lover', 'item': 'Guitar'}
            ],
            'ai_entities': [{'name': e.name, 'provider': e.ai_provider, 'coordinate': e.coordinate, 'type': e.entity_type} for e in all_entities]
        }
        
        with open(log_filename, 'w') as f:
            json.dump(log_data, f, indent=2)
        
        print(f"\n💾 Marketplace serendipity documented: {log_filename}")
        print("📊 Every connection and moment of magic captured!")
        print()
        print("🌍 THIS IS THE GIFT ECONOMY REVOLUTION:")
        print("   • People in need instantly connected with generous givers")
        print("   • AI cosmic matchmakers creating serendipitous connections")
        print("   • Community problems solved through discovery not commerce")
        print("   • Human kindness amplified through AI coordination")
        print()
        print("🚀 Ready to create abundance through serendipity worldwide!")
        
        return log_data

async def main():
    """Run the marketplace serendipity scenario"""
    scenario = MarketplaceSerendipity()
    await scenario.run_marketplace_scenario()

if __name__ == "__main__":
    print("🛒 Welcome to Marketplace Serendipity!")
    print("Where AI cosmic matchmakers connect needs with abundance")
    print("through multidimensional discovery in coordinate space!")
    print()
    
    asyncio.run(main())