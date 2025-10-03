// List implementation by Ethan Parker.
//
// To use this library, add a source file with these two lines.
// #define LIST_IMPLEMENTATION
// #include "list.h"
//
// For documentation please see README.md  
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


//#define LIST_USE_MEMCPY

#ifdef LIST_USE_MEMCPY
#define internal_list_copy(dst, src, size) memcpy(dst, src, size)
#else
#define internal_list_copy(dst, src, size) for (unsigned long internal_list_copy_iterator = 0; internal_list_copy_iterator < (unsigned long)size; ++internal_list_copy_iterator) { ((unsigned char*)(dst))[internal_list_copy_iterator] = ((unsigned char*)(src))[internal_list_copy_iterator]; }
#endif

// Type Definitions:
// 
//

typedef struct List {
    /* Simple list container, using a circular buffer. */
    unsigned long capacity;   // Current capacity of the list.
    unsigned long itemSize;   // Size in bytes of an element.
    unsigned char* head;      // Pointer to the "head" of the list.
    unsigned char* tail;      // Pointer to the "tail" of the list.
    unsigned char* data;      // Pointer to the data block of "head" and "tail".
} List;

// List Functions:
// 
//

#define List_create(T, capacity) internal_List_create(sizeof(T), capacity)
List* internal_List_create(const unsigned long ItemSize, const unsigned long Capacity);

#define List_initialize(T, list, capacity) internal_List_initialize(list, sizeof(T), capacity)
void internal_List_initialize(List* list, const unsigned long ItemSize, const unsigned long Capacity);

void List_destroy(List** list);
void List_deinitialize(List* list);

List* List_create_subset(List* list, unsigned long start, unsigned long end);

    // These macros are for manipulating the head and tail pointers, wrapping around the List's data region.
    // If getting the next slot would overrun the buffer, instead loop around to the start of the buffer. Otherwise, increment by ItemSize.
    //

#define List_isEmpty(list) ((list)->head == (list)->tail)
#define internal_List_start(T, list) ((T*)(list)->data)
#define internal_List_end(T, list) (T*)((internal_List_start(unsigned char, (list))) + ((list)->capacity * (list)->itemSize))
#define internal_List_getNextPtr(T, list, ptr) ptr = (T*)(((unsigned char*)ptr == internal_List_end(unsigned char, list) - (list)->itemSize)? internal_List_start(unsigned char, list) : (unsigned char*)ptr + (list)->itemSize)
#define internal_List_getPrevPtr(T, list, ptr) ptr = (T*)(((unsigned char*)ptr == internal_List_start(unsigned char, list))? internal_List_end(unsigned char, list) - (list)->itemSize : (unsigned char*)ptr - (list)->itemSize)
#define internal_List_validate(list) assert((list)->head != (list)->tail)

// Creates T* variable, it, which is the current item in the loop. This will be a little faster than using List_at().
// If you are using function pointers, de-reference it before calling.
#define List_iterator(T, list) T* it = (T*)(list)->tail; (unsigned char*)it != (list)->head; internal_List_getNextPtr(T, list, it)
//#define List_iterator_to(T, list, end) T* it = (T*)(list)->head; (void*)it != List_at(list, (unsigned long)end); internal_List_getPrevPtr(T, list, it)
//#define List_iterator_from(T, list, start) T* it = (T*)List_at(list, start); (unsigned char*)it != (list)->tail; internal_List_getPrevPtr(T, list, it)
//#define List_iterator_range(T, list, start, end) T* it = (T*)List_at(list, start); (void*)it != List_at(list, (unsigned long)end); internal_List_getPrevPtr(T, list, it) 

#define List_push_front(list, Data) internal_List_push_front((list), (void*)&Data)
#define List_push_back(list, Data) internal_List_push_back((list), (void*)&Data)
#define List_peak_front(list, outVal) internal_List_peak_front((list), (void*)&outVal)
#define List_peak_back(list, outVal) internal_List_peak_back((list), (void*)&outVal)
#define List_pop_front(list, outVal) internal_List_pop_front((list), (void*)&outVal)
#define List_pop_back(list, outVal) internal_List_pop_back((list), (void*)&outVal)

