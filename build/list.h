// List implementation by Ethan Parker.
//
// To use this library, add a source file with these two lines.
// #define LIST_IMPLEMENTATION
// #include "list.h"
//
// For documentation and useage information please see README.md  
//

#pragma once

#if !defined(_LIST_INCLUDED)
#define _LIST_INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//#define LIST_USE_MEMCPY

#ifdef LIST_USE_MEMCPY
#define _list_copy(dst, src, size) memcpy(dst, src, size)
#else
#define _list_copy(dst, src, size) do { for (size_t __iter = 0; __iter < (size_t)size; ++__iter) { ((uint8_t*)(dst))[__iter] = ((uint8_t*)(src))[__iter]; } } while(0)
#endif

// Type Definitions:
// 
//

/* Simple list container, using a circular buffer. */
typedef struct List {
    size_t capacity;   // Current capacity of the list.
    size_t _typeSize;   // Size in bytes of an element.
    uint8_t* head;     // Pointer to the "head" of the list.
    uint8_t* tail;     // Pointer to the "tail" of the list.
    uint8_t* data;     // Pointer to the data block of "head" and "tail".
} List;

// List Functions:
// 
//

#define List_create(T, capacity) _List_create(sizeof(T), capacity)
List* _List_create(const size_t ItemSize, const size_t Capacity);

// Destroy a list. This will also 
#define List_destroy(list) _List_destroy(&(list))
void _List_destroy(List** list);

// Create a new List as a subset of another List. Returns NULL if the subset cannot be created.
List* List_create_subset(List* list, size_t start, size_t end);

#define List_is_empty(list) ((list)->head == (list)->tail)

// These macros are for manipulating the head and tail pointers, wrapping around the List's data region.
// If getting the next slot would overrun the buffer, instead loop around to the start of the buffer. Otherwise, increment or decrement by "typeSize".
// This allows for fast item retrival - besides the function call to access the data - it is just an array access.

#define _List_start(T, list) ((T*)(list)->data)
#define _List_end(T, list) (T*)((_List_start(uint8_t, (list))) + ((list)->capacity * (list)->_typeSize))
#define _List_get_next(T, list, ptr) ptr = (T*)(((uint8_t*)ptr == _List_end(uint8_t, list) - (list)->_typeSize)? _List_start(uint8_t, list) : (uint8_t*)ptr + (list)->_typeSize)
#define _List_get_prev(T, list, ptr) ptr = (T*)(((uint8_t*)ptr == _List_start(uint8_t, list))? _List_end(uint8_t, list) - (list)->_typeSize : (uint8_t*)ptr - (list)->_typeSize)
#define _List_validate(list) assert((list)->head != (list)->tail)

// Similar to a "foreach" loop in languages like C# or C++. Creates T* variable, "it", which is the current item.
#define List_iterator(T, list) T* it = (T*)(list)->tail; (uint8_t*)it != (list)->head; _List_get_next(T, list, it)

//#define List_iterator_to(T, list, end) T* it = (T*)(list)->head; (void*)it != List_at(list, (size_t)end); _List_get_prev(T, list, it)
//#define List_iterator_from(T, list, start) T* it = (T*)List_at(list, start); (uint8_t*)it != (list)->tail; _List_get_prev(T, list, it)
//#define List_iterator_range(T, list, start, end) T* it = (T*)List_at(list, start); (void*)it != List_at(list, (size_t)end); _List_get_prev(T, list, it) 

#define List_push_front(list, value) _List_push_front((list), (void*)&(value))
#define List_push_back(list, value) _List_push_back((list), (void*)&(value))

#define List_peak_front(list, outVal) _List_peak_front((list), (void*)&outVal)

#define List_peak_back(list, outVal) _List_peak_back((list), (void*)&outVal)

// Remove an item from the front of the list. The value is copied into "out".
#define List_pop_front(list, outVal) _List_pop_front((list), (void*)&outVal)

// Remove an item from the back of the list. The value is copied into "out".
#define List_pop_back(list, outVal) _List_pop_back((list), (void*)&outVal)

