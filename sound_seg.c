#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct sound_seg
{
    // int16_t* data;      // Pointer to raw PCM audio samples
    // size_t length;      // Number of samples
    // size_t start_pos;   // Starting position in the shared buffer (for inserts)
    // struct sound_seg* parent;  // Pointer to parent segment (for shared backing)
    // int ref_count;      // Reference count for shared memory tracking
    //  represents one audio track
};

int get_file_size(const char *filename, long *size)
{
    if (filename == NULL || size == NULL)
    {
        return -1; // Invalid arguments
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        return -1; // idk if i need to account for this? i think its assumed that it will be ok?
    }

    fseek(file, 0, SEEK_END); // Move to end
    *size = ftell(file);      // Get file position (size)

    fclose(file); // Close file
    return 0;
}

// Load a WAV file into buffer
void wav_load(const char *filename, int16_t *dest)
{
    // i think the header is 44 bytes long
    // need to read the file from filename -> skip the first 44 bytes and then copy the remaining data
    // (which should be the raw audio data into dest)
    long file_size;
    get_file_size(filename, &file_size);
    size_t num_samples = (file_size - 44) / sizeof(int16_t);

    FILE *fptr = fopen(filename, "rb"); // assuming IO operations are always successful.
    fseek(fptr, 44, SEEK_SET);

    fread(dest, sizeof(int16_t), num_samples, fptr);
    fclose(fptr);
    return;
}

// Create/write a WAV file from buffer
void wav_save(const char *fname, int16_t *src, size_t len)
{
    // fname maybe already exist or need to be created
    // src contains the audio sample
    // need to create the necessary header also (44 bytes) - hw do i do this?
    // the song will always be PCM, 16 bits per sample, mono, 8000Hz sample rate.?  -useful for the header

    FILE *fptr = fopen(fname, "wb");

    // WAV Header values
    uint32_t chunk_size = 44 + (len * sizeof(int16_t)) - 8;
    uint32_t chunk2_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t num_channels = 1; // mono
    uint32_t sample_rate = 8000;
    uint16_t bits_per_sample = 16;
    uint16_t block_align = num_channels * (bits_per_sample / 8);
    uint32_t byte_rate = sample_rate * block_align;
    uint32_t chunk3_size = len * sizeof(int16_t);

    fwrite("RIFF", 4, 1, fptr);           // ckID
    fwrite(&chunk_size, 4, 1, fptr);      // cksize
    fwrite("WAVE", 4, 1, fptr);           // WAVEID
    fwrite("fmt ", 4, 1, fptr);           // ckID
    fwrite(&chunk2_size, 4, 1, fptr);     // cksize
    fwrite(&audio_format, 2, 1, fptr);    // wFormatTag
    fwrite(&num_channels, 2, 1, fptr);    // nChannels
    fwrite(&sample_rate, 4, 1, fptr);     // nSamplesPerSec
    fwrite(&byte_rate, 4, 1, fptr);       // nAvgBytesPerSec
    fwrite(&block_align, 2, 1, fptr);     // nBlockAlign
    fwrite(&bits_per_sample, 2, 1, fptr); // wBitsPerSample
    fwrite("data", 4, 1, fptr);           // ckID
    fwrite(&chunk3_size, 4, 1, fptr);     // cksize

    // write the actual data contained in source
    fwrite(src, sizeof(int16_t), len, fptr);

    fclose(fptr);

    return;
}

// Initialize a new sound_seg object
struct sound_seg *tr_init()
{
    return NULL;
}

// Destroy a sound_seg object and free all allocated memory
void tr_destroy(struct sound_seg *obj)
{
    return;
}

// Return the length of the segment
size_t tr_length(struct sound_seg *seg)
{
    return (size_t)-1;
}

// Read len elements from position pos into dest
void tr_read(struct sound_seg *track, int16_t *dest, size_t pos, size_t len)
{
    return;
}

// Write len elements from src into position pos
void tr_write(struct sound_seg *track, int16_t *src, size_t pos, size_t len)
{
    return;
}

// Delete a range of elements from the track
bool tr_delete_range(struct sound_seg *track, size_t pos, size_t len)
{
    return true;
}

// Returns a string containing <start>,<end> ad pairs in target
char *tr_identify(struct sound_seg *target, struct sound_seg *ad)
{
    return NULL;
}

// Insert a portion of src_track into dest_track at position destpos
void tr_insert(struct sound_seg *src_track,
               struct sound_seg *dest_track,
               size_t destpos, size_t srcpos, size_t len)
{
    return;
}
