#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <string.h>
#include <ctype.h>
#include "data_structure.h"
#include "decor.h"
#include "helper_functions.h"
#include <ncurses.h>
#include <locale.h>

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
        free(*header);
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
        if (data->is_playing && data->current_sample < data->total_sample) {
            out[i] = data->normalized_data[data->current_sample++] * data->volume;

            }
        else {
            // Optimal way to output pure silence
            memset(output, 0, framecount * data->channels * sizeof(float));
            }
        }
    return paContinue;

    }

int main(int argv, char* argc[]) {
    setlocale(LC_ALL, "");
    if (argv > 2) {
        puts("Usage: ./wav_player < option [D/d : Flag for Dubug] >");
        return 1;
        }

    if ((argv == 2) && strcmp(argc[1], "d") == 0) {
        debug = true;
        }

    printf("Enter the file path : \t");
    char* input = read_input(debug);

    // Read headers
    wav_header_t* header = NULL;
    int16_t* data = read_wav_data(input, &header);
    if (!data) { return 1; }
    // initalize playback data 
    playbackdata* play_data = (playbackdata*)malloc(sizeof(playbackdata));
    if (!play_data) {
        special_print(ERROR, "playbackdata", "Failed to allocate memory ");
        return 1;
        }
    play_data->is_playing = true;
    play_data->volume = 0.75f;

    ll_node_insert(&head_input, play_data, debug);
    play_data->total_sample = (header->bytes_to_read / (header->fmt.bits_per_sample / 8));
    play_data->normalized_data = normalization(data, play_data->total_sample, debug);
    if (!play_data->normalized_data) {
        special_print(ERROR, "Normalization", "Failed to Normalise Data");
        return 1;
        }
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

    // selecting output DEvice dynamically
    PaDeviceIndex device = select_output_device(debug);
    if (device == paNoDevice) {
        special_print(ERROR, "Port_audio", "No valid output device selected or available.");
        return 1;
        }

    // Print the detailed table for the chosen device
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

    // ==========================================
    //          1. INITIALIZE NCURSES
    // ==========================================
    initscr();              // Start ncurses mode
    cbreak();               // Disable line buffering (react instantly)
    noecho();               // Don't print typed keys to the screen
    nodelay(stdscr, TRUE);  // Make getch() non-blocking
    curs_set(0);            // Hide the terminal cursor
    start_color();
    use_default_colors();
    init_pair(1, COLOR_GREEN, -1);
    bool engine_running = true;

    // 2. THE MAIN ENGINE LOOP
    // ==========================================
    while (engine_running && play_data->current_sample < play_data->total_sample) {

        // --- Input Handling ---
        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            engine_running = false;
            }
        else if (ch == 'p' || ch == 'P') {
            play_data->is_playing = !play_data->is_playing;
            }
        else if (ch == '+' || ch == '=') {
            // REASONING: Cap max volume at 1.0f to prevent digital clipping (distortion)
            if (play_data->volume < 1.0f) play_data->volume += 0.05f;
            }
        else if (ch == '-') {
            // REASONING: Cap min volume at 0.0f to prevent phase inversion (negative volume)
            if (play_data->volume > 0.0f) play_data->volume -= 0.05f;
            }

        // --- Dynamic Centering Math ---
        int max_y, max_x;
        getmaxyx(stdscr, max_y, max_x); // Fetches current terminal dimensions

        int box_width = 45;
        int box_height = 9;

        // Calculate top-left corner coordinates to perfectly center the box
        int start_y = (max_y - box_height) / 2;
        int start_x = (max_x - box_width) / 2;

        // --- UI Rendering ---
        erase(); // Use erase() instead of clear() to prevent screen flickering

        float current_sec = (float)play_data->current_sample / (header->fmt.sample_rate * header->fmt.audio_channels);
        int vol_percent = (int)(play_data->volume * 100);

        // Use the dynamic offsets (start_y, start_x) for all drawing
        mvprintw(start_y + 0, start_x, "┌───────────────────────────────────────────┐");
        mvprintw(start_y + 1, start_x, "│          WAV Audio Engine (TUI)           │");
        mvprintw(start_y + 2, start_x, "├───────────────────────────────────────────┤");

        if (play_data->is_playing) {
            mvprintw(start_y + 3, start_x, "│  Status   : [ ▶ PLAYING ]                 │");
            }
        else {
            mvprintw(start_y + 3, start_x, "│  Status   : [ ⏸ PAUSED  ]                 │");
            }

        mvprintw(start_y + 4, start_x, "│  Volume   : %3d%%                          │", vol_percent);
        mvprintw(start_y + 5, start_x, "│  Time     : %6.2fs / %5.2fs             │", current_sec, duration);
        mvprintw(start_y + 6, start_x, "│  Controls : [P] Play/Pause | [Q] Quit     │");
        mvprintw(start_y + 7, start_x, "│             [+/-] Volume                  │");
        mvprintw(start_y + 8, start_x, "└───────────────────────────────────────────┘");

        // Render a visual progress bar (Centered just below the box)
        int bar_width = 30;
        int progress_chars = (int)((current_sec / duration) * bar_width);
        mvprintw(start_y + 10, start_x + 2, "Progress: [");
        for (int i = 0; i < bar_width; i++) {
                if (i < progress_chars) {
                    attron(COLOR_PAIR(1));  // Turn Green Background ON
                    printw("#");            // Print a space (it will be solid green)
                    attroff(COLOR_PAIR(1)); // Turn Green Background OFF
                    }
                else {
                    printw("-");
                    }
            }
        
        printw("]");

        refresh();

        // --- Prevent CPU Overload ---
        Pa_Sleep(30);
        }
    // ==========================================
    // 3. CLEANUP NCURSES 
    // ==========================================
    curs_set(1); //  Bring the blinking terminal cursor back
    endwin();    //  Shut down ncurses and restore normal terminal behavior

    // stop && clse Stream 
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    // free heap memory and terminating the portaudio
    free(header);

    Pa_Terminate();
    ll_free(head_input, debug);

    return 0;
    }