// Clears the List, resizes if needed, and fills it with the data stored in template. Template is expected to be the same type as was used to initialize the list.
#define List_set(list, template, count) _List_set(list, (void*)&(template), count)

void _List_push_front(List* list, void* data);
void _List_push_back(List* list, void* data);
void _List_pop_front(List* list, void* outVal);
void _List_pop_back(List* list, void* outVal);
void _List_peak_front(List* list, void* outVal);
void _List_peak_back(List* list, void* outVal);

void _List_set(List* list, void* template, const size_t count);

void* List_create_array(List* list);

size_t List_count(const List* list);
size_t List_byte_count(const List* list);
bool List_contains_item(const List* list, void* item, size_t* out);

void List_realloc(List* list, size_t Capacity);
void List_reorder(List* list);

void List_remove_at(List* list, const size_t index);
void* List_at(const List* list, const size_t index);

void List_append(List* src, List* dst);

#if defined(__cplusplus)
}
#endif
#endif

#if defined(LIST_IMPLEMENTATION)
#undef LIST_IMPLEMENTATION

static bool _List_initialize(List* list, const size_t ItemSize, const size_t Capacity) {
    list->data = (uint8_t*)calloc(ItemSize, Capacity);
    
    if (!list->data) {
        return false;
    }
    list->capacity = Capacity;
    list->_typeSize = ItemSize;
    list->head = list->data;
    list->tail = list->data;
    
    return true;
}




List* _List_create(const size_t ItemSize, const size_t Capacity) {
    List* newList = (List*)malloc(sizeof(List));

    if (!_List_initialize(newList, ItemSize, Capacity)) {
        retrun NULL; 
    }

    return newList;
}


List* List_create_subset(List* list, size_t start, size_t end) {
    

    // cannot create subset with start "in-front of" end.
    if (start >= end) return NULL;

    size_t listCount = List_count(list);

    // cannot create subset which is outside of the bounds of the original list.
    if (end > listCount || start >= listCount) return NULL;

    size_t capacity = end - start;                    // capacity needed to store the number of items in the subset.
    size_t capacityBytes = capacity * list->_typeSize; // capacity needed in bytes.

    void* subsetArray = malloc(capacityBytes);
    void* listArray = List_create_array(list);
    _list_copy(subsetArray, listArray, capacityBytes);

    List* subset = (List*)malloc(sizeof(List));

    subset->capacity = capacity;
    subset->_typeSize = list->_typeSize;
    subset->data = (uint8_t*)subsetArray;
    subset->head = (uint8_t*)subsetArray;
    subset->tail = (uint8_t*)subsetArray;

    return subset;
}


void List_destroy(List** list) {
    if (!list || !(*list)) return;

    free((*list)->data);
    (*list)->data = NULL;
    (*list)->head = NULL;   // points into list->data, no need to free.
    (*list)->tail = NULL;   // points into list->data, no need to free.

    free((*list));
    (*list) = NULL;
}


void _List_set(List* list, void* template, const size_t count) {
    //
    //

    // If count is greater than the list capacity, reallocate with doubling factor of 1.5
    if (count >= list->capacity) {
        uint8_t* newData = (uint8_t*)malloc(list->_typeSize * (count + (count >> 1)));
        free(list->data);
        list->head = newData;
        list->tail = newData;
        list->data = newData;
        list->capacity = count;
    }

    uint8_t* buffer = list->data;
    for (size_t i = 0; i < count; i++, buffer += list->_typeSize) {
        _list_copy(buffer, template, list->_typeSize);
    }

    list->head = list->data + (count * list->_typeSize);
}


