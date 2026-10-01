
# Notedio

**Notedio** is a desktop app that turns your voice into organized, editable notes, entirely on your own machine. Speak, and your words are transcribed while you're still talking. Tap a button, and any note is condensed into a clean summary. No cloud, no accounts, no data leaving your computer.

The name says it all: *note* + *audio*. Notedio helps you capture ideas the moment they come, without stopping to type.

## Build and Run

**Prerequisites**

* Qt 6 or above with the Widgets and Multimedia modules, from https://www.qt.io/download-qt-installer
* A C++17 compiler
* CMake 3.16 or above
* [`whisper.cpp`](https://github.com/ggerganov/whisper.cpp) with `whisper-cli` compiled, plus a `.bin` model of your choice
* [`llama.cpp`](https://github.com/ggerganov/llama.cpp) with `llama-cli` compiled, plus a `.gguf` model of your choice

**Setting up whisper.cpp and llama.cpp**

Notedio does not bundle the speech and language models. You need a compiled `whisper.cpp` folder and a compiled `llama.cpp` folder, and you can get them in one of two ways:

1. **Use an existing compiled folder.** If you already have `whisper.cpp` and `llama.cpp` built (with `whisper-cli`, `llama-cli` and a model for each), place them inside the project, at `notedio/whisper.cpp` and `notedio/llama.cpp`, or point to them from `config.json`.
2. **Build them yourself.** Clone each repository into the project folder, compile it following its own instructions, and download the model you want to use. A bigger model is more accurate but slower, so pick one that suits your hardware.

Either way, make sure the executable and model paths in `config.json` match what you have.

**Build**

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

`config.json` is copied next to the built executable. Open it and point `whisperExecutable`, `llamaExecutable` and the model paths to your own binaries and models.

**Run**

```bash
./notedio
```

Or open `CMakeLists.txt` in Qt Creator and click Run.

## Overview

### Recording
Click **+ New Recording**, press **RECORD**, and talk. Pause and resume whenever you like, and watch the live transcript build up as you speak. Press **STOP** and your note appears at the top of the list, ready to use.

### Live Transcription
Audio is recorded in rolling chunks (20 seconds by default). Each chunk is sent to `whisper.cpp` in the background the moment it finishes, so by the time you press STOP only the last few seconds are left to process. The wait is minimal, even for long recordings.

### Rich Text Editing
Every note is fully editable. Fix a mis-heard word, or format your note with bold, italic, underline, text color and highlights.

### On-Demand Summaries
Click **Summarize** under any note and a local `llama.cpp` model writes a short summary right beneath it. The prompt template is yours to change in `config.json`.

## Configuration

Nothing is hardcoded. Everything lives in `config.json`, so you can swap models or executables without rebuilding:

| Setting | What it controls |
|---|---|
| `whisperExecutable`, whisper model | Transcription binary and model |
| `llamaExecutable`, llama model | Summarization binary and model |
| `chunkDurationSeconds` | Length of each audio chunk |
| thread counts | CPU threads for whisper and llama |
| summarization prompt | Template used when you click Summarize |

## Under the Hood

```
notedio/
├── CMakeLists.txt
├── config.json
├── main.cpp
├── core/
│   ├── AppConfig               # loads config.json, resolves paths
│   ├── Note                    # note model + JSON (de)serialization
│   ├── NoteStore               # one JSON file per note
│   ├── AudioChunkRecorder      # rolling chunk recording (QMediaRecorder)
│   ├── TranscriptionEngine     # serial background whisper-cli queue
│   └── SummarizationEngine     # background llama-cli queue
├── ui/
│   ├── MainWindow              # note list + New Recording
│   ├── RecordingWindow         # controls + live transcript
│   ├── NoteListItemWidget      # editor + Summarize + summary
│   └── NoteEditorWidget        # rich text editor
├── voiceInput/                 # raw audio chunks (runtime)
└── database/notes/             # saved notes
```

Chunks are transcribed one at a time, reassembled in order, and saved as a `Note` through `NoteStore`. Recorder state, queueing, process results and persistence are all logged to the terminal via `qDebug()`, which makes debugging easy.
