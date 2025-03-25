#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct sound_seg
{
    int16_t *data;            // Pointer to raw PCM audio samples
    size_t length;            // Number of samples
    size_t start_pos;         // Starting position in the shared buffer (for inserts)
    struct sound_seg *parent; // Pointer to parent segment (for shared backing)
    int ref_count;            // Reference count for shared memory tracking
    size_t capacity;          // total allocated samples
};

int get_file_size(const char *filename, long *size)
{
    if (filename == NULL || size == NULL)
    {
        return -1; // invalid arguments - although i think i assume its always correct?
    }

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        return -1; // idk if i need to account for this? i think its assumed that it will be ok?
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file); // get file position (size)

    fclose(file); // cant forget the close
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
    struct sound_seg *track = malloc(sizeof(struct sound_seg));
    if (track == NULL)
    {
        return NULL;
    }

    track->data = NULL; // no buffer yet, allocate in tr_write()
    track->length = 0;  // no samples yet
    track->start_pos = 0;
    track->ref_count = 1; // default reference count
    track->capacity = 0;  // capacity will be determined on the first write call
    track->parent = NULL;

    return track;
}

// Destroy a sound_seg object and free all allocated memory
void tr_destroy(struct sound_seg *obj)
{
    if (obj == NULL)
    {
        return;
    }

    if (obj->ref_count == 1)
    {
        if (obj->data != NULL)
        {
            free(obj->data);
            obj->data = NULL;
        }
    }
    else
    {
        // dont want to free the data yet since it is being used by an
        obj->ref_count--;
    }
    free(obj);
    obj == NULL;
}

// Return the length of the segment
size_t tr_length(struct sound_seg *seg)
{

    if (seg == NULL)
    {
        return 0; // edge case
    }
    return seg->length;
}

// Read len elements from position pos into dest
void tr_read(struct sound_seg *track, int16_t *dest, size_t pos, size_t len)
{

    if (track->data == NULL)
    {
        return;
    }

    if (pos + len > track->length)
    {
        return;
    }

    for (size_t i = 0; i < len; i++)
    {
        *(dest + i) = (track->data)[i + pos];
    }
    return;
}

// Write len elements from src into position pos
void tr_write(struct sound_seg *track, int16_t *src, size_t pos, size_t len)
{
    size_t total_new_length = pos + len;
    if (track->data == NULL)
    {
        // need to initialise struct values and malloc space for the samples
        track->capacity = total_new_length;
        track->data = malloc(track->capacity * sizeof(int16_t));
        if (track->data == NULL)
        {
            // i.e an error occures
            return;
        }
    }
    else if (total_new_length > track->capacity)
    {
        size_t new_capacity = track->capacity;
        // increase the capacity by doubling it until it is sufficenet to handle to whole new length
        // see if this is in line with what is in the lectures?
        while (new_capacity < total_new_length)
        {
            new_capacity *= 2;
        }

        int16_t *temp = realloc(track->data, new_capacity * sizeof(int16_t));
        if (temp == NULL)
        {
            return;
        }
        track->data = temp;
        track->capacity = new_capacity;
    }
    for (size_t i = 0; i < len; i++)
    {
        track->data[i + pos] = *(src + i); // this part should be all? the main difficulty is the memory.
    }
    if (total_new_length > track->length)
    {
        track->length = total_new_length;
    }
    return;
}

// Delete a range of elements from the track
bool tr_delete_range(struct sound_seg *track, size_t pos, size_t len)
{

    if (track == NULL)
    {
        return false;
    }
    if (pos + len > track->length)
    {
        return false;
    }

    // when would the function return false?

    memmove(track->data + pos, track->data + pos + len, (track->length - pos - len) * sizeof(int16_t));

    for (int i = pos + len; i < track->length; i++)
    {
        track->data[i] = 0; // might not be necessary as we are adjusting the logical length
    }

    // TODO - check if i must delete if the track is shared across multiple sounds?

    // can i just replace the pos - pos + len with the portion following pos+ len?
    // then set the remaining bits as null? idk do i reduce the allocated space or just leave it as empty?
    // need to let it marinate a bit and see what the best option is

    track->length -= len;
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

    // track->length += len;

    return;
}

int main()
{
    int i = 0;
}
// set obj and obj->data as NULL after free in the destroy function
// track->parent = NULL;