void* List_create_array(List* list) {
    // Creates a C-Style pointer array from the list.
    // you must free this list yourself!
    //

    void* array = malloc(List_byte_count(list));

    // The copy can be simplified since the list is either continuous, or split in two.
    //
    // if the head is behind the tail, list is split in two:
    if (list->head < list->tail) {
        // Copy first half.
        size_t bytesToCopyFromTail = (size_t)(_List_end(uint8_t, list) - list->tail);
        _list_copy(array, list->tail, bytesToCopyFromTail);

        // Copy the second half.
        size_t bytesToCopyFromHead = list->head - _List_start(uint8_t, list);
        _list_copy((uint8_t*)array + bytesToCopyFromTail, _List_start(uint8_t, list), bytesToCopyFromHead);
    }
    // list is continuous, and so only one _list_copy is needed:
    else {
        size_t bytesToCopy = (size_t)(list->head - list->tail);
        _list_copy(array, list->head, bytesToCopy);
    }

    return array;
}


void List_reorder(List* list) {
    // Reorders the data section of the list so that it isn't split.
    //
    //

    // do nothing if the list is empty.
    if (list->head == list->tail) return;

    // tail = data means the list is already ordered, so do nothing.
    if (list->tail == list->data) return;

    size_t countBytes = List_byte_count(list);

    // if the list is continuous, and there is enough room at the start of the list's data block to store the whole list:
    if ((list->tail - list->data) >= countBytes && list->head > list->tail) {
        _list_copy(list->data, list->tail, countBytes);
    }
    else {
        void* temp = List_create_array(list);
        _list_copy(list->data, temp, list->_typeSize * List_count(list));
        free(temp);
    }

    list->tail = list->data;
    list->head = list->data + countBytes;
}


void List_realloc(List* list, size_t Capacity) {
    // Reallocates the data section of the list.
    // This function cannot shrink a list. 
    //

    size_t oldCount = List_count(list);

    // Always try to reallocate the buffer. Simply clamp Capacity to always be at least list count.
    if (Capacity < oldCount) {
        Capacity = oldCount;
    }

    uint8_t* newData = (uint8_t*)malloc(list->_typeSize * Capacity);

    // Jump to end if for some reason you're reallocating an empty array?? why are you doing that?
    if (list->head == list->tail) {
        goto ReassignPointers;
    }

    // The copy can be simplified since the list is either continuous, or split in two.
    //
    // if the head is behind the tail, list is split in two:
    if (list->head < list->tail) {
        // Copy first half.
        size_t bytesToCopyFromTail = _List_end(uint8_t, list) - list->tail;
        _list_copy(newData, list->tail, bytesToCopyFromTail);

        // Copy the second half.
        size_t bytesToCopyFromHead = list->head - _List_start(uint8_t, list);
        _list_copy(newData + bytesToCopyFromTail, _List_start(uint8_t, list), bytesToCopyFromHead);
    }
    // list is continuous, and so only one _list_copy is needed:
    else {
        size_t bytesToCopy = (size_t)(list->head - list->tail);
        _list_copy(newData, list->tail, bytesToCopy);
    }

    ReassignPointers:
    // Reassign the head and tail to point into the new data section.
    list->tail = newData;
    list->head = newData + (oldCount * list->_typeSize);

    // free the old data, and set the pointer.
    free(list->data);
    list->data = newData;

    // Update the capacity.
    list->capacity = Capacity;
}


size_t List_byte_count(const List* list) {
    //  Returns the number of bytes used.
    //
    //

    // If the head is in-front of the tail:
    if (list->head > list->tail) {
        return (size_t)(list->head - list->tail);
    }
    // If the head is behind of the tail:
    else {
        return list->capacity - (size_t)(list->tail - list->head);
    }
}


size_t List_count(const List* list) {
    //  Returns the number of slots used.
    //
    //

    if (list->head == list->tail) {
        return 0;
    }
    // If the head is in-front of the tail:
    else if (list->head > list->tail) {
        return ((size_t)(list->head - list->tail)) / list->_typeSize;
    }
    // If the head is behind of the tail:
    else {
        return (list->capacity - ((size_t)(list->tail - list->head) / list->_typeSize));
    }
}


