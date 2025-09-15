# List
### Preface

This readme file and project is **unfinished**. I will add additional examples at a later date.
The version of list.h is partially untested and may not work. 

This work came from a graphics project as a low-level replacement for std::vector in c++. 
After using it in other projects and iterating its design, 
I feel that it has reached a point in maturity that it should be added as its own repository. 
The code in this project came from my [graphics project](https://github.com/E-Parker/C_Graphics). 
You can look there for usage examples.

### Performance information
Generally, this project was designed around 64 bit modern CPU architectures, 
with lots of cache and memory. If you are thinking of using this on an imbedded level I strongly 
suggest you either reconsider using this, or use it as a starting point but strip out anything 
dynamic about it.  

The list implementation internally uses a circular queue with supporting functions to reallocate 
and reorder the array. Indexing or iterating the list is generally not significantly more expensive 
than it would be for an array, only requiring a single branch for if the array is split across the 
buffer or not. The major slowdown you will see using this over an array, is in that the compiler 
cannot easily optimize array accesses since it's hidden behind a function call, 
plus that aforementioned function call.

# Requirements
This codebase relies on the c standard lib for malloc, calloc, and free. Besides this, 
it is largely platform independent. If you so wished, it wouldn't be difficult to replace those 
functions since they are the only three utilized. There is a macro for enabling the use of memcpy, 
"LIST_USE_MEMCPY". If this macro is defined, a definition for memcpy will also be required.  

# List of features
Other generic list behaviors supported are
- indexing
- iterating
- concatenation
- resizing
- automatic resizing
- item insert*
- item remove
- index remove
- initialize with value

The list also has functions to operate as a stack or queue. 
This can be done using the following functions
- Push to front
- Push to back
- Pop from front
- Pop from back
- Peak front
- Peak back

## Usage

All functions and macros are prefixed with "List". This is to avoid collisions with other 
generically named functions and to aid auto-complete.

## Creation and Destruction

Lists must be created and destroyed in a similar way to how you would malloc and free an array.
List_Create takes only two parameters. The "type" stored, and capacity. "type" can be any type, 
either user defined or built-in. 
Capacity determines the initial size of the internal buffer. 
**The list is still considered Empty until date is pushed into it.**  
```
// Create a list on the heap. Passing the "type" stored in the list, and the initial capacity. 
List* heapList = List_create(int, 4);
```
Note that list can also store pointers to objects. In this example, a function pointer is used.
```
// Create a list which contains function pointers.
List* functionList = List_create(void (*function)(), 4);
```
List can handle raw definitions and pointers, but it is generally cleaner and more clear to use 
typedef. 
```
// Create a list which contains typedefed function pointers.
typedef void typedef void (*Function_Void_NoParam)();
List* functionList = List_create(Function_Void_NoParam, 4);
```
Just as you would call free on an array, you ***must*** call List_destroy() on Lists when you are 
done using them. The list pointer is passed by reference and is set to NULL after being destroyed 
so that it isn't possible to end up with a use after free. 
```
// Pass in the pointer to the list by reference.
List_destroy(&heapList);
```
Creating Lists on the stack is also possible. However, because lists use heap memory internally, 
you still have to call a function at the end of it's scope. 
*You may also call these functions to initialize or deinitialize a list created on the heap.*  
```
// create a stack list. Passing the "type" stored in it, the List by reference, and it's capacity.
List stackList;
List_initialize(int, &stackList, 4);

// Destroy the list before it falls out of scope.
List_deinitialize(&stackList);
```
## Indexing & Iterating

### Indexing
Indexing a List is made very easy with List_at(). This function provides a reference to the item 
owned by the list, at the specified index. The function returns NULL if the index is out of bounds.
```
// Create a list on the heap. 
List* heapList = List_create(int, 4);

// Lists count their size by how many items have been pushed into the list.
// Even though we initialized to capacity 4, we still need data in the list.
List_push_front(heapList, 1);
List_push_front(heapList, 2);
List_push_front(heapList, 3);

// List_at returns a reference to the memory owned by the List.
// If the index is out of bounds, the returned value is NULL.
int* num;
num = (int*)List_at(heapList, 0);
```
### Iterating
The List_iterator(T, list) macro is the simplest way to iterate through the list. Use this macro in place 
of the normal definition you would have for a for loop. This mimics the behavior of a ForEach loop
you might have seen in other programming languages. The macro provides a parameter It of type T. 
```
// Create a list on the heap. 
List* heapList = List_create(int, 4);

// create a for loop where the iterator is type "int".
for List_iterator(int, heapList) {
    // do something in the loop:
    *it ...
}
```
Another way to iterate through the list is with the function List_create_array(). This creates a 
hard copy of the list, trimmed down to the count of the list. Iterating this way will not modify 
the original list.
```
// Create a list on the heap. 
List* heapList = List_create(char, 4);

List_push_front(heapList, 'h');
List_push_front(heapList, 'i');
List_push_front(heapList, '\0');

char* listAsArray;
listAsArray = (char*)List_create_array(heapList);

// do something with the array:
//for (unsigned int i = 0; i < List_count(heapList); i++) {
//    listAsArray[i] ...
//}

// char* otherArray = (char*)malloc(List_byte_count(heapList));
// memcpy(otherArray, listAsArray, List_byte_count(heapList))
// ... 

// Be sure to always free the array after use!
free(listAsArray);
```
On a related note to the above scenario, If you need direct access to the array you ***must*** 
always use List_reorder(list) beforehand. This function shuffles around the buffer such that it is in
correct order, the same as it would appear if you created a new buffer with List_create_array(list). 
**It is discouraged to do this. if you mishandle the buffer there may be undesired effects.** 
```
// Create a list on the heap. 
List* heapList = List_create(int, 8);

List_push_front(heapList, 1);
List_push_front(heapList, 2);
List_push_front(heapList, 3);
List_push_front(heapList, 4);

List_reorder(heapList);

// Now that the list has been reordered, we can access the data directly.
int* listArray = (int*)heapList->data;
```
## Manipulating data

There are several ways to manipulate the data stored in the list. Indexing the list and reordering 
have already been covered above.

### Pushing and Popping

Similarly to how a queue or stack would work, you can push data onto and pop data off of a list.
Resizing is handled automatically so if you ever push more elements than can be stored with the
current capacity, the List will request 1.5x the current capacity.
```
// Create a list on the heap. 
List* heapList = List_create(int, 4);

// push something onto the heap.
List_push_front(heapList, 1);
List_push_back(heapList, 2);

// peak the value that would be removed by a pop.
int val;
val = List_peak_front(heapList);
val = List_peak_back(heapList);

// pop values off the heap.
List_pop_front(heapList);
List_pop_back(heapList);
```

### Initializing

There is also the List_set(list, template, count) function for clearing and filling a list with
a set value. This is the preferred way to initialize a list if you plan on using it as an array.
This function does not resize the underlying array unless the count provided is greater than the 
current capacity.

```
// Create a list on the heap. 
List* heapList = List_create(int, 2);

// since 16 is greater than the initial capacity, it will resize to 1.5x 16, then fill 0 - 16 with 0.
List_set(heapList, 0, 16);
...
```
### Copying & Subsets
As mentioned above in [the iterating section](#iterating), there is a way to get a hard copy of the
data stored in a list using the List_create_array(list) function. However if you are looking to 
keep the data as a list, use the List_create_subset(list, start, end) function. With this you can 
create a hard copy of any slice of the list. If your start and end are not possible, the function 
will instead return NULL.

```
// Create a list on the heap. 
List* heapList = List_create(int, 2);

// Fill the list with data.
List_set(heapList, 0, 16);

// Create a copy.
List* heapListCopy = List_create_subset(heapList, 0, List_count(heapList) - 1);


// Remember that the original and copy must now be destroyed.
List_destroy(&heapList);
List_destroy(&heapListCopy);
```

### Concatenating & Appending

There is no function for concatenating lists, but the effect can be achieved with the 
List_append(src, dst) function. This function modifies the dst list, appending the contents of src
to it, resizing if necessary.
```
// Create a lists on the heap. 
List* listA = List_create(int, 2);
List* listB = List_create(int, 6);

// Fill the lists with data.
List_set(listA, 1, 2);
List_set(listB, 2, 2);

// Append ListA to listB, making ListAB.
List* ListAB = List_create_subset(ListA, 0, 1);
List_Append(listB, ListAB);
...
```
