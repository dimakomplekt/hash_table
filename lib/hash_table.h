// =========================================================================================== INFO
// UNIVERSAL HASH TABLE
// A universal hash table implementation in C.
//
// Supports key types: int, float, string.
// Values can be of any type (via void* + value_size).
// Hashing is handled separately for each key type.
// Average time complexity for insertion, deletion, and search is O(1).
//
// NOTE: This library requires a C11-compliant compiler.
// =========================================================================================== INFO




#ifndef HASH_TABLE
#define HASH_TABLE


// =========================================================================================== IMPORT
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include <stddef.h> // size_t type import
#include <stdarg.h> // For function with optional argumenst quantity at the call (variadic)

#include <stdio.h>
#include <stdlib.h>

// =========================================================================================== IMPORT


#ifdef __cplusplus
extern "C" {
#endif


// =========================================================================================== KEY TYPES

// Pass method for the values
typedef enum {

    PASS_BY_REFERENCE,     // Link on outer element
    PASS_BY_COPY           // Local copy

} pass_method;

// Macros for the value pass method check
#define pass_method_check(curr_pass_method) \
    ((curr_pass_method == PASS_BY_REFERENCE) || (curr_pass_method == PASS_BY_COPY))


// Key types enum
// Add the new types at the end of the enum, check macros and ht_key struct (but u will need to add the new hashing methods too XD)
typedef enum {

    HT_INT_KEY,            // int-type key
    HT_STR_KEY,            // str-type key (const char*)

} key_type;

// Macros for the key type check
#define key_type_check(curr_key_type) \
    ((curr_key_type == HT_INT_KEY) || (curr_key_type == HT_STR_KEY))
// =========================================================================================== KEY TYPES


// =========================================================================================== VALUE TYPES

// Value types enum
// Add the new types at the end of the enum, check macros and ht_value struct!
typedef enum {

    HT_U_INT8_VAL,        // uint8_t
    HT_U_INT16_VAL,       // uint16_t
    HT_U_INT32_VAL,       // uint32_t
    HT_U_INT64_VAL,       // uint64_t
    HT_INT8_VAL,          // int8_t
    HT_INT16_VAL,         // int16_t
    HT_INT32_VAL,         // int32_t
    HT_INT64_VAL,         // int64_t
    HT_U_SHORT_VAL,       // unsigned short int
    HT_SHORT_VAL,         // short int
    HT_U_INT_VAL,         // unsigned int
    HT_INT_VAL,           // int
    HT_U_LONG_VAL,        // unsigned long
    HT_LONG_VAL,          // long
    HT_U_LONG_LONG_VAL,   // unsigned long long
    HT_LONG_LONG_VAL,     // long long
    HT_FLOAT_VAL,         // float
    HT_DOUBLE_VAL,        // double
    HT_LONG_DOUBLE_VAL,   // long double
    HT_CHAR_VAL,          // char
    HT_STR_VAL,           // const char* (string)
    HT_ARRAY_VAL          // any type massive (pointer to the void (void*) + size of the massive)

} value_type;

// Macros for the value type check
#define value_type_check(curr_value_type) \
    ((curr_value_type) >= HT_U_INT8_VAL && (curr_value_type) <= HT_ARRAY_VAL)

// =========================================================================================== VALUE TYPES



// =========================================================================================== KEY STRUCT

typedef struct
{

    key_type type;                 // key type

    union
    {

        int as_int;                // data-storage for int-type key
        const char* as_str;        // data-storage for str-type key

    } data;
    
} ht_key;

// =========================================================================================== KEY STRUCT


// =========================================================================================== VALUE STRUCT

typedef struct
{

    value_type type;                     // Value type
    pass_method curr_pass_method;        // Value pass method

    // Union for the types from value_type enum
    union
    {
        // Main types
        uint8_t             as_uint8;
        uint16_t            as_uint16;
        uint32_t            as_uint32;
        uint64_t            as_uint64;

        int8_t              as_int8;
        int16_t             as_int16;
        int32_t             as_int32;
        int64_t             as_int64;

        unsigned short      as_ushort;
        short               as_short;
        unsigned int        as_uint;
        int                 as_int;
        unsigned long       as_ulong;
        long                as_long;
        unsigned long long  as_ulong_long;
        long long           as_long_long;

        float               as_float;
        double              as_double;
        long double         as_long_double;

        char                as_char;

        const char*         as_str;        // Strings

        void*               as_ptr;        // Arrays, structs, outer data by the pointer

    } data;

    size_t data_size;

} ht_value;

// =========================================================================================== VALUE STRUCT


// =========================================================================================== KEY-VALUE pairs

typedef struct {

    ht_key key;                   // Current key

    ht_value value;               // Current value

    unsigned int occupied;        // Flag - 0 is empty, 1 - is engaged

} ht_element;


// =========================================================================================== KEY-VALUE pairs


// =========================================================================================== HASH_TABLE

typedef struct
{
    
    ht_element* elements;               // Dynamic massive with every element of the table

    size_t capacity;                    // Capacity of the table
    size_t size;                        // Quantity of the engaged table cells

    double max_load_factor;             // Max load   factor - if size / capacity > max_load_factor — do resize()

    size_t first_element_hash_index;    // Hash index of the first element
    size_t last_element_hash_index;     // Hash index of the last element

} hash_table;


// =========================================================================================== HASH_TABLE


// =========================================================================================== PUBLIC API


// Function for hash-table creation (varargs!).
// Creates the table with initialized capacity
hash_table* ht_create();


// Table delete and memory clean function
// Delete the table (DANGLING POINTER, NEED NEW INITIALIZATION!)
// Call as: ht_delete(&table_1);
void ht_delete(hash_table** table);


// Table memory clean (elements delete) function
// Clear the data, don't touch the table pointer (NO DANGLING POINTER)
// Call as: ht_clear(&table_1);
void ht_clear(hash_table** table);



// Table element memory clean (1 element delete) function
// Call as: ht_clear_item(&table_1, key);
void ht_clear_item(hash_table** table, key);



// =========================================================================================== ADDER AND ADDER DATA

/*
 * 1. The Problem:
 *    - C and C++ cannot automatically convert arbitrary expressions into
 *      structures when passing them as function arguments.
 *    - For example, you can't just pass "10" and have the compiler wrap it
 *      into a struct with a pointer and size automatically.
 *
 * 2. Variadic functions:
 *    - Functions like ht_add_va(hash_table**, size_t, ...) can accept any
 *      number of arguments, but the compiler doesn't know the type of each
 *      argument beyond standard promotions (int -> int, float -> double, etc.).
 *    - You can't automatically construct a struct from each argument inside
 *      the function unless you tell the compiler exactly what type each
 *      argument has.
 *
 * 3. _Generic in C11:
 *    - _Generic allows compile-time type selection.
 *    - We can use it to detect whether an argument is int, float, or char*
 *      and then create the appropriate ht_key or ht_value struct.
 *    - Example:
 *        #define MAKE_KEY(x) _Generic((x), \
 *            int:    (ht_key){ HT_INT_KEY,   .data.as_int = (x) }, \
 *            float:  (ht_key){ HT_FLOAT_KEY, .data.as_float = (x) }, \
 *            char*:  (ht_key){ HT_STR_KEY,   .data.as_str = (x) }, \
 *            const char*: (ht_key){ HT_STR_KEY, .data.as_str = (x) } \
 *        )
 *
 * 4. COUNT_ARGS macro:
 *    - Variadic macros (__VA_ARGS__) let us pass multiple arguments.
 *    - COUNT_ARGS(...) computes the number of arguments by creating a
 *      temporary void* array and dividing its total size by the size of
 *      one element.
 *    - This allows ht_add() to figure out how many key-value pairs were passed.
 *
 * 5. Wrapping with a macro:
 *    - ht_add(table, ...) expands to:
 *        ht_add_va(&table, COUNT_ARGS(__VA_ARGS__), __VA_ARGS__)
 *    - This macro automatically:
 *        a) Takes the table variable.
 *        b) Counts arguments.
 *        c) Passes everything into ht_add_va.
 *    - Inside ht_add_va, each key-value argument is converted using:
 *        ht_key key = MAKE_KEY(raw_key);
 *        ht_value value = HT_VAL(raw_value);
 *
 * 6. HT_VAL(x):
 *    - Wraps any value into a struct containing:
 *        - ptr -> pointer to the value
 *        - size -> size of the value in bytes
 *    - This is necessary because C cannot automatically infer &x + sizeof(x)
 *      inside a function; it must be done at the call site (macro does that).
 *
 * 7. Why this "works around" language limitations:
 *    - By using a macro at the call site, we convert plain expressions
 *      into the exact structures required by the internal API.
 *    - _Generic ensures the correct key type.
 *    - HT_VAL ensures the correct value pointer and size.
 *    - ht_add_va receives uniform structs and can insert them without
 *      caring about the original types.
 *
 * 8. Result:
 *    - User can write:
 *        ht_add(table,
 *            "apple", 10,
 *            "banana", 20
 *        );
 *    - Macro + _Generic automatically convert this to:
 *        ht_add_va(&table, 4,
 *            "apple", HT_VAL(10),
 *            "banana", HT_VAL(20)
 *        );
 *    - ht_add_va then converts raw_key and raw_value into ht_key and ht_value
 *      and calls ht_add_pair, which does the real insertion.
 *
 * 9. Key Takeaways:
 *    - C requires explicit handling of types and sizes when using generic
 *      containers.
 *    - Macros combined with _Generic allow creating a nicer, type-safe
 *      API without writing boilerplate for each call.
 *    - This is why we don't need to manually construct ht_key and ht_value
 *      at every call; the macro does it for us.
 */


// Struct for adder macro, which exist to automate the value size pass to the adder function
typedef struct {

    void* ptr; // Value pointer
    size_t size; // Value size variable

} ht_value;


// Macro for the automatic key-creation
#define MAKE_KEY(x) _Generic((x), \
    int:    (ht_key){ HT_INT_KEY,   .data.as_int = (x) }, \
    char*:  (ht_key){ HT_STR_KEY,   .data.as_str = (x) }, \
    const char*: (ht_key){ HT_STR_KEY, .data.as_str = (x) } \
)


