#pragma once
#if !defined(_LIST_INCLUDED)
#define _LIST_INCLUDED

/* List implementation by Ethan Parker.
*
* To use this library, add a source file with these two lines:
*
* #define LIST_IMPLEMENTATION
* #include "list.h"
*
* For documentation and usage information please see README.md
*/

#if __STDC_VERSION__ < 199901L
#error List implementation is only compatable with C99 and above.
#endif

#include <stddef.h>

#define _list_copy(dst, src, size) do { for (size_t __iter = 0; __iter < (size_t)size; ++__iter) { ((char*)(dst))[__iter] = ((char*)(src))[__iter]; } } while(0)

/* Simple list container, using a circular buffer. */
typedef struct List {
    size_t      _typeSize;  /* Size (in bytes) of an element.                   */
    size_t      capacity;   /* Current capacity of the list.                    */
    char*       head;       /* Pointer to the "head" of the list.               */
    char*       tail;       /* Pointer to the "tail" of the list.               */
    char*       data;       /* Pointer to the data block of "head" and "tail".  */
} List;

/* @brief Creates a List. 
* @param typeSize The size of the type (in bytes) stored in this list. 
* @param capacity initial capacity the list should be initialized to. 
*/
#define List_create(T, capacity) _List_create(sizeof(T), capacity)

List   _List_create (const size_t typeSize, const size_t capacity);
void    List_destroy (List* list);

/* @brief Create a new List as a subset of another List, copying the values into the new list. 
* @param list Pointer to a list the subset is to be created from. 
* @param start index the subset starts from.
* @param end index the subset ends at.
* @return A valid list when success, or an invalid List if the subset cannot be created (data pointer will be set NULL). 
*/
List List_create_subset (List* list, size_t start, size_t end);

#define List_set(list, T, value, count) _List_set(list, (void*)(&(T){value}), count)
_Bool _List_set(List* list, void* data, const size_t count);

/* 
* These macros are for manipulating the head and tail pointers, wrapping around the List's data region.
* If getting the next slot would overrun the buffer, instead loop around to the start of the buffer. Otherwise, increment or decrement by "typeSize".
* This allows for fast item retrieval - besides the function call to access the data - it is just an array access.
*/

#define List_is_empty(list) ((list)->head == (list)->tail)
#define _List_start(T, list) ((T*)(list)->data)
#define _List_end(T, list) (T*)((_List_start(char, (list))) + ((list)->capacity * (list)->_typeSize))
#define _List_get_next(T, list, ptr) ptr = (T*)(((char*)ptr == _List_end(char, list) - (list)->_typeSize)? _List_start(char, list) : (char*)ptr + (list)->_typeSize)
#define _List_get_prev(T, list, ptr) ptr = (T*)(((char*)ptr == _List_start(char, list))? _List_end(char, list) - (list)->_typeSize : (char*)ptr - (list)->_typeSize)

/* Similar to a for - each loop in languages like C# or C++. Creates T * variable, "it", which is the current item. */
#define List_iterator(list, T) T* it = (T*)(list)->tail; (char*)it != (list)->head; _List_get_next(T, list, it)

/* @brief Copy an item into the front list. 
* @param list List to push data into.
* @param T Type of the value being passed in. Must be the same type as was used to create the list.
* @param value value to be pushed into the list. See documentation for examples.
* @returns true (1) when the push was successful, false (0) if the list was too small and could not be reallocated. */
#define List_push_front(list, T, value) _List_push_front(list, (void*)(&(T){value}))
_Bool  _List_push_front (List* list, void* data);

/* @brief Copy an item into the back of the list. 
* @param T Type of the value being passed in. Must be the same type as was used to create the list.
* @param list List to push data into.
* @param value value to be pushed into the list. See documentation for examples.
* @returns true (1) when the push was successful, false (0) if the list was too small and could not be reallocated. */
#define List_push_back(list, T, value) _List_push_back(list, (void*)(&(T){value}))
_Bool   _List_push_back (List* list, void* data);

