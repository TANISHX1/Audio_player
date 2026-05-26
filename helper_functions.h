#ifndef HELPER_FUNCTION_H
#define HELPER_FUNCTION_H

#include "data_structure.h"
#include "decor.h"

// special print MACROS
#define DEBUG 0
#define SUCCESS 1
#define ERROR 2
#define INFO_PRINT 3

// linked list head pointers 
node* head_input = NULL;

// function decleration 
void special_print(int type, char* category, char* msg);
char* ll_node_insert(node** head, char* address,bool debug);

// 1. Type-specific underlying functions
char* int_to_str(int val) {
    int len = snprintf(NULL, 0, "%d", val);
    char* str = (char*)malloc(len + 1);
    if (str) snprintf(str, len + 1, "%d", val);
    ll_node_insert(&head_input, str,debug);
    return str;
    }

char* size_to_str(size_t val) {
    int len = snprintf(NULL, 0, "%zu", val);
    char* str = (char*)malloc(len + 1);
    ll_node_insert(&head_input, str,debug);
    if (str) snprintf(str, len + 1, "%zu", val);
    return str;
    }

char* float_to_str(float val) {
    int len = snprintf(NULL, 0, "%f", val);
    char* str = (char*)malloc(len + 1);
    if (str) snprintf(str, len + 1, "%f", val);
    ll_node_insert(&head_input, str,debug);
    return str;
    }

//  The Generic Macro (Requires C11 standard)
#define num_to_str(X) _Generic((X), \
    int: int_to_str, \
    size_t: size_to_str, \
    float: float_to_str, \
    double: float_to_str \
)(X)



char* ll_node_insert(node** head, char* address,bool debug) {
    node* new_node = (node*)malloc(sizeof(node));
    if (!new_node) {
        puts(FG_BRED"LL_node insertion Failed"RESET);
        return NULL;
        }
    new_node->address = address;
    new_node->next = NULL;
    if (*head == NULL) {
        *head = new_node;
        return address;
        }
    node* pos = *head;
    while (pos->next != NULL) {
        pos = pos->next;
        }
    pos->next = new_node;
    if (debug) {
        pos = *head;
        while (pos != NULL) {
            special_print(DEBUG, "ll_node_insertion {Transversing the linkedlist }", pos->address);
            pos = pos->next;
            }
        }
    return address;
    }

void ll_free(node* head, bool debug) {
    if (head == NULL) {
        puts("["FG_RED "Errror"RESET" ] Empty linked_list");
        return;
        }

    while (head->next != NULL) {
        node* temp = head;
        head = head->next;
        free(temp->address);
        free(temp);
        }

    if (head->next == NULL) {
        free(head->address);
        free(head);
        }
    if (debug) {
        puts("[ "FG_CYAN"DEBUG "RESET"] Successfully freed input linked_list !..");
        }
    }

char* read_input(bool debug) {
    char buffer[128];
    
    fgets(buffer, 128, stdin);
    uint8_t size = strlen(buffer);
    if (size > 1 && buffer[size - 1] == '\n') {
        buffer[size - 1] = '\0';
        size--;
        }
    char* input = ll_node_insert(&head_input, (char*)strdup(buffer),debug);
    if(debug) printf("from read_line : %s\n", input);
    return input;
    }

float* normalization(int16_t* data, size_t samples, bool debug) {

    float* data_new = (float*)malloc(samples * sizeof(float));

    if (!data_new) {
        special_print(ERROR, "normilization", "Failed to Allocate Memory");
        return NULL;
        }
    if (debug) { special_print(DEBUG, "normalisation ", num_to_str(samples)); }
    for (size_t i = 0;i < samples;i++) {
        data_new[i] = data[i] / 32768.0f;
        }

    return data_new;
    }

void special_print(int type, char* category, char* msg) {
    switch (type)
        {
        case 0:
            printf("["BOLD FG_BCYAN" DEBUG "RESET"] {Related_to :%s} %s\n", category, msg);
            break;

        case 1:
            printf("["BOLD FG_BGREEN" SUCCESS "RESET"] {Related_to : %s } "BLINK" %s\n"RESET, category, msg);
            break;

        case 2:
            printf("[" BOLD FG_BRED" ERROR "RESET"] {Related_to : %s } %s\n", category, msg);
            break;
        case 3:
            printf(BOLD "\033[38;5;155m %s\n %s\n" RESET, category, msg);
            break;
        default:
            break;
        }
    }

void device_info(PaDeviceIndex device) {
    const PaDeviceInfo* info = Pa_GetDeviceInfo(device);
    printf(FG_BGREEN "\t\t┌───────────────────────────────────────────────────────────┐\n" RESET);
    printf(FG_BGREEN "\t\t│" RESET " Device [%d] | Device Name : %-30s " FG_BGREEN "│\n" RESET, device, info->name);
    printf(FG_BGREEN "\t\t├───────────────────────────────────────────────────────────┤\n" RESET);
    printf(FG_BGREEN "\t\t│" RESET "    Input:                                                 " FG_BGREEN "│\n" RESET);
    printf(FG_BGREEN "\t\t│" RESET "           Max Input channels:          %10d         " FG_BGREEN "│\n" RESET, info->maxInputChannels);
    printf(FG_BGREEN "\t\t│" RESET "           High Input latency [Default]: %10f        " FG_BGREEN "│\n" RESET, info->defaultHighInputLatency);
    printf(FG_BGREEN "\t\t│" RESET "           Low Input Latency  [Default]: %10f        " FG_BGREEN "│\n" RESET, info->defaultLowInputLatency);
    printf(FG_BGREEN "\t\t│" RESET "    Output:                                                " FG_BGREEN "│\n" RESET);
    printf(FG_BGREEN "\t\t│" RESET "           Max Output channels:         %10d         " FG_BGREEN "│\n" RESET, info->maxOutputChannels);
    printf(FG_BGREEN "\t\t│" RESET "           High Output latency [Default]: %10f       " FG_BGREEN "│\n" RESET, info->defaultHighOutputLatency);
    printf(FG_BGREEN "\t\t│" RESET "           Low Output Latency  [Default]: %10f       " FG_BGREEN "│\n" RESET, info->defaultLowOutputLatency);
    printf(FG_BGREEN "\t\t│" RESET "    Sample Rate: %-10.1f                                " FG_BGREEN "│\n" RESET, info->defaultSampleRate);
    printf(FG_BGREEN "\t\t└───────────────────────────────────────────────────────────┘\n" RESET);

    }

void print_header_info(wav_header_t* header) {
    printf(FG_BGREEN "\t\t┌─────────────────────────────────────────────────┐\n" RESET);
    printf(FG_BGREEN "\t\t│" RESET "\t\t    WAV Header                    " FG_BGREEN "│\n" RESET);
    printf(FG_BGREEN "\t\t├─────────────────────────────────────────────────┤\n" RESET);
    printf(FG_BGREEN "\t\t│" RESET "    Audio Format:                %8u        " FG_BGREEN "│\n" RESET, header->fmt.audio_format);
    printf(FG_BGREEN "\t\t│" RESET "    Audio Channels:              %8u        " FG_BGREEN "│\n" RESET, header->fmt.audio_channels);
    printf(FG_BGREEN "\t\t│" RESET "    Bits Per Sample:             %8u        " FG_BGREEN "│\n" RESET, header->fmt.bits_per_sample);
    printf(FG_BGREEN "\t\t│" RESET "    Sample Rate:                 %8u        " FG_BGREEN "│\n" RESET, header->fmt.sample_rate);
    printf(FG_BGREEN "\t\t└─────────────────────────────────────────────────┘\n" RESET);
    }




#endif