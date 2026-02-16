# List
### Preface

This read-me file and project is **unfinished**. I will add additional examples at a later date.
Not all committed versions are guaranteed to work, This repository may change at any time.

This work came from a graphics project as a low-level replacement for std::vector in c++. 
After using it in other projects and iterating its design, I feel that it has reached a point in 
maturity that it should be added as its own repository. 

The code in this project originally came from my [OpenGL graphics project](https://github.com/E-Parker/C_Graphics), but since then I have reworked the majority of it to fit the needs of my Vulkan game engine (repo currently private).

### Performance information

This project was designed with 64 bit modern CPU architectures in mind. If you are thinking of 
using this on an embedded level I strongly suggest you either reconsider using this, or that you 
fork the project and remove all the dynamic reallocation functionality. This could work super well 
with just a fixed size buffer. The parts of the code with just stdlib functions can be found [here](#c-standard-library-usage).

The list implementation internally uses a circular queue with supporting functions to reallocate 
and reorder the array. Indexing or iterating the list is generally not significantly more expensive 
than it would be for an array, only requiring a single branch for if the array is split across the 
buffer or not. The major slowdown you will see using this over an array, is in that the compiler 
cannot easily optimize array accesses since it's hidden behind a function call, 
plus that aforementioned function call.

# Requirements

This code base relies on the C standard library for malloc, calloc, and free. Besides this, it is 
largely platform independent. If you so wished, it wouldn't be difficult to replace those functions 
since they are the only three utilized.

This code base also relies on stddef.h for size_t. To be honest, I'm just using this because it's a
pain to figure out what size_t should be just using the preprocessor, and I don't have access to a
bunch of obscure platforms to test this library on.

##### C Standard Library usage
For those who would like to remove the use of standard library functions, and replace it with their
own allocator: 
calloc is used in \_List\_create.
malloc is used in List\_reorder.
free is used in List\_destroy.

# List of features

Generic list behaviors supported are:
- indexing
- iterating
- concatenation
- resizing
- automatic resizing
- item insertion*
- item removal
- indexed removal
- ordered removal

The list also has functions to operate as a stack or queue, using the following functions:
- Push to front
- Push to back
- Pop from front
- Pop from back
- Peak front
- Peak back

Features which are missing:
- ordered insertion
- sorting

# Usage

All functions and macros are prefixed with "List". This is to avoid collisions with other 
generically named functions and to aid finding these functions with intellisense.

## Creation & Destruction

Lists must be created and destroyed in a similar way to how you would malloc and free an array.
List_Create takes only two parameters. The "type" stored, and capacity. "type" can be any type, 
either user defined or built-in. Capacity determines the initial size of the internal buffer. 
**Lists are empty until date is pushed into them.**  
```
// Create a new list. Passing the "type" stored in the list, and the initial capacity. 
List intList = List_create(int, 4);
```
Note that list can also store pointers to objects. In this example, a function pointer is used.
```
// Create a list which contains function pointers.
List functionList = List_create(void (*function)(), 4);
```
While Lists *can* handle raw definitions and pointers, but I'd recommend using a use typedef. 
```
// Create a list which contains typedefed function pointers.
typedef void typedef void (*Function_Void_NoParam)();
List functionList = List_create(Function_Void_NoParam, 4);
```
Just as you would call free on an array, you ***must*** call List_destroy() on Lists when you are 
done using them. This handles destroying the internal buffer, and setting references to it to NULL.
```
// Pass in the list by reference.
List_destroy(&intList);
```

## Manipulating data

There are several ways to manipulate the data stored in the list. This section covers:
[Initializing a list with values](#initializing).
[inserting values](#insertion).
[Pushing, peaking, and popping values](#pushing--popping).
[Copying and creating subsets](#copying--subsets).
[Concatenation and appending](#concatenating--appending).

Indexing and iterating is covered in the next section, [here](#indexing--iterating).

### Initializing

There is the List_set(list, T, value, count) function for clearing and filling a list with a value. 
This function will only resize the array if the count provided is greater than or equal too the 
current capacity - 1. *This is because if the array was completely full, the head and tail would be
pointing to the same address, and in this state the list is indistinguishable from an empty list. 

```
// Create a list on the heap. 
List* intList = List_create(int, 2);

// 16 is greater than the initial capacity, so it will resize to count + 1 and fill with values.
List_set(&intList, int, 0, 16);
...
```

#### passing values
The value passed in to either push function can be any source of the type, T. the Type T should 
always be the same type and same size as the type used to create the list. Doesn't matter if it's a 
constant, macro, or even the return value from a function call, it will work just fine.
```
...
static int getNumber () {
    return 3;
}
...
#define THE_NUMBER_FOUR 4
...
void main () {
    // Create a list. 
    List intList = List_create(int, 4);
    
    // Demoing all the valid uses of List_set.
    int five = 5;
    List_set(intList, int, 2, 16);                  // fill with a constant value.
    List_set(intList, int, getNumber(), 16);        // fill with a return value.
    List_set(intList, int, THE_NUMBER_FOUR, 16);    // fill with a macro value.
    List_set(intList, int, five, 16);               // fill with a variable value.
}

```

### Insertion

Values can be inserted into a list using the List_push_back() and List_push_front() functions. 
Values are handled the same way as in [the passing values section](#passing-values).
```
...
void main () {
    // Create a list. 
    List intList = List_create(int, 4);
    
    // Demoing all the valid uses of List_push_back (also applicable to List_push_front).
    int five = 5;
    List_push_back(intList, int, 2);                // push a constant value.
    List_push_back(intList, int, getNumber());      // push a return value.
    List_push_back(intList, int, THE_NUMBER_FOUR);  // push a macro value.
    List_push_back(intList, int, five);             // push a variable value.
}
```

### Pushing & Popping

Similarly to how a queue or stack would work, you can push data onto and pop data off of a list.
Resizing is handled automatically so if you ever push more elements than can be stored with the
current capacity, the List will request 1.5x the current capacity.
```
// Create a list. 
List intList = List_create(int, 4);

// push something onto the list.
List_push_front(intList, int, 1);
List_push_back(intList, int, 2);

// peak the value that would be removed by a pop.
int val;
List_peak_front(intList &val);
List_peak_back(intList &val);

// pop values off the list.
List_pop_front(intList, NULL);  
List_pop_back(intList, &val);
```

### Copying & Subsets

As mentioned above in [the iterating section](#iterating), there is a way to get a hard copy of the data stored 
in a list using the List_create_array(list) function. However if you are looking to keep the data 
as a list, use the List_create_subset(list, start, end) function. With this you can create a hard 
copy of any slice of the list. If your start and end are not possible, the function will instead 
return an invalid list (data pointing to NULL).

```
// Create a list on the heap. 
List intList = List_create(int, 2);

// Fill the list with data.
List_set(intList, int, 0, 16);

// Create a copy.
List intListCopy = List_create_subset(intList, 0, List_count(intList));

// Remember that both the original, and the copy must be destroyed.
List_destroy(&intList);
List_destroy(&intListCopy);
```

### Concatenating & Appending

There is no function for concatenating lists, however the effect can be achieved by copying the 
list, then joining the two lists together, use the List_append(src, dst) function. This function 
modifies the dst list, copying the contents of the src list into to it, resizing if necessary.
```
// Create a lists on the heap. 
List listA = List_create(int, 2);
List listB = List_create(int, 6);

// Fill the lists with data.
List_set(&listA, int, 1, 2);
List_set(&listB, int, 2, 2);

// Use the subset function to create a copy of ListA.
// then append listB onto listAB. This results in listAB being the concatenation of listA and listB.
List listAB = List_create_subset(&ListA, 0, List_count(&ListA));
List_Append(&listAB, &listB);
...
```

## Indexing & Iterating

### Indexing

Indexing a List is made very easy with List_at(). This function provides a reference to the item 
owned by the list, at the specified index. The function returns NULL if the index is out of bounds.
This reference is only valid until the list is modified with any of the other list functions. If 
you need persistent access to a reference in a list, its best to just store the index, and 
re-acquire the reference when you need it.

```
// Create a list. 
List list = List_create(int, 4);

// Lists count their size by how many items have been pushed into the list.
// Even though we initialized to capacity 4, we still need data in the list.
List_push_front(&list, int, 1);
List_push_front(&list, int, 2);
List_push_front(&list, int, 3);

// List_at returns a reference to the memory owned by the List.
// If the index is out of bounds, the returned value is NULL.
int* num;
num = (int*)List_at(list, 0);
```

### Iterating

The List_iterator(list, T) macro is the simplest way to iterate through the list. Use this macro 
in place of the normal definition you would have for a for loop. This mimics the behavior of a 
ForEach loop you might have seen in other programming languages. 
The macro provides an iterator, "it" which is of type T.
```
// Create a list on the heap. 
List list = List_create(int, 4);

// create a for loop where the iterator is type "int".
for List_iterator(list, int) {
    // do something in the loop:
    *it ...
}
```
Another way to iterate through the list is with the function List_copy_to_array(). This creates a 
hard copy of the list. Iterating this way will not modify the original list.
```
// Create a list of chars.
List* charList = List_create(char, 4);

List_push_front(&charList, char, 'h');
List_push_front(&charList, char, 'i');
List_push_front(&charList, char, '\0');

// Create an array on the heap
char* listArray = (char*)malloc(4);
List_create_array(&charList, listArray);

// do something with the array:
//for (unsigned int i = 0; i < List_count(charList); i++) {
//    listArray[i] ...
//}

// char* otherArray = (char*)malloc(List_byte_count(&charList));
// memcpy(otherArray, listArray, List_byte_count(&charList))
// ... 

// Be sure to always free a heap array after use!
free(listArray);
free(otherArray);
```
On a related note to the above scenario, If you need direct access to the list's internal buffer, 
you ***must*** always use List_reorder(&list) beforehand. This function shuffles around the buffer 
such that it is in correct order (tail at the start of the list, head at it's end.), the same as 
it would appear if you copied the list into an array using List_copy_to_array().
**It is discouraged to do this. if you mishandle the buffer there may be undesired effects.** 
```
// Create a list on the heap. 
List intList = List_create(int, 8);

List_push_front(intList, int 1);
List_push_front(intList, int 2);
List_push_front(intList, int 3);
List_push_front(intList, int 4);

List_reorder(intList);

// Now that the list has been reordered, we can access the data directly.
int* listArray = (int*)intList->data;
```
Keep in mind, that if we modify the List using any of the list functions, the array we access may 
be invalid. As a rule of thumb, **Always** use List_reorder(&list) to ensure the buffer is 
correctly aligned.
```
...
List_push_front(intList, int 1);
List_push_back(intList, int 8);
...
// Since we accessed the list, the array might be invalid. reorder, and get the pointer again.
List_reorder(intList);
listArray = (int*)intList->data;
```

