#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

struct sound_seg
{
    int16_t *data;       // Pointer to raw PCM audio samples (for parents)
    size_t total_length; // Number of samples

    size_t capacity; // total allocated samples

    struct node *head;        // a list of logical portions
    size_t start_pos;         // Starting position in the shared buffer (for inserts) - is this necessary?
    int ref_count;            // Number of parents the track has
    struct sound_seg *parent; // might delete this
    int child_count;
};

struct node
{
    struct sound_seg *segment; // backing data
    size_t offset;             // where to start reading from segment->data
    size_t length;             // how much to read
    size_t position_in_data;
    struct node *next; // next node
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

    track->data = NULL;      // no buffer yet, allocate in tr_write()
    track->total_length = 0; // no samples yet
    track->start_pos = 0;
    track->ref_count = 0; // default reference count - why is it one? shouldnt it be 0?
    track->capacity = 0;  // capacity will be determined on the first write call
    track->child_count = 0;
    track->parent = NULL;
    track->head = NULL;

    return track;
}

// Destroy a sound_seg object and free all allocated memory
void tr_destroy(struct sound_seg *obj)
{
    if (obj == NULL)
    {
        return;
    }

    struct node *curr = obj->head;
    while (curr != NULL)
    {
        curr->segment->ref_count--;
        curr->segment->child_count--;

        struct node *next = curr->next;
        free(curr);
        curr = next;
    }

    if (obj->data != NULL)
    {
        free(obj->data);
        obj->data = NULL;
        // tr_destroy will only be called at the end of the program
    }

    free(obj);
    obj = NULL;
}

// Return the length of the segment
size_t tr_length(struct sound_seg *seg)
{

    if (seg == NULL)
    {
        return 0;
    }
    return seg->total_length;
}

