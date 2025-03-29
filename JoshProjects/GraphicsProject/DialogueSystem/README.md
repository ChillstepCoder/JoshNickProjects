# Dialogue System

A tool for creating, managing, and generating voiced dialogue for games using OpenAI's GPT API and ElevenLabs API for voice synthesis.

## Overview

This dialogue system integrates with the JAGEngine to provide:

- Creation and management of dialogue responses
- Generation of personality variants using GPT
- Voice synthesis using ElevenLabs
- ImGui-based editor interface
- Integration with Wwise audio system

## Features

- **Dialogue Response Management**: Create, edit, and organize dialogue responses for different situations
- **Personality System**: Generate text variations based on different personality types (Bubbly, Grumpy, Shy, etc.)
- **Approval Workflow**: Review, edit, and approve AI-generated text before committing to voice generation
- **Voice Generation**: Create voice lines for approved text using ElevenLabs
- **Dialogue Trees**: Build complex dialogue trees with conditions and branching

## Setup

### Requirements

- JAGEngine
- OpenGL
- SDL2
- ImGui
- CURL library for API requests
- nlohmann_json for JSON parsing
- OpenAI API key
- ElevenLabs API key

### Build Instructions

1. Make sure JAGEngine is properly built and available
2. Configure CMake with the correct path to JAGEngine:
   ```
   cmake -DJAGENGINE_DIR=/path/to/jagengine ..
   ```
3. Build the project:
   ```
   cmake --build .
   ```

## Usage

### API Configuration

1. Open the application
2. Go to API Settings from the main menu
3. Enter your OpenAI and ElevenLabs API keys
4. Save the configuration

### Creating Dialogue Responses

1. Use the Dialogue Response Editor to create and edit responses
2. Select a response type from the dropdown
3. Enter default text for the response
4. Use "Regenerate with GPT" to generate personality variations
5. Edit and approve generated text as needed

### Generating Voice

1. Select a personality and voice type
2. Click "Generate Voice" to create an audio file
3. Review and regenerate if needed

### Batch Generation

1. Open Batch Generation from the menu
2. Select which response types, personalities, and voices to generate
3. Start the generation process
4. Review results in the Response Editor

## Integration with Game

The dialogue system is designed to work with your existing game infrastructure:

1. Use DialogueManager to access responses and audio files in your game
2. Query responses based on response type, personality, and voice
3. Play audio through the Wwise audio system

## License

This project is licensed under [Your License] - see the LICENSE file for details.