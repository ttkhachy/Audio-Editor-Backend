// generated code to test c binding behaviour

#include <stdint.h>  
#include <stddef.h>  
#include <stdlib.h>  
#include <stdbool.h>  
#include <string.h>  
  
struct sound_seg {  
    int16_t **refcounts;                 // Dynamic array of pointers to 16-bit ints  
    int16_t **parents;                   // Dynamic array of pointers to int16_t* (parent refcounts)  
    int16_t **data;                      // Dynamic array of pointers to 16-bit ints  
    int16_t **underlying_data;           // Dynamic array of pointers to arrays of 16-bit ints  
    int16_t **underlying_refcounts;      // Dynamic array of pointers to arrays of 16-bit ints  
    size_t size;                         // Length of refcounts, parents, and data arrays  
    size_t underlying_size;              // Length of underlying_data array  
    size_t underlying_refcounts_size;    // Length of underlying_refcounts array  
};  
  
// Initialize a new sound_seg object  
struct sound_seg* init() {  
    struct sound_seg* seg = malloc(sizeof(struct sound_seg));  
    seg->refcounts = NULL;  
    seg->parents = NULL;  
    seg->data = NULL;  
    seg->underlying_data = NULL;  
    seg->underlying_refcounts = NULL;  
    seg->size = 0;  
    seg->underlying_size = 0;  
    seg->underlying_refcounts_size = 0;  
    return seg;  
}  
  
// Destroy a sound_seg object and free all allocated memory  
void destroy(struct sound_seg* obj) {  
    // Free each element of underlying_data  
    for (size_t i = 0; i < obj->underlying_size; ++i) {  
        free(obj->underlying_data[i]);  
    }  
    // Free each element of underlying_refcounts  
    for (size_t i = 0; i < obj->underlying_refcounts_size; ++i) {  
        free(obj->underlying_refcounts[i]);  
    }  
    // Free the arrays  
    free(obj->underlying_data);  
    free(obj->underlying_refcounts);  
    free(obj->data);  
    free(obj->refcounts);  
    free(obj->parents);  
    // Free the struct itself  
    free(obj);  
}  
  
// Read len elements from position pos into dest  
void read(struct sound_seg* seg, int16_t* dest, size_t pos, size_t len) {  
    // Assume pos and len are within bounds  
    for (size_t i = 0; i < len; ++i) {  
        dest[i] = *(seg->data[pos + i]);  
    }  
}  
  
// Write len elements from src into position pos  
void write(struct sound_seg* seg, int16_t* src, size_t pos, size_t len) {  
    size_t end_pos = pos + len;  
    if (end_pos <= seg->size) {  
        // In-range write  
        for (size_t i = 0; i < len; ++i) {  
            *(seg->data[pos + i]) = src[i];  
        }  
    } else {  
        // Calculate in-range and out-of-range lengths  
        size_t in_range_len = (pos < seg->size) ? seg->size - pos : 0;  
        size_t out_of_range_len = len - in_range_len;  
  
        // Write in-range elements  
        for (size_t i = 0; i < in_range_len; ++i) {  
            *(seg->data[pos + i]) = src[i];  
        }  
  
        // Handle out-of-range elements  
        if (out_of_range_len > 0) {  
            // Step 1: Create new arrays for data and refcounts  
            int16_t* new_data_array = malloc(out_of_range_len * sizeof(int16_t));  
            int16_t* new_refcounts_array = malloc(out_of_range_len * sizeof(int16_t));  
  
            // Copy data and initialize refcounts to 0  
            for (size_t i = 0; i < out_of_range_len; ++i) {  
                new_data_array[i] = src[in_range_len + i];  
                new_refcounts_array[i] = 0;  
            }  
  
            // Step 2: Append new arrays to underlying_data and underlying_refcounts  
            seg->underlying_data = realloc(seg->underlying_data, (seg->underlying_size + 1) * sizeof(int16_t*));  
            seg->underlying_data[seg->underlying_size] = new_data_array;  
            seg->underlying_size++;  
  
            seg->underlying_refcounts = realloc(seg->underlying_refcounts, (seg->underlying_refcounts_size + 1) * sizeof(int16_t*));  
            seg->underlying_refcounts[seg->underlying_refcounts_size] = new_refcounts_array;  
            seg->underlying_refcounts_size++;  
  
            // Step 3: Extend data, refcounts, and parents arrays  
            size_t new_size = end_pos;  
            seg->data = realloc(seg->data, new_size * sizeof(int16_t*));  
            seg->refcounts = realloc(seg->refcounts, new_size * sizeof(int16_t*));  
            seg->parents = realloc(seg->parents, new_size * sizeof(int16_t*));  
  
            // Initialize any gaps between current size and pos  
            for (size_t i = seg->size; i < pos; ++i) {  
                seg->data[i] = NULL;  
                seg->refcounts[i] = NULL;  
                seg->parents[i] = NULL;  
            }  
  
            // Step 4: Populate data, refcounts, and parents with new elements  
            for (size_t i = 0; i < out_of_range_len; ++i) {  
                seg->data[pos + in_range_len + i] = &new_data_array[i];  
                seg->refcounts[pos + in_range_len + i] = &new_refcounts_array[i];  
                seg->parents[pos + in_range_len + i] = NULL;  
            }  
  
            // Update the size  
            seg->size = new_size;  
        }  
    }  
}  
  
