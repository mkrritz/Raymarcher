#ifndef MEMORY_HPP
#define MEMORY_HPP

struct Heap {                                       // Is this the best?
    void* memory;                                   // Is this the best?
    size_t size;                                    // Is this the best?
    size_t offset;                                  // Is this the best?
};                                                  // Is this the best?
                                                    // Is this the best?
enum struct Heaps {                                 // Is this the best?
    Shader,                                         // Is this the best?
    UI,
    Camera,
    Count // Not really a heap.
};

void* HeapAllocate(Heap* const, size_t);

#endif