// Read len elements from position pos into dest
void tr_read(struct sound_seg *track, int16_t *dest, size_t pos, size_t len)
{
    //  i think there is somethin wrong with the way i am reading the index
    // like i am skipping past the data in the raw as well? need to confirm

    if (track->data == NULL)
    {
        return;
    }

    if (pos + len > track->total_length)
    {
        return;
    }

    size_t logical_index = 0; // index for final logical stream
    size_t dest_index = 0;    // index for dest buffer
    struct node *curr = track->head;

    while (dest_index < len)
    {
        // check if we are inside a node (insertion)
        if (curr != NULL &&
            logical_index >= curr->position_in_data &&
            logical_index < curr->position_in_data + curr->length)
        {
            size_t offset = logical_index - curr->position_in_data;
            dest[dest_index++] = curr->segment->data[curr->offset + offset];
            logical_index++;
        }
        else
        {
            // calculate raw index by subtracting length of all prior inserts
            size_t raw_index = logical_index;
            struct node *temp = track->head;
            while (temp != NULL)
            {
                if (temp->position_in_data < logical_index)
                {
                    raw_index -= temp->length;
                }
                temp = temp->next;
            }

            dest[dest_index++] = track->data[raw_index];
            logical_index++;

            // skip current node if we’ve passed it
            if (curr != NULL && logical_index >= curr->position_in_data + curr->length)
            {
                curr = curr->next;
            }
        }
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
    if (total_new_length > track->total_length)
    {
        track->total_length = total_new_length;
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
    if (track->child_count > 0)
    {
        return false;
    }
    if (pos + len > track->total_length)
    {
        return false;
    }

    memmove(track->data + pos, track->data + pos + len, (track->total_length - pos - len) * sizeof(int16_t));

    track->total_length -= len;
    return true;
}

// Calculates the dot product for a given sound_seg. Used to assist in calculating cross correlation
void get_dot_product(struct sound_seg *track1, struct sound_seg *track2, int64_t *correlation, size_t start1, size_t end1,
                     size_t start2, size_t end2)
{
    *correlation = 0;

    for (int i = 0; i < end1 - start1; i++)
    {
        *correlation += (track1->data[start1 + i] * track2->data[start2 + i]);
    }
}

// Returns the max of two given values. If equal the default behaviour is to return val1
void max(size_t *max, size_t val1, size_t val2)
{
    if (val1 >= val2)
    {
        *max = val1;
    }
    else
    {
        *max = val2;
    }
}

// Returns a string containing <start>,<end> ad pairs in target
char *tr_identify(struct sound_seg *target, struct sound_seg *ad)
{
    int64_t autocorrelation = 0;
    get_dot_product(ad, ad, &autocorrelation, 0, ad->total_length, 0, ad->total_length);

    size_t str_capacity = 50;
    char *ret_indices;
    ret_indices = (char *)malloc(str_capacity * sizeof(char));
    ret_indices[0] = '\0';
    size_t curr_string_length = 0;
    size_t i = 0;
    while (i <= (target->total_length - ad->total_length))
    {
        int64_t target_product = 0;
        get_dot_product(target, ad, &target_product, i, i + ad->total_length, 0, ad->total_length);
        double ratio = 100.0 * (double)target_product / (double)autocorrelation;
        if (ratio >= 95)
        {
            char matched_string[32];
            snprintf(matched_string, sizeof(matched_string), "%zu,%zu\n", i, i + ad->total_length - 1);
            size_t new_data_length = strlen(matched_string);
            if (curr_string_length + new_data_length + 1 > str_capacity)
            {

                size_t new_capacity = 0;
                max(&new_capacity, str_capacity * 2, curr_string_length + new_data_length + 1);

                char *temp = realloc(ret_indices, new_capacity * sizeof(char));
                if (temp == NULL)
                {
                    free(ret_indices);
                    return NULL;
                }
                ret_indices = temp;
                str_capacity = new_capacity;
            }

            strncpy(ret_indices + curr_string_length, matched_string, new_data_length);
            curr_string_length += new_data_length;
            ret_indices[curr_string_length] = '\0';
            i += ad->total_length;
        }
        else
        {
            i++;
        }
    }

    if (curr_string_length > 0)
    {
        // adding the NULL byte for a dynamically allocated string
        ret_indices[curr_string_length - 1] = '\0';
    }
    return ret_indices;
    // instead of returning the pointer to the string, could we pass it into the function as a param
    // and then modify it directly?
}

// Insert a portion of src_track into dest_track at position destpos
void tr_insert(struct sound_seg *src_track, struct sound_seg *dest_track, size_t destpos, size_t srcpos, size_t len)
{

    struct node *new_segment = malloc(sizeof(struct node));

    if (new_segment == NULL)
    {
        return;
    }
    // set new_segment values
    new_segment->segment = src_track; // the segment from which it reads from is the src -> its a pointer to the entire struct
    new_segment->offset = srcpos;
    new_segment->length = len;
    new_segment->next = NULL;
    new_segment->position_in_data = destpos;

    if (dest_track->head == NULL || destpos == 0)
    {
        // i.e this is the first insert operation
        new_segment->next = dest_track->head;
        dest_track->head = new_segment;
    }
    else
    {
        size_t position_count = 0;
        // there is already a preexisting sequence of nodes to consider - we first assume no overlap.
        struct node *curr = dest_track->head;
        struct node *prev = NULL;
        while (curr != NULL && (position_count + curr->length) <= destpos)
        {
            position_count += curr->length;
            prev = curr;
            curr = curr->next;
        }
        if (prev == NULL)
        {
            new_segment->next = dest_track->head;
            dest_track->head = new_segment;
        }
        else
        {
            prev->next = new_segment;
            new_segment->next = curr;
        }
    }

    src_track->ref_count++; // honestly think these 2 are the same thing...
    src_track->child_count++;

    dest_track->total_length += len;

    // probs other things i need to do in this function ?
    return;
}

// int main()
// {
//     struct sound_seg *base = tr_init();
//     int16_t base_data[] = {10, 11, 12, 13, 14, 15, 16, 17};
//     tr_write(base, base_data, 0, 8);

//     struct sound_seg *insert = tr_init();
//     int16_t insert_data[] = {99, 98};
//     tr_write(insert, insert_data, 0, 2);
//     tr_insert(insert, base, 2, 0, 2);

//     struct sound_seg *second = tr_init();
//     int16_t second_data[] = {70, 71, 72};
//     tr_write(second, second_data, 0, 3);
//     tr_insert(second, base, 6, 0, 3); // note: inserting into 'base'

//     int16_t result[13];
//     tr_read(base, result, 0, 13);

//     printf("Final logical track after inserts:\n");
//     for (size_t i = 0; i < 13; i++)
//     {
//         printf("%d ", result[i]);
//     }
//     printf("\n");

//     tr_destroy(base);
//     tr_destroy(second);
//     tr_destroy(insert);

//     return 0;
// }
