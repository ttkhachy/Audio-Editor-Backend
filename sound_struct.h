#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
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