#include <string>

// Return a string representing an object's bits.
std::string bits(
    const void *object_ptr,
    size_t object_size,
    char bytes_separator = ' ');
