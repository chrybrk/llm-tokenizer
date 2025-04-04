#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256
#define MAX_PAIRS 256

typedef struct {
    unsigned char new_symbol;
    unsigned char pair[2];
} PairMapping;

// Function to find the most frequent pair of bytes
void find_most_frequent_pair(const unsigned char *data, int length, unsigned char *pair, int *count) {
    int freq[MAX_SYMBOLS][MAX_SYMBOLS] = {0};

    // Count frequency of each pair
    for (int i = 0; i < length - 1; i++) {
        freq[data[i]][data[i + 1]]++;
    }

    // Find the most frequent pair
    *count = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        for (int j = 0; j < MAX_SYMBOLS; j++) {
            if (freq[i][j] > *count) {
                *count = freq[i][j];
                pair[0] = i;
                pair[1] = j;
            }
        }
    }
}

// Function to perform Byte Pair Encoding
int byte_pair_encode(unsigned char *data, int length, PairMapping *mappings, int *mapping_count) {
    unsigned char pair[2];
    int count;

    *mapping_count = 0;

    while (1) {
        find_most_frequent_pair(data, length, pair, &count);
        
        // If no pair is found, break
        if (count <= 0) {
            break;
        }

        // Create a new symbol for the pair
        unsigned char new_symbol = MAX_SYMBOLS + 1; // Start new symbols after 256
        while (new_symbol < MAX_SYMBOLS && memchr(data, new_symbol, length) != NULL) {
            new_symbol++;
        }

        // If we run out of symbols, we can't encode anymore
        if (new_symbol >= 256) {
            break;
        }

        // Store the mapping
        mappings[*mapping_count].new_symbol = new_symbol;
        mappings[*mapping_count].pair[0] = pair[0];
        mappings[*mapping_count].pair[1] = pair[1];
        (*mapping_count)++;

        // Replace occurrences of the pair with the new symbol
        for (int i = 0; i < length - 1; i++) {
            if (data[i] == pair[0] && data[i + 1] == pair[1]) {
                data[i] = new_symbol;
                memmove(&data[i + 1], &data[i + 2], length - i - 2);
                length--;
                i--; // Check the new byte at the same position
            }
        }
    }

    return length; // Return the new length of the data
}

// Function to perform Byte Pair Decoding
void byte_pair_decode(unsigned char *data, int length, PairMapping *mappings, int mapping_count) {
    int i, j;

    // Repeat until no more mappings can be applied
    for (i = 0; i < mapping_count; i++) {
        unsigned char new_symbol = mappings[i].new_symbol;
        unsigned char pair[2] = {mappings[i].pair[0], mappings[i].pair[1]};

        // Replace occurrences of the new symbol with the original pair
        for (j = 0; j < length; j++) {
            if (data[j] == new_symbol) {
                // Shift the data to make space for the pair
                memmove(&data[j + 2], &data[j + 1], length - j - 1);
                data[j] = pair[0];
                data[j + 1] = pair[1];
                length += 1; // Increase length due to added pair
                j++; // Move past the newly inserted pair
            }
        }
    }
}

int main() {
    unsigned char data[] = "ababcabcababc";
    int length = strlen((char *)data);
    PairMapping mappings[MAX_PAIRS];
    int mapping_count;

    printf("Original data: %s\n", data);
    length = byte_pair_encode(data, length, mappings, &mapping_count);
    printf("Encoded data: ");
    for (int i = 0; i < length; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");

		// Decode the data
    unsigned char decoded_data[512]; // Buffer for decoded data
    memcpy(decoded_data, data, length); // Copy encoded data to decoded_data
    int decoded_length = length; // Length of the decoded data

    byte_pair_decode(decoded_data, decoded_length, mappings, mapping_count);
    printf("Decoded data: %s\n", decoded_data);

    return 0;
}