// Delete a range of elements from the segment  
bool delete_range(struct sound_seg* seg, size_t pos, size_t len) {  
    // Assume pos and len are within bounds  
  
    // Step 1: Check refcounts in the specified range  
    for (size_t i = pos; i < pos + len; ++i) {  
        if (*(seg->refcounts[i]) != 0) {  
            // Cannot delete if any refcount is not zero  
            return false;  
        }  
    }  
  
    // Step 2: Decrease the corresponding refcounts via seg->parents by 1  
    for (size_t i = pos; i < pos + len; ++i) {  
        if (seg->parents[i] != NULL) {  
            (*(seg->parents[i]))--;  
        }  
    }  
  
    size_t new_size = seg->size - len;  
  
    // Shift elements after the deleted range  
    if (pos + len < seg->size) {  
        size_t move_count = seg->size - (pos + len);  
        size_t move_size = move_count * sizeof(int16_t*);  
  
        // Shift data pointers  
        memmove(&seg->data[pos], &seg->data[pos + len], move_size);  
        // Shift refcounts pointers  
        memmove(&seg->refcounts[pos], &seg->refcounts[pos + len], move_size);  
        // Shift parents pointers  
        memmove(&seg->parents[pos], &seg->parents[pos + len], move_size);  
    }  
  
    // Resize the arrays to the new size  
    seg->data = realloc(seg->data, new_size * sizeof(int16_t*));  
    seg->refcounts = realloc(seg->refcounts, new_size * sizeof(int16_t*));  
    seg->parents = realloc(seg->parents, new_size * sizeof(int16_t*));  
  
    // Update the size  
    seg->size = new_size;  
  
    return true;  
}  
  
// Return the length of the segment  
size_t length(struct sound_seg* seg) {  
    return seg->size;  
}  
  
// Insert a portion of src_obj into dest_obj at position destpos  
void insert(struct sound_seg* src_obj, struct sound_seg* dest_obj, size_t destpos, size_t srcpos, size_t len) {  
    // Assume srcpos and len are within src_obj's bounds  
    // Assume destpos is between 0 and dest_obj->size inclusive  
  
    // Step 1: Make temporary arrays and copy the data pointers and parent pointers  
    int16_t** temp_data = malloc(len * sizeof(int16_t*));  
    int16_t** temp_parents = malloc(len * sizeof(int16_t*));  
  
    for (size_t i = 0; i < len; ++i) {  
        temp_data[i] = src_obj->data[srcpos + i];  
        temp_parents[i] = src_obj->refcounts[srcpos + i];  
    }  
  
    // Step 1b: Create a new refcounts array initialized to 0  
    int16_t* new_refcounts_array = malloc(len * sizeof(int16_t));  
    for (size_t i = 0; i < len; ++i) {  
        new_refcounts_array[i] = 0;  
    }  
    // Append the new refcounts array to dest_obj's underlying_refcounts  
    dest_obj->underlying_refcounts = realloc(dest_obj->underlying_refcounts, (dest_obj->underlying_refcounts_size + 1) * sizeof(int16_t*));  
    dest_obj->underlying_refcounts[dest_obj->underlying_refcounts_size] = new_refcounts_array;  
    dest_obj->underlying_refcounts_size++;  
  
    // Step 2: Extend dest_obj's arrays to accommodate new elements  
    size_t new_size = dest_obj->size + len;  
    dest_obj->data = realloc(dest_obj->data, new_size * sizeof(int16_t*));  
    dest_obj->refcounts = realloc(dest_obj->refcounts, new_size * sizeof(int16_t*));  
    dest_obj->parents = realloc(dest_obj->parents, new_size * sizeof(int16_t*));  
  
    // Step 3: Shift existing elements in dest_obj's arrays to make space for insertion  
    size_t move_count = dest_obj->size - destpos;  
    if (move_count > 0) {  
        size_t move_size = move_count * sizeof(int16_t*);  
  
        // Shift data pointers  
        memmove(&dest_obj->data[destpos + len], &dest_obj->data[destpos], move_size);  
        // Shift refcounts pointers  
        memmove(&dest_obj->refcounts[destpos + len], &dest_obj->refcounts[destpos], move_size);  
        // Shift parents pointers  
        memmove(&dest_obj->parents[destpos + len], &dest_obj->parents[destpos], move_size);  
    }  
  
    // Step 4: Insert into dest_obj's arrays  
    for (size_t i = 0; i < len; ++i) {  
        dest_obj->data[destpos + i] = temp_data[i];                      // Copy data pointer  
        dest_obj->refcounts[destpos + i] = &new_refcounts_array[i];      // Point to new refcounts initialized to 0  
        dest_obj->parents[destpos + i] = temp_parents[i];                // Point to src_obj's refcounts  
    }  
  
    // Step 5: Increase src_obj->refcounts by 1 for the inserted elements  
    for (size_t i = 0; i < len; ++i) {  
        (*(temp_parents[i]))++;  
    }  
  
    // Step 6: Free the temporary arrays  
    free(temp_data);  
    free(temp_parents);  
  
    // Update dest_obj's size  
    dest_obj->size = new_size;  
}  
