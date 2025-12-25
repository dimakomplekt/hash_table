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


// =========================================================================================== IMPORT
#include <assert.h>
#include "hash_table.h"

// =========================================================================================== IMPORT


// =========================================================================================== DEFINES

// Default initial table capacity
// !!! SHOULD BE VALUE^2 !!!
#define HT_INITIAL_CAPACITY 16

// Increase capacity (capacity * 2) after this load factor value
#define HT_MAX_DEFAULT_LOAD_FACTOR 0.75
// Decrease capacity (capacity / 2) after this load factor value
#define HT_MIN_DEFAULT_LOAD_FACTOR 0.25

// =========================================================================================== DEFINES


// =========================================================================================== FUNCTIONS DEFINITIONS

bool check_ht_pair_input(void* key, key_type key_t, void* value, value_type val_t);

bool ht_capacity_check(hash_table* table, capacity_check_call_type check_call_type);
void ht_resize(hash_table* table);

size_t hash_the_key(void* key, key_type curr_key_type);
static size_t hash_int(int key, size_t capacity);
static size_t hash_str(const char* key_string, size_t capacity);

bool check_ht_pair_input(void* key, key_type key_t, void* value, value_type val_t);

bool boundary_index_check (hash_table* table, size_t new_hashed_index);

// =========================================================================================== FUNCTIONS DEFINITIONS



// =========================================================================================== HASH TABLE INITIALIZATION

// ht_create() function realization

/*
 * ht_create() - create and initialize a new hash table
 * @k_type: expected key type (HT_INT_VAL, HT_FLOAT_VAL, HT_STR_VAL, HT_ARRAY_VAL)
 * @v_type: expected value type (HT_INT_VAL, HT_FLOAT_VAL, HT_STR_VAL, HT_ARRAY_VAL)
 * @...: optional arguments (element.g., array size for HT_ARRAY_VAL)
 *
 * This function allocates and initializes a new hash table structure.
 * The table is allocated dynamically and must be freed using ht_free().
 *
 * The function performs the following steps:
 *  1. Allocates memory for the hash_table structure.
 *  2. Validates the provided key and value types.
 *  3. Initializes capacity, size, and load factor.
 *  4. Allocates memory for the element array using calloc().
 *     calloc() ensures all fields are zeroed (occupied = 0, etc.).
 *  5. Optionally reads additional parameters (element.g. array size).
 *
 * Error handling:
 *  - If memory allocation fails, returns NULL.
 *  - If capacity is zero or overflow is detected, returns NULL.
 *  - If invalid key/value types are provided, returns NULL.
 *  - If array size is zero, prints a warning but still continues.
 *
 * Returns:
 *  Pointer to the allocated hash_table on success,
 *  or NULL on failure.
 */
hash_table* ht_create()
{
    // Allocate memory for the table
    // It's possible to allocate the memory for the whole table, because we storage
    // only the 3 values and 1 pointer on the elements massive.
    // Table allocated via malloc because all fields are explicitly initialized
    hash_table* table = malloc(sizeof(*table));

    // Allocation check
    if (!table)
    {
        fprintf(stderr, "[ERROR] Failed to allocate %zu bytes for hash_table\n", sizeof(hash_table));
        return NULL;
    }

    // Initialize fields
    table->capacity = HT_INITIAL_CAPACITY;
    table->size = 0;
    table->max_load_factor = HT_MAX_DEFAULT_LOAD_FACTOR;

    // Sanity checks
    if (table->capacity == 0 || table->capacity > SIZE_MAX / sizeof(ht_element))
    {
        fprintf(stderr, "[ERROR] Invalid capacity: %zu\n", table->capacity);
        free(table);
        return NULL;
    }

    // Allocate elements - autofill elements array with element structs
    // with NULL or 0 values for all struct variables (null by calloc)
    table->elements = calloc(table->capacity, sizeof(ht_element));

    if (!table->elements)
    {
        fprintf(stderr, "[ERROR] Failed to allocate %zu elements\n", table->capacity);
        free(table);
        return NULL;
    }

    return table;
}

