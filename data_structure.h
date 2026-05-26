#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H
#include <stdint.h>
#include <stdbool.h>

bool debug = false;

typedef struct {
    // RIFF 
    uint8_t riff[4];
    uint32_t file_size;
    uint8_t wave[4];
    }riff_header_t;

// used to identify chunks and skip them
typedef struct
    {
    uint8_t chunkid[4];
    uint32_t chunk_size;
    }chunk_header_t;


typedef struct {
    // FORMAT HEADERS
    uint16_t audio_format;
    uint16_t audio_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    }fmt_chunk_t;

// main structure to read wav header
typedef struct
    {
    riff_header_t riff;
    fmt_chunk_t fmt;
    uint32_t data_size;
    uint32_t bytes_to_read;
    uint32_t data_offset;
    }wav_header_t;

// essential chunks
typedef struct
    {
    char fmt_chunk_id[4];
    int fmt_chunk_size;
    char data_chunk_id[4];
    } __chunks;

typedef struct node
    {
    char* address;
    struct node* next;
    }node;

typedef struct {
    float* normalized_data;
    size_t total_sample;
    size_t current_sample;
    uint8_t channels;
    }playbackdata;

#endif