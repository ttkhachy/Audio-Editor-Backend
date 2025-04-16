# Audio Segment Processor Backend

This project provides a lightweight C library to manipulate and analyze raw PCM audio data. It supports core functionalities like segment insertion, deletion, reading/writing WAV files, and identifying matching audio regions using correlation.

## Features
- Load and save 16-bit mono WAV files (8000 Hz)
- Create and manage audio segments with linked insertions
- Efficient read/write with logical tracking
- Segment deletion and memory cleanup
- Identify audio patterns using cross-correlation
- Custom memory-managed audio track manipulation

## Skills Developed
- Deepened understanding of memory management, pointers, and dynamic allocation in C.
- Data Structures: Designed and implemented custom linked lists to manage logical audio insertions.
- File I/O: Gained experience with low-level binary file operations, specifically for WAV audio format parsing and writing.
- Audio Processing Fundamentals: Applied digital signal processing concepts such as dot product correlation to identify matching audio segments.
