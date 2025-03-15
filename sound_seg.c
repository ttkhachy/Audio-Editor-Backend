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
}

// Load a WAV file into buffer
void wav_load(const char *filename, int16_t *dest)
{
    return;
    // i think the header is 44 bytes long
    // need to read the file from filename -> skip the first 44 bytes and then copy the remaining data
    // (which should be the raw audio data into dest)
    long file_size;
    get_file_size(filename, &file_size);
    size_t num_samples = (file_size - 44) / sizeof(int16_t);

    FILE *fptr;

    fopen(filename, "rb"); // assuming IO operations are always successful.

    fread(dest, sizeof(int16_t), num_samples, fptr);
    fclose(fptr);
}

// Create/write a WAV file from buffer
void wav_save(const char *fname, int16_t *src, size_t len)
{
    // fname maybe already exist or need to be created
    // src contains the audio sample
    // need to create the necessary header also (44 bytes)
    // the song will always be PCM, 16 bits per sample, mono, 8000Hz sample rate.?  -useful for the header
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
