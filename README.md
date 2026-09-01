# Notedio

An AI-powered voice-note summarizer. Records audio in rolling chunks, transcribes each chunk in the background via `whisper.cpp` while recording continues, lets you edit notes with rich text formatting, and summarizes any note on demand via a local `llama.cpp` model.

## Architecture

```
notedio/
├── CMakeLists.txt
├── config.json                 # all paths/params live here, nothing hardcoded in source
├── main.cpp
├── core/
│   ├── AppConfig.{h,cpp}        # loads config.json, resolves paths relative to the executable
│   ├── Note.{h,cpp}             # note data model + JSON (de)serialization
│   ├── NoteStore.{h,cpp}        # persists notes as individual JSON files in database/notes
│   ├── AudioChunkRecorder.{h,cpp}   # records audio in rolling N-second chunks (QMediaRecorder)
│   ├── TranscriptionEngine.{h,cpp}  # serial background queue of whisper-cli QProcess calls
│   └── SummarizationEngine.{h,cpp}  # background queue of llama-cli QProcess calls
├── ui/
│   ├── MainWindow.{h,cpp}       # note list + "New Recording" button
│   ├── RecordingWindow.{h,cpp}  # record/pause controls + live transcript
│   ├── NoteListItemWidget.{h,cpp}   # per-note row: editor + Summarize button + summary
│   └── NoteEditorWidget.{h,cpp}     # rich text editor: bold/italic/underline/color/highlight
├── voiceInput/                  # raw audio chunks land here at runtime
└── database/notes/              # one JSON file per note
```

## How chunked transcription works

1. `AudioChunkRecorder` records into `chunk_0000.wav`, `chunk_0001.wav`, ... rotating to a new file every `chunkDurationSeconds` (default 20s, configurable in `config.json`).
2. Each time a chunk finishes, `chunkReady` fires and `TranscriptionEngine` enqueues it. A single background `QProcess` runs `whisper-cli` per chunk, one at a time, so by the time you hit STOP, all but the last ~20s are already transcribed.
3. Only the final (partial) chunk needs transcribing after STOP, so the wait is minimal.
4. Chunks are reassembled in order (`transcriptProgress` / `transcriptFinalized`) into the note's transcript, which is saved as a new `Note` via `NoteStore`.
5. `SummarizationEngine` runs `llama-cli` with a configurable prompt template whenever "Summarize" is clicked on a note.

All whisper/llama paths, thread counts, chunk duration, and the summarization prompt are defined in `config.json` next to the built executable — nothing is hardcoded in source, so swapping models or executables requires no rebuild.

Everything logs to the terminal via `qDebug()` — recorder state changes, chunk queueing, transcription/summarization process starts and results, and note persistence.

## Prerequisites

* Qt 6 (Widgets, Multimedia)
* C++17 compiler
* CMake ≥ 3.16
* A built `whisper.cpp` (`whisper-cli`) with a `.bin` model or build whisper-cli with a model of your need inside */whisper.cpp
* A built `llama.cpp` (`llama-cli`) with a `.gguf` model build llama-cli with a model of your need inside */llama.cpp

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

`config.json` is copied next to the built binary. Edit it (or point `whisperExecutable` / `llamaExecutable` / model paths to your actual binaries) before running.

## Run

```bash
./notedio
```

Click **+ New Recording** → RECORD → talk → STOP. The note appears at the top of the list, fully editable, with a Summarize button underneath.
