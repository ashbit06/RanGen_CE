#include <fileioc.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define SEPARATOR '|'

// Function to read a value by key from an appvar
bool readKeyValue(const char *appvarName, const char *key, char *value, size_t valueSize) {
    ti_var_t file = ti_Open(appvarName, "r");
    if (!file) {
        return false; // Appvar not found
    }

    char buffer[128];
    size_t bufferIndex = 0;
    int c;

    // Read the file character by character
    while ((c = ti_GetC(file)) != EOF) {
        if (c == '\n' || bufferIndex >= sizeof(buffer) - 1) {
            // End of a line or buffer full
            buffer[bufferIndex] = '\0'; // Null-terminate the buffer
            char *delimiter = strchr(buffer, SEPARATOR);
            if (delimiter) {
                *delimiter = '\0'; // Split key and value
                if (strcmp(buffer, key) == 0) {
                    strncpy(value, delimiter + 1, valueSize - 1);
                    value[valueSize - 1] = '\0'; // Ensure null-termination
                    ti_Close(file);
                    return true;
                }
            }
            bufferIndex = 0; // Reset buffer for the next line
        } else {
            buffer[bufferIndex++] = (char)c; // Add character to buffer
        }
    }

    ti_Close(file);
    return false; // Key not found
}

// Function to write or update a key-value pair in an appvar
bool writeKeyValue(const char *appvarName, const char *key, const char *value) {
    ti_var_t file = ti_Open(appvarName, "r+");
    if (!file) {
        file = ti_Open(appvarName, "w");
        if (!file) {
            return false; // Failed to create appvar
        }
    }

    char buffer[128];
    size_t fileSize = ti_GetSize(file);
    char *data = malloc(fileSize + 128); // Allocate memory for existing data + new entry
    if (!data) {
        ti_Close(file);
        return false; // Memory allocation failed
    }

    size_t offset = 0;
    bool updated = false;
    size_t bufferIndex = 0;
    int c;

    // Read existing data and update if key exists
    while ((c = ti_GetC(file)) != EOF) {
        if (c == '\n' || bufferIndex >= sizeof(buffer) - 1) {
            // End of a line or buffer full
            buffer[bufferIndex] = '\0'; // Null-terminate the buffer
            char *delimiter = strchr(buffer, SEPARATOR);
            if (delimiter) {
                *delimiter = '\0';
                if (strcmp(buffer, key) == 0) {
                    snprintf(data + offset, 128, "%s%c%s\n", key, SEPARATOR, value);
                    updated = true;
                } else {
                    snprintf(data + offset, 128, "%s%c%s\n", buffer, SEPARATOR, delimiter + 1);
                }
                offset += strlen(data + offset);
            }
            bufferIndex = 0; // Reset buffer for the next line
        } else {
            buffer[bufferIndex++] = (char)c; // Add character to buffer
        }
    }

    // Add new key-value pair if not updated
    if (!updated) {
        snprintf(data + offset, 128, "%s%c%s\n", key, SEPARATOR, value);
        offset += strlen(data + offset);
    }

    // Write updated data back to the appvar
    ti_Rewind(file); // Move to the beginning of the file
    ti_Write(data, offset, 1, file); // Write the updated data
    free(data);
    ti_Close(file);
    return true;
}