/* @breif Remove an item from the front of the list. The value is copied into "out".
* @param list List to be modified.
* @param out (OPTIONAL) pointer to a variable that will store the data removed from the list.
* @returns true (1) if data was be removed, false (0) if the list is empty. */
_Bool   List_pop_front (List* list, void* out);

/* @breif Remove an item from the back of the list. The value is copied into "out".
* @param list List to be modified.
* @param out (OPTIONAL) pointer to a variable that will store the data removed from the list.
* @returns true (1) if data was be removed, false (0) if the list is empty. */
_Bool   List_pop_back (List* list, void* out);

_Bool   List_peak_front (List* list, void* out);
_Bool   List_peak_back (List* list, void* out);

/* Copy the data from a list into an array. Use List_byte_count to allocate a correctly sized array. No bounds checking will be performed. */
void    List_copy_to_array (List* list, void* array);

size_t  List_count (const List* list);
size_t  List_byte_count (const List* list);

/* Reallocates the data section of the list, shrinking or growing to accommodate the new capacity. */
_Bool   List_realloc (List* list, size_t Capacity);

/* Reorder the internal data structure, ensuring the buffer is contiguous with the start at index 0.*/
void    List_reorder (List* list);

/* Remove an item from an index and preserve the order of the array.*/
_Bool   List_remove_ordered (List* list, const size_t index);

/* Remove an item by replacing it with what is stored at the end of the list.*/
_Bool   List_remove_unordered (List* list, const size_t index);

/* Get an item from a list at a specific index.*/
void*   List_at (const List* list, const size_t index);

/* Appends the contents of source list to the end of destination list.*/
_Bool   List_append (List* src, List* dst);

#endif

#if defined(LIST_IMPLEMENTATION)
#undef LIST_IMPLEMENTATION

#if !defined(_LIST_INCLUDED)
#error Missing list.h include in definition.
#endif

#include <stdlib.h>

List _List_create (const size_t typeSize, const size_t capacity) {
    char* data = (char*)calloc(typeSize, capacity);

    if (!data) {
        return (List) { 0 };
    }

    List list = {
        .capacity = capacity,
        ._typeSize = typeSize,
        .data = data,
        .head = data,
        .tail = data,
    };

    return list;
}


List List_create_subset (List* list, size_t start, size_t end) {
    /* cannot create a subset from an invalid list. */
    if (!list || list->_typeSize == 0) return (List) { 0 };

    /* cannot create subset with start "in-front of" end, or empty list. */
    if (start >= end) return (List) { 0 };

    size_t listByteCount    = List_byte_count(list);
    size_t listCount        = listByteCount / list->_typeSize;

    /* cannot create subset which is outside of the bounds of the original list. */
    if (end > listCount || start >= listCount) return (List) { 0 };

    size_t capacity = end - start + 1;                  /* capacity needed to store the number of items in the subset. */
    size_t capacityBytes = capacity * list->_typeSize;  /* capacity needed in bytes. */

    void* subsetArray = malloc(capacityBytes);
    List_copy_to_array(list, subsetArray);

    List subset = {
        .capacity = capacity,
        ._typeSize = list->_typeSize,
        .data = (char*)subsetArray,
        .tail = (char*)subsetArray,
        .head = (char*)subsetArray + listByteCount,
    };

    return subset;
}


_Bool _List_set (List* list, void* data, const size_t count) {
    if (!list) {
        return 0;
    }

    /* effectively erase what was in the list by pointing head and tail back to data. */
    list->head = list->data;
    list->tail = list->data;
    
    if (list->capacity <= count) {
        if (!List_realloc(list, count + 1)) {
            return 0;
        }
    }

    for (size_t i = 0; i < count; ++i) {
        if (!_List_push_back(list, data)) {
            return 0;
        }
    }
    return 1;
}


void List_destroy (List* list) {
    free(list->data);
    list->data = NULL;
    list->head = NULL;   /* points into list->data, no need to free. */
    list->tail = NULL;   /* points into list->data, no need to free. */
}