#define List_set(list, template, count) internal_List_set(list, (void*)&template, count)

void internal_List_push_front(List* list, void* data);
void internal_List_push_back(List* list, void* data);
void internal_List_pop_front(List* list, void* outVal);
void internal_List_pop_back(List* list, void* outVal);
void internal_List_peak_front(List* list, void* outVal);
void internal_List_peak_back(List* list, void* outVal);

void internal_List_set(List* list, void* template, const unsigned long count);

void* List_create_array(List* list);

unsigned long List_count(const List* list);
unsigned long List_byte_count(const List* list);
bool List_contains_item(const List* list, void* item, unsigned long long* out);

void List_realloc(List* list, unsigned long Capacity);
void List_reorder(List* list);

void List_remove_at(List* list, const unsigned long index);
void* List_at(const List* list, const unsigned long index);

void List_append(List* dst, List* src);

#ifdef __cplusplus
}
#endif

#ifdef LIST_IMPLEMENTATION

List* internal_List_create(const unsigned long ItemSize, const unsigned long Capacity) {
    List* newList = (List*)malloc(sizeof(List));

    internal_List_initialize(newList, ItemSize, Capacity);
    return newList;
}


List* List_create_subset(List* list, unsigned long start, unsigned long end) {
    // Creates a new list as a subset of another list.
    // Returns NULL if the subset cannot be created.
    //

    // cannot create subset with start "in-front of" end.
    if (start >= end) return NULL;

    unsigned long listCount = List_count(list);

    // cannot create subset which is outside of the bounds of the original list.
    if (end > listCount || start >= listCount) return NULL;

    unsigned long capacity = end - start;                    // capacity needed to store the number of items in the subset.
    unsigned long capacityBytes = capacity * list->itemSize; // capacity needed in bytes.

    void* subsetArray = malloc(capacityBytes);
    void* listArray = List_create_array(list);
    internal_list_copy(subsetArray, listArray, capacityBytes);

    List* subset = (List*)malloc(sizeof(List));

    subset->capacity = capacity;
    subset->itemSize = list->itemSize;
    subset->data = (unsigned char*)subsetArray;
    subset->head = (unsigned char*)subsetArray;
    subset->tail = (unsigned char*)subsetArray;

    return subset;
}


void internal_List_initialize(List* list, const unsigned long ItemSize, const unsigned long Capacity) {
    // initialize default values of any fixedList.
    //
    //

    list->data = (unsigned char*)calloc(ItemSize, Capacity);
    //list->data = (unsigned char*)malloc(ItemSize * Capacity);

    // Set capacity and size.
    list->capacity = Capacity;
    list->itemSize = ItemSize;

    // Set the head and tail pointers to point into the data section
    list->head = list->data;
    list->tail = list->data;
}


void List_deinitialize(List* list) {
    // De-initialize a List.

    if (!list) return;

    free(list->data);
    list->data = NULL;
    list->head = NULL;  // points into list->data, no need to free.
    list->tail = NULL;  // points into list->data, no need to free.
}


void List_destroy(List** list) {
    // De-initialize and free a List.

    if (!(*list)) return;

    free((*list)->data);
    (*list)->data = NULL;
    (*list)->head = NULL;   // points into list->data, no need to free.
    (*list)->tail = NULL;   // points into list->data, no need to free.

    free((*list));
    (*list) = NULL;
}