// Macro for the automatic ht_value struct creation
#define HT_VAL(x) ((ht_value){ .ptr = &(x), .size = sizeof(x) })


/*
 * HT_ARG_COUNT - macro to count the number of arguments passed to it
 * @...: any number of arguments (keys and values for hash table)
 *
 * This macro uses a helper macro HT_ARG_COUNT_IMPL with a fixed number of
 * parameters to "catch" the last argument as N. By appending numbers in
 * descending order, we can determine the total number of arguments
 * passed to HT_ARG_COUNT.
 *
 * Example:
 *   HT_ARG_COUNT(a, b, c, d) -> 4
 *
 * Notes:
 * - Each key/value pair counts as two arguments.
 * - This approach works reliably with any type of arguments (int, float, char*),
 *   unlike the old method using sizeof(void*[]), which was unsafe for non-pointer types.
 * - Useful for variadic macros/functions to determine the number of arguments
 *   before forwarding them to ht_add_va.
 *
 * Usage in ht_add macro:
 *   ht_add(table,
 *       key1, val1,
 *       key2, val2,
 *       ...
 *   );
 *   The macro automatically counts the total number of arguments and passes
 *   it to ht_add_va.
 */
#define HT_ARG_COUNT_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,N,...) N
#define HT_ARG_COUNT(...) HT_ARG_COUNT_IMPL(__VA_ARGS__,10,9,8,7,6,5,4,3,2,1,0)


/* ht_add - main macro for convenient add/set function syntax
 *
 * Accepts key/value pairs as variadic arguments.
 * Automatically counts the number of arguments using HT_ARG_COUNT
 * and forwards them to ht_add_va with a pointer to the table.
 *
 * Example:
 *   ht_add(table,
 *       1, val1,
 *       2, val2,
 *       "apple", 10,
 *       "banana", 20
 *   );
 */


 // Real adder function, which uses by the main macro ht_add()
// Expects the pairs as arguments
void ht_add_elements(hash_table** table, size_t elements_quantity, ...);


#define ht_add(table, ...) ht_add_va(&(table), HT_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

// =========================================================================================== ADDER AND ADDER DATA


// =========================================================================================== PUBLIC API

#ifdef __cplusplus
}
#endif

#endif /* HASH_TABLE */