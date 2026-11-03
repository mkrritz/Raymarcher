#ifndef MEMORY_HPP
#define MEMORY_HPP

struct Heap {
    void* memory;
    size_t size;
    size_t offset;
};

enum struct Heaps {
    Shader,
    UI,
    Camera,
    Count // Not really a heap.
};

void* HeapAllocate(Heap* const, size_t);

#endif