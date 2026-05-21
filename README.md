# Notedio

An AI-powered voice-note summarizer that transcribes multi-lingual audio into structured text. This application processes audio inputs, converts speech to text, and generates concise summaries optimized for lectures, meetings, and personal notes.

## Features
* **Multi-Lingual Support:** Processes voice input in any language.
* **Speech-to-Text:** High-accuracy audio transcription.
* **Smart Summarization:** Extracts key takeaways and action items into clean bullet points.

## Prerequisites
* C++17 compliant compiler (GCC, Clang, or MSVC)
* CMake (version 3.15 or higher)

## Quick Start

1. Clone the repository:
   ```bash
   git clone [https://github.com/KritamKhatiwada/notedio]
   cd notedio
   ```
   
2. Build the project:
    ```bash     
    mkdir build && cd build
    cmake ..
    cmake --build .
    ```
    
3. Run the application:     
    ```bash
    ./notedio
    ```