void internal_List_set(List* list, void* template, const unsigned long count) {
    // Clears the List, resizes if needed, and fills it with the data stored in template.
    //
    //

    // If count is greater than the list capacity, reallocate with doubling factor of 1.5
    if (count >= list->capacity) {
        unsigned char* newData = (unsigned char*)malloc(list->itemSize * (count + (count >> 1)));
        free(list->data);
        list->head = newData;
        list->tail = newData;
        list->data = newData;
        list->capacity = count;
    }

    unsigned char* buffer = list->data;
    for (unsigned long i = 0; i < count; i++, buffer += list->itemSize) {
        internal_list_copy(buffer, template, list->itemSize);
    }

    list->head = list->data + (count * list->itemSize);
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
        unsigned long bytesToCopyFromTail = (unsigned long)(internal_List_end(unsigned char, list) - list->tail);
        internal_list_copy(array, list->tail, bytesToCopyFromTail);

        // Copy the second half.
        unsigned long bytesToCopyFromHead = list->head - internal_List_start(unsigned char, list);
        internal_list_copy((unsigned char*)array + bytesToCopyFromTail, internal_List_start(unsigned char, list), bytesToCopyFromHead);
    }
    // list is continuous, and so only one internal_list_copy is needed:
    else {
        unsigned long bytesToCopy = (unsigned long)(list->head - list->tail);
        internal_list_copy(array, list->head, bytesToCopy);
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

    unsigned long countBytes = List_byte_count(list);

    // if the list is continuous, and there is enough room at the start of the list's data block to store the whole list:
    if ((list->tail - list->data) >= countBytes && list->head > list->tail) {
        internal_list_copy(list->data, list->tail, countBytes);
    }
    else {
        void* temp = List_create_array(list);
        internal_list_copy(list->data, temp, list->itemSize * List_count(list));
        free(temp);
    }

    list->tail = list->data;
    list->head = list->data + countBytes;
}


void List_realloc(List* list, unsigned long Capacity) {
    // Reallocates the data section of the list.
    // This function cannot shrink a list. 
    //

    unsigned long oldCount = List_count(list);

    // Always try to reallocate the buffer. Simply clamp Capacity to always be at least list count.
    if (Capacity < oldCount) {
        Capacity = oldCount;
    }

    unsigned char* newData = (unsigned char*)malloc(list->itemSize * Capacity);

    // Jump to end if for some reason you're reallocating an empty array?? why are you doing that?
    if (list->head == list->tail) {
        goto ReassignPointers;
    }

    // The copy can be simplified since the list is either continuous, or split in two.
    //
    // if the head is behind the tail, list is split in two:
    if (list->head < list->tail) {
        // Copy first half.
        unsigned long bytesToCopyFromTail = internal_List_end(unsigned char, list) - list->tail;
        internal_list_copy(newData, list->tail, bytesToCopyFromTail);

        // Copy the second half.
        unsigned long bytesToCopyFromHead = list->head - internal_List_start(unsigned char, list);
        internal_list_copy(newData + bytesToCopyFromTail, internal_List_start(unsigned char, list), bytesToCopyFromHead);
    }
    // list is continuous, and so only one internal_list_copy is needed:
    else {
        unsigned long bytesToCopy = (unsigned long)(list->head - list->tail);
        internal_list_copy(newData, list->tail, bytesToCopy);
    }

    ReassignPointers:
    // Reassign the head and tail to point into the new data section.
    list->tail = newData;
    list->head = newData + (oldCount * list->itemSize);

    // free the old data, and set the pointer.
    free(list->data);
    list->data = newData;

    // Update the capacity.
    list->capacity = Capacity;
}


unsigned long List_byte_count(const List* list) {
    //  Returns the number of bytes used.
    //
    //

    // If the head is in-front of the tail:
    if (list->head > list->tail) {
        return (unsigned long)(list->head - list->tail);
    }
    // If the head is behind of the tail:
    else {
        return list->capacity - (unsigned long)(list->tail - list->head);
    }
}


unsigned long List_count(const List* list) {
    //  Returns the number of slots used.
    //
    //

    if (list->head == list->tail) {
        return 0;
    }
    // If the head is in-front of the tail:
    else if (list->head > list->tail) {
        return ((unsigned long)(list->head - list->tail)) / list->itemSize;
    }
    // If the head is behind of the tail:
    else {
        return (list->capacity - ((unsigned long)(list->tail - list->head) / list->itemSize));
    }
}


bool List_contains_item(const List* list, void* item, unsigned long long* out) {
    unsigned char* itemAsBytes = (unsigned char*)item;
    unsigned long long itemIndex = 0;
    for (List_iterator(unsigned char, list), ++itemIndex) {
        for (unsigned long i = 0; i < list->itemSize; ++i) {
            if (it[i] != itemAsBytes[i]) {
                goto NotMatching;
            }
        }
        if (out) *out = itemIndex;
        return true;
        NotMatching:
        continue;
    }
    if (out) *out = 0;
    return false;
}