bool List_contains_item(const List* list, void* item, size_t* out) {
    uint8_t* itemBytes = (uint8_t*)item;
    size_t itemIndex = 0;
    for (List_iterator(uint8_t, list), ++itemIndex) {
        for (size_t i = 0; i < list->_typeSize; ++i) {
            if (it[i] != itemBytes[i]) {
                goto NOT_MATCHING;
            }
        }
        if (out) *out = itemIndex;
        return true;
        NOT_MATCHING:
        continue;
    }
    if (out) *out = 0;
    return false;
}


void* List_at(const List* list, const size_t index) {
    // Returns the item at a particular index.
    //
    //

    // Return NULL if index is out of range.
    if (index > List_count(list)) {
        return NULL;
    }

    // Get the pointer to the tail, Index should count up from the tail towards the head.
    uint8_t* dataPointer = (uint8_t*)list->tail;

    // Add the offset to the data pointer,
    dataPointer += index * list->_typeSize;

    if (dataPointer < _List_end(uint8_t, list)) {
        return dataPointer;
    }

    // if its outside the bounds of the List, wrap around to the start. 
    // This is okay since i <= capacity so it could only ever wrap once.
    dataPointer -= list->capacity * list->_typeSize;
    return dataPointer;
}


bool List_ordered_remove(List* list, const size_t index) {
    if (index > List_count(list)) {
        return false;
    }

    void* currentIndex  = List_at(list, index);
    void* nextIndex     = NULL;

    for (size_t i = index; i < list->capacity - 1; ++i) {
        nextIndex = List_at(list, i + 1);
        _list_copy(currentIndex, nextIndex, list->_typeSize); 
        currentIndex = nextIndex;
    }

    // Decrement the head since we shifted everything back. 
    _List_get_prev(uint8_t, list, list->head);

    return true;
}


void _List_push_back(List* list, void* data) {
    if (List_count(list) >= list->capacity - 1) {
        List_realloc(list, list->capacity + (list->capacity << 1));
    }
    _list_copy(list->head, data, list->_typeSize);
    _List_get_next(uint8_t, list, list->head);
}


void _List_push_front(List* list, void* data) {
    if (List_count(list) >= list->capacity - 1) {
        List_realloc(list, list->capacity + (list->capacity << 1));
    }
    _list_copy(list->tail, data, list->_typeSize);
    _List_get_prev(uint8_t, list, list->tail);
}


void _List_pop_front(List* list, void* outVal) {
    
    if (list->head == list->tail) return;   // Leave early if there is no data.

    _List_get_prev(uint8_t, list, list->head);
    _list_copy(outVal, list->head, list->_typeSize);
}


void _List_pop_back(List* list, void* outVal) {
    // Returns next item from the back of the list. 
    // If this is a pointer type, you must now free it.
    //
    if (list->head == list->tail) return;   // Leave early if there is no data.

    _list_copy(outVal, list->tail, list->_typeSize);
    _List_get_next(uint8_t, list, list->tail);
}


void _List_peak_front(List* list, void* outVal) {
    if (list->head == list->tail) return;   // Leave early if there is no data.

    uint8_t* temp = list->head;
    _List_get_prev(uint8_t, list, temp);
    _list_copy(outVal, temp, list->_typeSize);
}


void _List_peak_back(List* list, void* outVal) {
    if (list->head == list->tail) return;   // Leave early if there is no data.
    _list_copy(outVal, list->tail, list->_typeSize);
}


bool List_append(List* dst, List* src) {
    if (!dst || !src) return false;

    // leave early if the list contain different data "types" (size miss-match).
    if (dst->_typeSize != src->_typeSize) return false;

    size_t  sourceByteCount = List_byte_count(src);
    size_t  sourceCount     = List_count(src);
    size_t  combinedCount   = List_count(dst) + sourceCount;
    void*   srcArray        = List_create_array(src);

    if (combinedCount >= dst->capacity) {
        if (!List_realloc(dst, combinedCount)) {
            return false;
        }
    }

    else {
        List_reorder(dst);
    }

    // At this point, the list must be in order since both realloc and reorder cause the array to not be split.
    _list_copy(dst->head, srcArray, sourceByteCount);
    dst->head += sourceByteCount;
    free(srcArray);
    return true;
}

#endif
