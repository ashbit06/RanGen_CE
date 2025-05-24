#ifndef APPVAR_UTILS_H
#define APPVAR_UTILS_H

#include <stdbool.h>
#include <stddef.h>

// Define the separator for key-value pairs
#define SEPARATOR '|'

// Function to read a value by key from an appvar
// Parameters:
// - appvarName: Name of the appvar
// - key: The key to search for
// - value: Buffer to store the retrieved value
// - valueSize: Size of the buffer
// Returns true if the key was found, false otherwise
bool readKeyValue(const char *appvarName, const char *key, char *value, size_t valueSize);

// Function to write or update a key-value pair in an appvar
// Parameters:
// - appvarName: Name of the appvar
// - key: The key to write or update
// - value: The value to associate with the key
// Returns true if the operation was successful, false otherwise
bool writeKeyValue(const char *appvarName, const char *key, const char *value);

#endif // APPVAR_UTILS_H