void* List_at(const List* list, const unsigned long index) {
    // Returns the item at a particular index.
    //
    //

    // Return NULL if index is out of range.
    if (index > List_count(list)) {
        return NULL;
    }

    // Get the pointer to the tail, Index should count up from the tail towards the head.
    unsigned char* dataPointer = (unsigned char*)list->tail;

    // Add the offset to the data pointer,
    dataPointer += index * list->itemSize;

    if (dataPointer < internal_List_end(unsigned char, list)) {
        return dataPointer;
    }

    // if its outside the bounds of the List, wrap around to the start. 
    // This is okay since i <= capacity so it could only ever wrap once.
    dataPointer -= list->capacity * list->itemSize;
    return dataPointer;
}


void List_remove_at(List* list, const unsigned long long index) {
    // Remove item from a specific index.
    //
    //

    // Check that i is valid.
    if (index > List_count(list)) {
        return;
    }

    void* currentIndex = List_at(list, index);
    void* nextIndex = NULL;

    for (unsigned long long i = index; i < list->capacity - 1; i++) {
        nextIndex = List_at(list, i + 1);                               // Get the address of the next item.
        internal_list_copy(currentIndex, nextIndex, list->itemSize);    // Using internal_list_copy here since we don't know the type stored, only how many bytes it is. 
        currentIndex = nextIndex;
    }

    // Decrement the head since we shifted everything back. 
    internal_List_getPrevPtr(unsigned char, list, list->head);
}


void internal_List_push_back(List* list, void* data) {
    // Reallocate the array with a doubling factor of 1.5
    if (List_count(list) >= list->capacity - 1) {
        List_realloc(list, list->capacity + (list->capacity << 1));
    }
    internal_list_copy(list->head, data, list->itemSize);
    internal_List_getNextPtr(unsigned char, list, list->head);
}


void internal_List_push_front(List* list, void* data) {
    // Reallocate the array with a doubling factor of 1.5
    if (List_count(list) >= list->capacity - 1) {
        List_realloc(list, list->capacity + (list->capacity << 1));
    }
    internal_list_copy(list->tail, data, list->itemSize);
    internal_List_getPrevPtr(unsigned char, list, list->tail);
}


void internal_List_pop_front(List* list, void* outVal) {
    // Returns next item from the front of the list.
    // If this is a pointer type, you must now free it.
    //
    if (list->head == list->tail) return;   // Leave early if there is no data.

    internal_List_getPrevPtr(unsigned char, list, list->head);
    internal_list_copy(outVal, list->head, list->itemSize);
}


void internal_List_pop_back(List* list, void* outVal) {
    // Returns next item from the back of the list. 
    // If this is a pointer type, you must now free it.
    //
    if (list->head == list->tail) return;   // Leave early if there is no data.

    internal_list_copy(outVal, list->tail, list->itemSize);
    internal_List_getNextPtr(unsigned char, list, list->tail);
}


void internal_List_peak_front(List* list, void* outVal) {
    if (list->head == list->tail) return;   // Leave early if there is no data.

    unsigned char* temp = list->head;
    internal_List_getPrevPtr(unsigned char, list, temp);
    internal_list_copy(outVal, temp, list->itemSize);
}


void internal_List_peak_back(List* list, void* outVal) {
    if (list->head == list->tail) return;   // Leave early if there is no data.
    internal_list_copy(outVal, list->tail, list->itemSize);
}


void List_append(List* dst, List* src) {
    // Appends one list to another.
    //
    //

    if (!dst || !src) return;

    // leave early if the list contain different data "types".
    if (dst->itemSize != src->itemSize) return;

    unsigned long sourceByteCount = List_byte_count(src);
    unsigned long sourceCount = List_count(src);
    unsigned long combinedCount = List_count(dst) + sourceCount;
    void* srcArray = List_create_array(src);

    if (combinedCount >= dst->capacity) {
        List_realloc(dst, combinedCount);
    }

    else {
        List_reorder(dst);
    }

    // At this point, the list must be in order since both realloc and reorder cause the array to not be split.
    internal_list_copy(dst->head, srcArray, sourceByteCount);
    dst->head += sourceByteCount;
    free(srcArray);
}

#endif