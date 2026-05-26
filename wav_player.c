#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <string.h>
#include <ctype.h>
#include "data_structure.h"
#include "decor.h"
#include "helper_functions.h"

// to read WAV FILE
__chunks essential_chunks;
int16_t* read_wav_data(const char* filename, wav_header_t** header) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("[%sError%s] : File cannot open or not exist '%s'\n", FG_RED, RESET, filename);
        return NULL;
        }

    *header = (wav_header_t*)calloc(1, sizeof(wav_header_t));
    if (!*header) {
        puts("[Error] |Calloc| : Memory Allocation Failed");
        return NULL;
        }

    // Read RIFF chunk
    if (fread(&((*header)->riff), 1, sizeof(riff_header_t), file) < sizeof(riff_header_t)) {
        puts("[Error] : Failed to read RIFF header\n");
        free(header);
        fclose(file);
        return NULL;
        }

    if (strncmp((char*)(*header)->riff.riff, "RIFF", 4) != 0 || strncmp((char*)(*header)->riff.wave, "WAVE", 4) != 0) {
        puts("[Error] : Not a valid WAV file\n");
        free(*header);
        fclose(file);
        return NULL;
        }

    // Iterate through chunks
    chunk_header_t chunk;
    int fmt_found = 0;
    int data_found = 0;
    // loop to skip unwanted chunks
    while (fread(&chunk, 1, sizeof(chunk_header_t), file) == sizeof(chunk_header_t)) {

        // check  for format chunk (fmt)
        if (strncmp((char*)chunk.chunkid, "fmt ", 4) == 0) {
            uint32_t size_to_read = (chunk.chunk_size < sizeof(fmt_chunk_t)) ? chunk.chunk_size : sizeof(fmt_chunk_t);
            if (fread(&(*header)->fmt, 1, size_to_read, file) < size_to_read) {
                puts("[Error] : Failed to read fmt chunk\n");
                break;
                }
            // skip any extra bytes in fmt chunk 
            if (chunk.chunk_size > sizeof(fmt_chunk_t)) {
                fseek(file, chunk.chunk_size - sizeof(fmt_chunk_t), SEEK_CUR);
                }

            fmt_found = 1;
            memcpy(essential_chunks.fmt_chunk_id, "fmt ", 4);
            essential_chunks.fmt_chunk_size = size_to_read;
            }
        else if (strncmp((char*)chunk.chunkid, "data", 4) == 0) {
            (*header)->data_size = chunk.chunk_size;
            (*header)->data_offset = ftell(file);
            data_found = 1;
            memcpy(essential_chunks.data_chunk_id, "data", 4);
            break;
            }

        else {
            fseek(file, chunk.chunk_size, SEEK_CUR);
            // means skip N bytes (chunk.chunk_size) from current courser pos
            }

        }
    if (!fmt_found || !data_found) {
        printf("[Error] : Missing critical chunks (fmt: %s, data: %s)\n", fmt_found ? "found" : "MISSING", data_found ? "found" : "MISSING");
        free(*header);
        return NULL;
        }

    // Cal. number of samples
    size_t total_sample = (*header)->data_size / (*header)->fmt.block_align;

    // Allocate memory for audio data
    /*  block algin will give the size of snapshort / size of the sample (including all channels)
        multiply total samples with block_align to get the no of bytes to read
        */
    (*header)->bytes_to_read = total_sample * (*header)->fmt.block_align;
    /*why used int16_t and why not caused errors?
        malloc(bytes_to_read) reserves a contiguous block of bytes.
        Casting it to int16_t* tells the compiler to treat this block as an array of individual 8-bit bytes,
        which is perfect for binary data or raw data manipulation. */
    int16_t* data = (int16_t*)malloc((*header)->bytes_to_read);
    if (!data) {
        printf("[%sError%s] : Cannot allocate the memory to audio data \n", FG_RED, RESET);
        fclose(file);
        return NULL;
        }

    // Read audio data
    fseek(file, (*header)->data_offset, SEEK_SET);
    size_t bytes_read = fread(data, 1, (*header)->bytes_to_read, file);
    if (bytes_read < (*header)->bytes_to_read) {
        puts("[Error] | Data Reading | Failed to Read Payload\n");
        return NULL;
        }

    fclose(file);

    return data;

    }

