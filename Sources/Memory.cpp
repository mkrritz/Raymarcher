
#include "pch.h"

#include "mkr/Assert.h"
#include "mkr/Memory.h"

static Heap* HeapInit(size_t size) {
    Heap* heap = (Heap*)malloc(sizeof(Heap));
    ASSERT(heap == nullptr, "\nHoly shit");
    heap->memory = calloc(1, size);
    ASSERT(heap->memory == nullptr, "Not better");
    heap->size = size;
    heap->offset = 0;
    return heap;
}

void HeapsInit(Heap* heaps[]) {
    heaps[static_cast<uint32_t>(Heaps::Camera)]  = HeapInit(1024);
    heaps[static_cast<uint32_t>(Heaps::UI)]      = HeapInit(1024);
    heaps[static_cast<uint32_t>(Heaps::Shader)]  = HeapInit(10 * 1024);
    
}

void* HeapAllocate(Heap* const heap, size_t size) {
    ASSERT (heap->offset + size > heap->size, "Heap full. Give more memory.");
    void* ptr = (char*)heap->memory + heap->offset;
    heap->offset += size;
    return ptr;
}