void List_copy_to_array (List* list, void* array) {
    if (!array) {
        return;
    }

    /* The copy can be simplified since the list is either continuous, or split in two. */
    /* if the head is behind the tail, list is split in two: */
    if (list->head < list->tail) {
        /* Copy first half. */
        size_t bytesToCopyFromTail = (size_t)(_List_end(char, list) - list->tail);
        _list_copy(array, list->tail, bytesToCopyFromTail);

        /* Copy the second half. */
        size_t bytesToCopyFromHead = list->head - _List_start(char, list);
        _list_copy((char*)array + bytesToCopyFromTail, _List_start(char, list), bytesToCopyFromHead);
    }
    /* list is continuous, and so only one _list_copy is needed: */
    else {
        size_t bytesToCopy = (size_t)(list->head - list->tail);
        _list_copy(array, list->head, bytesToCopy);
    }
}

void List_reorder(List* list) {
    /* do nothing if the list is empty. */
    if (list->head == list->tail) return;

    /* tail = data means the list is already ordered, so do nothing. */
    if (list->tail == list->data) return;

    size_t countBytes = List_byte_count(list);

    /* if the list is continuous, and there is enough room at the start of the list's data block to store the whole list: */
    if ((size_t)(list->tail - list->data) >= countBytes && list->head > list->tail) {
        _list_copy(list->data, list->tail, countBytes);
    }
    /* Otherwise, the start area is in use and we need to copy it in chunks. */
    else {
        char* temp = malloc(countBytes);
        if (!temp) {
            return;
        }

        if (list->head < list->tail) {
            /* Copy first half. */
            size_t bytesToCopyFromTail = _List_end(char, list) - list->tail;
            _list_copy(temp, list->tail, bytesToCopyFromTail);

            /* Copy the second half. */
            size_t bytesToCopyFromHead = list->head - _List_start(char, list);
            _list_copy(temp + bytesToCopyFromTail, _List_start(char, list), bytesToCopyFromHead);
        }
        /* list is continuous, and so only one _list_copy is needed: */
        else {
            size_t bytesToCopy = (size_t)(list->head - list->tail);
            _list_copy(temp, list->tail, bytesToCopy);
        }

        _list_copy(list->data, temp, countBytes);
        free(temp);
    }
    list->tail = list->data;
    list->head = list->data + countBytes;
}


_Bool List_realloc(List* list, size_t Capacity) {

    size_t oldCount = List_count(list);
    /* Always try to reallocate the buffer. Simply clamp Capacity to always be at least list count. */
    if (Capacity < oldCount) {
        Capacity = oldCount;
    }

    char* newData = (char*)malloc(list->_typeSize * Capacity);
    if (!newData) return 0;

    /* Jump to end if for some reason you're reallocating an empty array (Nothing to copy). */
    if (list->head == list->tail) {
        goto ReassignPointers;
    }

    /* The copy can be simplified since the list is either continuous, or split in two. */
    /* if the head is behind the tail, list is split in two:  */
    if (list->head < list->tail) {
        /* Copy first half. */
        size_t bytesToCopyFromTail = _List_end(char, list) - list->tail;
        _list_copy(newData, list->tail, bytesToCopyFromTail);

        /* Copy the second half. */
        size_t bytesToCopyFromHead = list->head - _List_start(char, list);
        _list_copy(newData + bytesToCopyFromTail, _List_start(char, list), bytesToCopyFromHead);
    }
    /* list is continuous, and so only one _list_copy is needed: */
    else {
        size_t bytesToCopy = (size_t)(list->head - list->tail);
        _list_copy(newData, list->tail, bytesToCopy);
    }

ReassignPointers:
    /* Reassign the head and tail to point into the new data section. */
    list->tail = newData;
    list->head = newData + (oldCount * list->_typeSize);

    /* free the old data, and set the pointer. */
    free(list->data);
    list->data = newData;

    /* Update the capacity. */
    list->capacity = Capacity;
    return 1;
}


size_t List_byte_count(const List* list) {
    /* If the head is in-front of the tail: */
    if (list->head > list->tail) {
        return (size_t)(list->head - list->tail);
    }
    /* If the head is behind of the tail: */
    else {
        return list->capacity - (size_t)(list->tail - list->head);
    }
}