// Audio call back function 
static int callback_function(const void* input, void* output, unsigned long framecount,
    const PaStreamCallbackTimeInfo* timeinfo, PaStreamCallbackFlags statusflag, void* userData)
    {
    playbackdata* data = (playbackdata*)userData;
    float* out = (float*)output;
    for (unsigned long i = 0; i < framecount * data->channels;i++) {
        if (data->current_sample < data->total_sample) {
            out[i] = data->normalized_data[data->current_sample++];

            }
        else {
            out[i] = 0.0f;
            }
        }
    return paContinue;

    }

int main(int argv, char* argc[]) {
    if (argv > 2) {
        puts("Usage: ./wav_player < option [D/d : Flag for Dubug] >");
        return 1;
        }

    if ((argv == 2) && strcmp(argc[1], "d") == 0) {
        debug = true;
        }

    printf("Enter the file path : \t");
    char* input = read_input(debug);
    if (debug) printf("path : %s\n", input);

    // Read headers
    wav_header_t* header = NULL;
    int16_t* data = read_wav_data(input, &header);
    if (!data) { return 1; }
    playbackdata* play_data = (playbackdata*)malloc(sizeof(playbackdata));
    if (!play_data) {
        special_print(ERROR, "playbackdata", "Failed to allocate memory ");
        return 1;
        }
    play_data->total_sample = (header->bytes_to_read / (header->fmt.bits_per_sample / 8));
    play_data->normalized_data = normalization(data, play_data->total_sample, debug);
    play_data->current_sample = 0;
    free(data);
    print_header_info(header);

    // port audio initialization 
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        special_print(ERROR, "Port_Audio", "PortAudio initilization Failed!");
        return 1;
        }
    special_print(SUCCESS, "Port_Audio", "PortAudio initialized Successfully");

    // selecting output DEvice
    PaDeviceIndex device = Pa_GetDefaultOutputDevice();
    if (device == paNoDevice) {
        special_print(ERROR, "Port_audio", "No output Device Available");
        return 1;
        }
    device_info(device);

    // Configure Device
    PaStreamParameters outputpara = { 0 };
    outputpara.channelCount = header->fmt.audio_channels;
    outputpara.device = device;
    outputpara.hostApiSpecificStreamInfo = NULL;
    outputpara.sampleFormat = paFloat32;
    outputpara.suggestedLatency = Pa_GetDeviceInfo(device)->defaultHighOutputLatency;
    play_data->channels = outputpara.channelCount;
    // open stream 
    PaStream* stream = NULL;
    err = Pa_OpenStream(
        &stream,
        NULL,
        &outputpara,
        header->fmt.sample_rate,
        paFramesPerBufferUnspecified,
        paClipOff,
        callback_function,
        (void*)play_data
    );
    if (err != paNoError) {
        special_print(ERROR, "Port Audio", "Failed to open stream");
        return 1;
        }
    float duration = header->bytes_to_read / header->fmt.byte_rate;
    special_print(INFO_PRINT, "\tRUN TIME INFORMATION", "\t\t\t\t==== Audio Player ====");
    printf("Duration : %0.2fs\n", duration);
    special_print(INFO_PRINT, "Playing Audio", "......");
    // start stream 
    Pa_StartStream(stream);
    Pa_Sleep(duration*1000 + 30);
    // stop && clse Stream 
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    // free heap memory and terminating the portaudio
    free(header);
    Pa_Terminate();
    ll_free(head_input, debug);

    return 0;
    }