// =========================================================================================== HASH TABLE INITIALIZATION


// =========================================================================================== ELEMENTS ADDING


// Insert helper-function
/*
Основная функция записи пар в таблицу которая принимает адрес таблицы
Ключ и значение в виде универсального пойнтера
их типы для дешифрации, способ передачи значений


*/

// Helper-logic enum for check_new_element_hash();
typedef enum {

    NEW_PAIR_NEW_HASH,
    NEW_PAIR_REPEATED_HASH,
    OLD_PAIR_NEW_VALUE,

} hash_check_status;


// Pass the values by the adress
bool ht_insert(hash_table* table,
    void* key, key_type curr_key_type, 
    void* value, value_type curr_value_type, pass_method value_pass_method)
{
    // Not initialized table error handler
    if (!table)
    {
        fprintf(stderr, "[ERROR] NULL hash_table pointer in ht_insert()\n");
        return false;
    }

    // Arguments pass error handler
    if(!check_ht_pair_input(key, curr_key_type, value, curr_value_type))
    {
        fprintf(stderr, "[ERROR] Invalid arguments in ht_insert()\n");
        return false;
    }

    // Current hash_table by link
    hash_table curr_ht = *table;

    // Check if there is enough capacity in the table after +1 element, resize if needed
    ht_capacity_check(table, ADD);

    // Hashed key value variable
    size_t tmp_hashed_key;

    // Hash key check status (by the hash_check_status enum)

    hash_check_status curr_hash_key_check_status; // Current value for logic
    hash_check_status tmp_hash_check_status; // Value temporary storage for iterative status check with linear hash probing
    
    // Flag for write data by index, which equal to current hash key value (not temped)
    bool val_write_permission = false;

    // Hash the key to get the initial hash index
    tmp_hashed_key = hash_the_key(key, curr_key_type) & (curr_ht.capacity - 1);

    if (!tmp_hashed_key)
    {
        printf(stderr, "[ERROR] Hash operation error in ht_insert()\n");\
        return false;
    }

    // Recursive status check (by hash_check_status), rehash and resize, until the obtaining of the correct insert
    // option conditions
    while (!val_write_permission)
    {
        curr_hash_key_check_status = check_new_element_hash(table, tmp_hashed_key, key, curr_key_type);
        
        // Error handler
        if (curr_hash_key_check_status != NEW_PAIR_NEW_HASH &&
            curr_hash_key_check_status != NEW_PAIR_REPEATED_HASH &&
            curr_hash_key_check_status != OLD_PAIR_NEW_VALUE)
        {   
            printf("[ERROR] Hash check status error in ht_insert()\n");
            return false;
        }
        


        // Actions for the key write with different hash check results
        switch (curr_hash_key_check_status)
        {   
            case NEW_PAIR_NEW_HASH:

                // Check if the current index is last or first
                if (!boundary_index_check(table, tmp_hashed_key))
                {
                    printf("[ERROR] Hash index boundary check failed in ht_insert()\n");
                    return false;
                }
                // Set the permission for value write
                // We've got the capacity, obtained good hash, checked the current add status and checked the boarders 
                else val_write_permission = true;

                break;


            case NEW_PAIR_REPEATED_HASH:

                tmp_hash_check_status = NEW_PAIR_REPEATED_HASH;
                size_t tmp_probing_hashed_key = tmp_hashed_key;

                while (tmp_hash_check_status != NEW_PAIR_REPEATED_HASH)
                {
                    // Linear hash intersection pass
                    
                    // TODO: LINEAR PROBING


                    tmp_probing_hashed_key = linear_prb(table, curr_key_type, tmp_probing_hashed_key);
                    // New hash check
                    tmp_hash_check_status = check_new_element_hash(table, tmp_probing_hashed_key, key, curr_key_type);
                }

                // Update the check status and hash value after linear probing
                curr_hash_key_check_status = tmp_hash_check_status;
                tmp_hashed_key = tmp_probing_hashed_key;

                break;


            case OLD_PAIR_NEW_VALUE:
                
                // Set the permission for value write
                val_write_permission = true;

                break;


            default:
                break;
        }
    }


    if (val_write_permission)
    {
        // Key write operation

        curr_ht.elements[tmp_hashed_key].hash_index = tmp_hashed_key;

        curr_ht.size += 1;

        // Write the key data type
        curr_ht.elements[tmp_hashed_key].key.type = curr_key_type;

        // Write the key value in dependence of the key type
        switch (curr_key_type)
        {
            case HT_INT_KEY:
                curr_ht.elements[tmp_hashed_key]->key.data.as_int = (int)(*key);
                break;

            case HT_STR_KEY:
                curr_ht.elements[tmp_hashed_key].key.data.as_str = (const char*)(*key);
                break;
            
            default:
                break;
        }

        // Value type
        curr_ht.elements[tmp_hashed_key]->value.type = curr_value_type;

        // Value write method
        curr_ht.elements[tmp_hashed_key]->value.curr_pass_method = value_pass_method;

        // Value and value size write by value type
        switch (curr_value_type)
        {
        case HT_U_INT8_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_uint8 = (uint8_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(uint8_t);

            break;

        case HT_U_INT16_VAL:
        
            curr_ht.elements[tmp_hashed_key].value.data.as_uint16 = (uint16_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(uint16_t);

            break;

        case HT_U_INT32_VAL:
        
            curr_ht.elements[tmp_hashed_key].value.data.as_uint32 = (uint32_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(uint32_t);

            break;

        case HT_U_INT64_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_uint64 = (uint64_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(uint64_t);

            break;

        case HT_INT8_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_int8 = (int8_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(int8_t);

            break;

        case HT_INT16_VAL:
        
            curr_ht.elements[tmp_hashed_key].value.data.as_int16 = (int16_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(int16_t);

            break;

        case HT_INT32_VAL:
        
            curr_ht.elements[tmp_hashed_key].value.data.as_int32 = (int32_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(int32_t);

            break;

        case HT_INT64_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_int64 = (int64_t)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(int64_t);

            break;     
            
        case HT_U_SHORT_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_ushort = (unsigned short)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(unsigned short);

            break;   

        case HT_SHORT_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_short = (short)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(short);

            break;               


        case HT_U_INT_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_uint = (unsigned int)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(unsigned int);

            break;  
            
        case HT_INT_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_int = (int)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(int);

            break;  

        case HT_U_LONG_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_ulong = (unsigned long)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(unsigned long);

            break;  
            
        case HT_LONG_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_long = (long)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(long);

            break;  

        case HT_U_LONG_LONG_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_ulong_long = (unsigned long long)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(unsigned long long);

            break;  
            
        case HT_LONG_LONG_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_long_long = (long long)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(long long);

            break;              

            
        case HT_FLOAT_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_float = (float)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(float);

            break;  

        case HT_DOUBLE_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_double = (double)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(double);

            break;  
            
        case HT_LONG_DOUBLE_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_long_double = (long double)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(long double);

            break;  

            
        case HT_CHAR_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_char = (char)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(char);

            break;  

        case HT_STR_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_str = (const char*)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(const char*);

            break;  
            
        case HT_ARRAY_VAL:

            curr_ht.elements[tmp_hashed_key].value.data.as_ptr = (void*)(*value);
            curr_ht.elements[tmp_hashed_key].value.data_size = sizeof(void*);

            break;  
            
            
        default:
            break;
        }
    }


    // РЕсайз по чеку отношений между size, capacity max_load_factor если нужно
}




// Helper-function for the user input post-pass check (VALUES WERE CHECKED BEFORE PASS)
bool check_ht_pair_input(void* key, key_type key_t, void* value, value_type val_t)
{
    // Types check
    if (!key_type_check(key_t))
    {
        fprintf(stderr, "[ERROR] Invalid key_type argument in ht_insert()\n");
        return false;
    }
    else if (!value_type_check(val_t))
    {
        fprintf(stderr, "[ERROR] Invalid value_type argument in ht_insert()\n");
        return false;
    }
    // Values check for NULLPTR after types check permission
    else
    {
        if (!key)
        {
            fprintf(stderr, "[ERROR] NULL key pointer in ht_insert()\n");
            return false;
        }
        if (!value)
        {
            fprintf(stderr, "[ERROR] NULL value pointer in ht_insert()\n");
            return false;
        }
    }

    return true;
}

// =========================================================================================== ELEMENTS ADDING

// =========================================================================================== HASH CREATE FUNCTIONS

/*
 * Integer hash function
 * Uses Knuth's multiplicative method:
 *   hash = key * 2654435761
 * Magic number 2654435761 = floor(2^32 / golden_ratio)
 * Helps to uniformly distribute keys over hash table buckets.
 * // https://stackoverflow.com/questions/11871245/knuth-multiplicative-hash#:~:text=Knuth%20multiplicative%20hash%20is,knuth)%20%3E%3E%20(32%20%2D%20p)%3B%0A%7D
 */

static size_t hash_int(int key)
{
    const uint32_t MULTIPLIER = 2654435769u; // Knuth multiplicative constant
    uint32_t k = (uint32_t)key;

    return (k * MULTIPLIER);
}

/*
 * String hash function
 * Uses the DJB2 algorithm (Dan Bernstein)
 * Magic number 5381, multiply-then-add approach:
 * hash = hash * 33 + c
 * Provides good distribution for typical string sets
 * https://stackoverflow.com/questions/1579721/why-are-5381-and-33-so-important-in-the-djb2-algorithm
 */

static size_t hash_str(const char* key_string)
{
    size_t hash = 5381;

    while (*key_string)
        hash = ((hash << 5) + hash) + (unsigned char)(*key_string++);

    return hash; // Or: hash % capacity
}


size_t hash_the_key(void* key, key_type curr_key_type)
{
    size_t hashed_key;

    // Hash the key 
    switch (curr_key_type)
    {
        case HT_INT_KEY:
            // Calculate the hashed key for int type key 
            hashed_key = hash_int(*(int*)key);
            break;

        case HT_STR_KEY:
            // Calculate the hashed key for str type key 
            hashed_key = hash_str(*(char**)key);
            break;

        default:
            fprintf(stderr, "Unknown key type\n");
            hashed_key = 0;
    }

    return hashed_key;
}

// =========================================================================================== HASH CREATE FUNCTIONS


// =========================================================================================== INSERT STATUS CHECK

// Function for the check of the element with same hash existence 
hash_check_status check_new_element_hash(hash_table* table, size_t current_hash, void* key, key_type curr_key_type)
{
    // Hash with capacity shift
    current_hash = current_hash & (table->capacity - 1);

    // Checked element
    ht_element* curr_element = &table->elements[current_hash];

    // Return the NEW_PAIR_NEW_HASH if the cell is empty
    if (!curr_element->occupied) return NEW_PAIR_NEW_HASH;

    // Pair with same hashed key exists
    else
    {
        // Check the table current hash and write the instruction number from hash_check_status
        // into current_hash_check_status 
        switch (curr_key_type)
        {
            case HT_INT_KEY:
            {
                // Pointer сast
                int* int_key_ptr = (int*)key;

                // Check the coincidence of the key values (key names are equal)
                if (curr_element->key.data.as_int == *int_key_ptr) return OLD_PAIR_NEW_VALUE;

                else return NEW_PAIR_REPEATED_HASH;
            }
    
            case HT_STR_KEY:
            {
                // Pointer cast
                const char* str_key_ptr = (const char*)key;

                if (curr_element->key.data.as_str &&
                strcmp(curr_element->key.data.as_str, str_key_ptr) == 0) return OLD_PAIR_NEW_VALUE;
                
                else return NEW_PAIR_REPEATED_HASH;
            }
    
            default:
                // invalid key type error handler
                assert(!"Invalid key_type in check_new_element_hash()");
                return NEW_PAIR_NEW_HASH;  // fallback - this line can't perform
        }
    }
}

// =========================================================================================== INSERT STATUS CHECK


// =========================================================================================== HT BOUNDARIES CHECK

bool boundary_index_check (hash_table* table, size_t new_hashed_index)
{
    hash_table curr_ht = *table; 

    if (new_hashed_index >= curr_ht.capacity)
    {
        fprintf(stderr, "[ERROR[ Hashed index is out of table range]]");
        return false;
    }

    if (curr_ht.size == 0)
    {
        curr_ht.first_element_hash_index = new_hashed_index; 
        curr_ht.last_element_hash_index = new_hashed_index;
    }

    if (curr_ht.size > 0 && new_hashed_index < curr_ht.first_element_hash_index)
    {
        curr_ht.first_element_hash_index = new_hashed_index; 
    }
    if (curr_ht.size > 0 && new_hashed_index > curr_ht.last_element_hash_index)
    {
        curr_ht.last_element_hash_index = new_hashed_index;
    }
}

// =========================================================================================== HT BOUNDARIES CHECK


// =========================================================================================== CAPACITY CHECK / RESIZE + REHASH

// 2 different variants of the capacity check logic
typedef enum {

    ADD,
    DELETE

} capacity_check_call_type;

// Function: ht_capacity_check()
// Helper-function for ht_add, which calls for every add / delete operation and checks
// if we need to resize the table.
bool ht_capacity_check(hash_table* table, capacity_check_call_type check_call_type)
{
    // Define an adder value by the function call type
    double adder;

    if (check_call_type == ADD) adder = 1.0;
    else if (check_call_type == DELETE) adder = -1.0;
    else
    {
        fnprintf(stderr, "ERROR, unknown check call type in ht_capacity_check()");
        return false;
    }

    // Check the load factor and resize if needed
    double load_factor_with_new_element = ((double)table->size + adder) / table->capacity;

    if (load_factor_with_new_element > HT_MAX_DEFAULT_LOAD_FACTOR ||
        load_factor_with_new_element < HT_MIN_DEFAULT_LOAD_FACTOR)
    {
        resize_ht(table, check_call_type);
    }

    return true;
}


// Doubles the capacity of the hash table and rehashes all elements
// Automatic call with the load factor overflow
// DON'T CALL ASYNC WITH FREE OR DELETE FUNCTIONS!
void resize_ht(hash_table* table, capacity_check_call_type check_call_type)
{
    if (!table) return;

    if (!table->elements)
    {
        fprintf(stderr, "Error: no data in the resized table!\n");
        return;
    }

    // Old capacity and elements
    size_t old_capacity = table->capacity;
    ht_element* old_elements = table->elements;

    if (check_call_type == ADD && old_capacity > SIZE_MAX / 2)
    {
        fprintf(stderr, "Resize skip: hash table too big to resize\n");
        return;
    }
    else if (check_call_type == DELETE && old_capacity <= HT_INITIAL_CAPACITY)
    {
        fprintf(stderr, "Resize skip: hash table too small to resize\n");
        return;
    }


    // New capacity and elements
    size_t new_capacity;

    switch (check_call_type)
    {
        case ADD:

            new_capacity = old_capacity * 2;
            break;

        case DELETE:

            new_capacity = old_capacity / 2;
            break;
        
        default:
            break;
    }
    
    // Allocate the memory for the new elements
    ht_element* new_elements = (ht_element*)calloc(new_capacity, sizeof(ht_element));

    // Allocation error handler
    if (!new_elements)
    {
        fprintf(stderr, "Error: failed to allocate memory for resized hash table\n");
        return;
    }


    // Rehash

    for (size_t i = 0; i < old_capacity; i++)
    {
        if (old_elements[i].occupied)
        {
            ht_element* old_element = &old_elements[i];

            size_t new_index;

            // Calculate new hash-index, based on new capacity
            // Rehash the key

            switch (old_element->key.type)
            {
                case HT_INT_KEY:

                    new_index = hash_the_key(&old_element->key.data.as_int, old_element->key.type) & (new_capacity - 1);
                    break;

                case HT_STR_KEY:

                    new_index = hash_the_key(&old_element->key.data.as_str, old_element->key.type) & (new_capacity - 1);
                    break;
                
                default:
                    break;
            }

            // Linear probing for collisions in new table
            while (new_elements[new_index].occupied) new_index = (new_index + 1) & (new_capacity - 1);

            // Move the new element to the new table
            ht_element* new_element = &new_elements[new_index];
            
            // Mark new cell as occupied
            new_element->occupied = 1;
            
            // Value copy from tmp to the cell
            new_element->value = old_element->value;
            
            // Key copy from tmp to the cell

            // Ordinary copy for int or float
            if (old_element->key.type != HT_STR_KEY) new_element->key = old_element->key;

            // Copy for string - hard logic cause the string is a dynamic memory pointer 
            else
            {
                new_element->key.type = HT_STR_KEY;
                new_element->key.data.as_str = strdup(old_element->key.data.as_str);

                if (!new_element->key.data.as_str)
                {
                    fprintf(stderr, "Error: failed to duplicate string key during resize\n");
                    exit(1);
                }

            }
        }
    }

    // Free old array (not the value - they've been removed)
    free(old_elements);


    // Update the capacity
    table->capacity = new_capacity;
    // Update table to new array
    table->elements = new_elements;
}

// =========================================================================================== RESIZE + REHASH


// =========================================================================================== VALUE PASS FUNCTIONS



// =========================================================================================== VALUE PASS FUNCTIONS



// =========================================================================================== HASH TABLE API REALIZATION



// ht_add_pair() function realization
// DON'T CALL ASYNC WITH FREE OR DELETE FUNCTIONS!


/*
 * Main add / set function.
 * Accept a pointer to the hash_table** table to be able to modify the table itself (if resize() occurs).
 * Accept the number of arguments (all: keys and values).
 * Parse the variadic parameters into pairs (key, value).
 * For each pair:
 * Generate ht_key using MAKE_KEY().
 * Generate ht_value using HT_VAL().
 * Call ht_add_pair(*table, key, value).
 */
void ht_add_elements(hash_table** table, size_t elements_quantity, ...)
{
    if (!table || !*table)
    {
        fprintf(stderr, "Error: invalid hash table pointer in ht_add_va.\n");
        return;
    }

    if (elements_quantity % 2 != 0)
    {
        fprintf(stderr, "Error: odd number of arguments in ht_add_va (keys and values must be paired).\n");
        return;      
    }

    va_list args;
    va_start(args, elements_quantity);

    for (size_t i = 0; i < elements_quantity; i += 2)
    {
        // Get the key
        ht_key key;

        // Key as int by the 1st iteration
        key = MAKE_KEY(va_arg(args, int));

        // If the key is a string
        if (key.type == HT_STR_KEY)
        {
            // Pass as a char*
            key.data.as_str = va_arg(args, char*);
        }

        // Get the value
        ht_value value;

        // Same actions as for key
        if (key.type == HT_INT_KEY)
        {
            int val = va_arg(args, int);
            value = HT_VAL(val);
        }

        else if (key.type == HT_STR_KEY)
        {
            char* sval = va_arg(args, char*);
            value = HT_VAL(sval);
        }

        // Add pair by helper-function
        ht_add_pair(*table, key, value);
    }

    va_end(args);
}


// Helper-function for the pair adding
// Calculate a hash based on the key type.
// Find a suitable index (linear probing).
// Insert (or replace) a value.
// Expand the table if necessary.
void ht_add_pair(hash_table* table, ht_key key, ht_value value)
{

    // Validate key type
    switch (k_type)
    {
        case HT_INT_KEY:
        case HT_STR_KEY:

            break;

        default:
            fprintf(stderr, "[ERROR] Invalid key_type in ht_create()\n");
            free(table);
            return NULL;
    }


    //  Validate value type
    switch (v_type)
    {
        case HT_U_INT8_VAL:
        case HT_U_INT16_VAL:
        case HT_U_INT32_VAL:
        case HT_U_INT64_VAL:
        case HT_INT8_VAL:
        case HT_INT16_VAL:
        case HT_INT32_VAL:
        case HT_INT64_VAL:
        case HT_SHORT_VAL:
        case HT_U_SHORT_VAL:
        case HT_U_INT_VAL:
        case HT_INT_VAL:
        case HT_LONG_VAL:
        case HT_U_LONG_VAL:
        case HT_LONG_LONG_VAL:
        case HT_U_LONG_LONG_VAL:
        case HT_FLOAT_VAL:
        case HT_DOUBLE_VAL:
        case HT_LONG_DOUBLE_VAL:
        case HT_CHAR_VAL:
        case HT_STR_VAL:
        case HT_ARRAY_VAL:

            break;

        default:
            fprintf(stderr, "[ERROR] Invalid value_type in ht_create()\n");
            free(table);
            return NULL;
    }



    // Error handler
    if (!table || !table->elements)
    {
        fprintf(stderr, "Error: invalid table pointer.\n");
        return;
    }

    if (value.ptr == NULL)
    {
        fprintf(stderr, "Error: value pointer is NULL.\n");
        return;
    }

    // Hash value calculation
    size_t hash_value = 0;

    switch (key.type)
    {
        case HT_INT_KEY:

            hash_value = hash_int(key.data.as_int, table->capacity);
            break;

        case HT_STR_KEY:

            hash_value = hash_str(key.data.as_str, table->capacity);
            break;

        default:
        
            fprintf(stderr, "Error: unknown key type.\n");
            return;
    }

    // Calculate index for the element
    size_t index = hash_value % table->capacity;


    // Collision fix
    // Linear probing if slot is occupied
    while (table->elements[index].occupied)
    {
        // Check if the same key already exists — replace the value
        ht_element* element = &table->elements[index];

        // Type error handler
        if (element->key.type != key.type)
        {
            index = (index + 1) % table->capacity;
            continue;
        }


        bool same_key = false;

        switch (key.type)
        {
            case HT_INT_KEY:
                same_key = (element->key.data.as_int == key.data.as_int);
                break;

            case HT_STR_KEY:
                same_key = (strcmp(element->key.data.as_str, key.data.as_str) == 0);
                break;
        }

        if (same_key)
        {
            // Empty value error handler
            if (!value.ptr)
            {
                fprintf(stderr, "Error: NULL value in memcpy\n");
                return;
            }

            // Replace existing value

            // Free old value
            free(element->value);

            // Add new data
            element->value = malloc(value.size);
            // Allocation error handler
            if (!element->value)
            {
                fprintf(stderr, "Error: failed to allocate memory for value\n");
                return;
            }

            memcpy(element->value, value.ptr, value.size);
            element->value_size = value.size;

            return;
        }

        index = (index + 1) % table->capacity; // Move to the next slot
    }

    // New element insert

    // Increment size and resize if needed
    table->size++;

    float load_factor = (float)table->size / table->capacity;

    // Check load factor and resize HT
    if (load_factor > table->max_load_factor)
    {
        resize_ht(table);
        index = hash_value % table->capacity;

        // Linear probing
        while (table->elements[index].occupied)
            index = (index + 1) % table->capacity;
    }

    // Insert
    ht_element* element = &table->elements[index];
    element->occupied = 1;

    if (key.type == HT_STR_KEY)
    {
        element->key.type = HT_STR_KEY;
        element->key.data.as_str = strdup(key.data.as_str);

        if (!element->key.data.as_str)
        {
            fprintf(stderr, "Error: failed to duplicate string key\n");
            return;
        }
    }
    else element->key = key;

    element->value = malloc(value.size);
    // Allocation error handler
    if (!element->value)
    {
        fprintf(stderr, "Error: failed to allocate memory for value\n");
        return;
    }

    memcpy(element->value, value.ptr, value.size);

    element->value_size = value.size;
}


// ht_delete() function realization
// Fully deletes the hash table: frees all keys, values, elements, and the table itself
void ht_delete(hash_table** table)
{
    // Safety check
    if (!table || !*table) return; 

    hash_table* curr_table = *table;

    // Free the memory allocated for elements
    if (curr_table->elements)
    {
        for (size_t i = 0; i < curr_table->capacity; i++)
        {
            ht_element* element = &curr_table->elements[i];

            if (element->occupied)
            {
                // Free string key if needed
                if (element->key.type == HT_STR_KEY && element->key.data.as_str)
                    free((void*)element->key.data.as_str);

                // Free stored value if allocated
                if (element->value)
                    free(element->value);
            }

            memset(element, 0, sizeof(ht_element));
        }

        // Free the array of elements
        free(curr_table->elements);
        
        curr_table->elements = NULL;
    }

    // Free the hash_table struct itself
    free(curr_table);
    
    *table = NULL; // Nullify caller's pointer
}



// ht_clear() function realization
// Clears all elements, frees string keys & values, 
// and resets the hash table to its initial capacity
void ht_clear(hash_table** table)
{
    // Safety check for NULL pointers
    if (!table || !*table) return;

    hash_table* curr_table = *table;

    // Handle empty table case (no elements yet)
    if (!curr_table->elements)
    {
        curr_table->capacity = HT_INITIAL_CAPACITY;
        curr_table->size = 0;
        curr_table->elements = calloc(curr_table->capacity, sizeof(ht_element));

        return;
    }

    size_t old_capacity = curr_table->capacity;


    // CASE 1: Table hasn't been resized — just clear elements
    if (old_capacity <= HT_INITIAL_CAPACITY)
    {
        for (size_t i = 0; i < old_capacity; ++i)
        {
            ht_element* element = &curr_table->elements[i];

            if (element->occupied)
            {
                
                if (element->key.type == HT_STR_KEY && element->key.data.as_str)
                    free((void*)element->key.data.as_str);

                // Free element value (allocated with malloc)
                if (element->value)
                    free(element->value);
            }

            // Reset element memory
            memset(element, 0, sizeof(ht_element));
        }

        curr_table->size = 0;
        curr_table->capacity = HT_INITIAL_CAPACITY;
        return;
    }
    

    // CASE 2: Table has been resized — shrink it back
    else
    {
        // Free all elements beyond initial capacity
        for (size_t i = HT_INITIAL_CAPACITY; i < old_capacity; ++i)
        {
            ht_element* element = &curr_table->elements[i];

            if (element->occupied)
            {
                if (element->key.type == HT_STR_KEY && element->key.data.as_str)
                    free((void*)element->key.data.as_str);

                if (element->value)
                    free(element->value);
            }
        }

        // Clear the first HT_INITIAL_CAPACITY elements
        for (size_t i = 0; i < HT_INITIAL_CAPACITY; ++i)
        {
            ht_element* element = &curr_table->elements[i];

            if (element->occupied)
            {
                if (element->key.type == HT_STR_KEY && element->key.data.as_str)
                    free((void*)element->key.data.as_str);

                if (element->value)
                    free(element->value);
            }

            memset(element, 0, sizeof(ht_element));
        }
    }

    // Shrink memory to initial capacity
    ht_element* new_elements = realloc(curr_table->elements, HT_INITIAL_CAPACITY * sizeof(ht_element));

    // After realloc (if succeeded), zero out the new memory part
    // Zero out resized memory just in case realloc gave a larger (fresh) block
    if (new_elements && old_capacity > HT_INITIAL_CAPACITY)
        memset(new_elements, 0, HT_INITIAL_CAPACITY * sizeof(ht_element));


    if (new_elements) curr_table->elements = new_elements;
    // else: keep old memory block to avoid pointer loss

    // Reset size and capacity
    curr_table->size = 0;
    curr_table->capacity = HT_INITIAL_CAPACITY;
}



// =========================================================================================== HASH TABLE API REALIZATION