size_t List_count(const List* list) {
    if (list->head == list->tail) {
        return 0;
    }
    /* If the head is in-front of the tail: */
    else if (list->head > list->tail) {
        return ((size_t)(list->head - list->tail)) / list->_typeSize;
    }
    /* If the head is behind of the tail: */
    else {
        return (list->capacity - ((size_t)(list->tail - list->head) / list->_typeSize));
    }
}


void* List_at(const List* list, const size_t index) {
    /* Return NULL if the list is empty. */
    if (list->head == list->tail) {
        return NULL;
    }
    
    /* Return NULL if index is out of range. */
    if (index > List_count(list)) {
        return NULL;
    }

    /* Start from the pointer to the tail, Index should count up from the tail towards the head. */
    char* dataPointer = list->tail + (index * list->_typeSize);

    if (dataPointer < _List_end(char, list)) {
        return dataPointer;
    }

    /* if it's outside the bounds of the List, wrap around to the start. */
    /* This is OK since index must be less or equal to capacity. It could only ever wrap once. */
    dataPointer -= (list->capacity * list->_typeSize);
    return dataPointer;
}


_Bool List_remove_unordered(List* list, const size_t index) {
    if (index > List_count(list)) {
        return 0;
    }

    void* currentIndex  = List_at(list, index);
    void* nextIndex     = (void*)list->head;

    _list_copy(currentIndex, nextIndex, list->_typeSize);
    _List_get_prev(char, list, list->head);
    return 1;
}


_Bool List_remove_ordered(List* list, const size_t index) {
    if (index > List_count(list)) {
        return 0;
    }
    size_t listLength   = List_count(list);
    void* currentIndex  = List_at(list, index);
    void* nextIndex     = NULL;

    for (size_t i = index; i < listLength - 1; ++i) {
        nextIndex = List_at(list, i + 1);
        _list_copy(currentIndex, nextIndex, list->_typeSize);
        currentIndex = nextIndex;
    }

    _List_get_prev(char, list, list->head);
    return 1;
}

_Bool _List_push_back(List* list, void* data) {
    if (List_count(list) >= list->capacity - 1) {
        if (!List_realloc(list, list->capacity + (list->capacity << 1))) {
            return 0;
        }
    }
    _list_copy(list->head, data, list->_typeSize);
    _List_get_next(char, list, list->head);
    return 1;
}


_Bool _List_push_front(List* list, void* data) {
    if (List_count(list) >= list->capacity - 1) {
        if (!List_realloc(list, list->capacity + (list->capacity << 1))) {
            return 0;
        }
    }
    _List_get_prev(char, list, list->tail);
    _list_copy(list->tail, data, list->_typeSize);
    return 1;
}


_Bool List_pop_front(List* list, void* out) {
    if (list->head == list->tail) return 0;
    if (out) { _list_copy(out, list->tail, list->_typeSize); }
    _List_get_next(char, list, list->tail);
    return 1;
}


_Bool List_pop_back(List* list, void* out) {
    if (list->head == list->tail) return 0;
    _List_get_prev(char, list, list->head);
    if (out) { _list_copy(out, list->head, list->_typeSize); }
    return 1;
}


_Bool List_peak_front(List* list, void* out) {
    if (list->head == list->tail) return 0;
    _list_copy(out, list->tail, list->_typeSize);
    return 1;
}


_Bool List_peak_back(List* list, void* out) {
    if (list->head == list->tail) return 0;
    char* temp = list->head;
    _List_get_prev(char, list, temp);
    _list_copy(out, temp, list->_typeSize);
    return 1;
}


_Bool List_append(List* dst, List* src) {
    if (!dst || !src) return 0;

    /* leave early if the list contain different data "types" (size miss - match). */
    if (dst->_typeSize != src->_typeSize) return 0;

    size_t  sourceCount     = List_count(src);
    size_t  combinedCount   = List_count(dst) + sourceCount;

    if (combinedCount >= dst->capacity) {
        if (!List_realloc(dst, combinedCount)) {
            return 0;
        }
    }

    for (List_iterator(src, char)) {
        if (!_List_push_back(dst, (void*)it)) {
            return 0;
        }
    }
    return 1;
}

#endif
