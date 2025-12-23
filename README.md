# hash_table
This repository hosts an experimental C library, aiming to provide a declarative-style interface for working with a universal hash map / hash table. IN PROGRESS. 


## Goal 🎯

The goal of this project is to provide a declarative-style interface for working with a universal hash map in C 🗃️


## Core problem ⚠️

Hash maps are a fundamental data structure used across a wide range of systems, yet C offers no standard or ergonomic solution.  
Experience from higher-level languages like C++ and Python shows that hash maps can be far more convenient and expressive to use.  
This project explores whether a similarly convenient, declarative interface can be built in C.


## Approach 🛠️

The library is implemented using structured programming with standard methods for element hashing with dynamic table capacity (resize and rehash).  
Interaction with the structure, however, is built around a combination of macros, generics, and void pointers to provide a more flexible and declarative API, like:

```c
ht_add(&table,
    1, val1,
    2, val2,
    "apple", 10,
    "banana", 20,
    key_var, hash_var
);
```  


## Low-level META divergence 🔬

I am fully aware that in C, large and universal data structures are typically built by manually overriding functions for each data type, or by using so-called "generics" (which are not true generics — see [video](https://www.youtube.com/watch?v=oEL9x6pP4FM)).  
However, these approaches are highly inconvenient when it comes to the final construction of programs and the usability of the API. They do not provide the familiar high-level experience found in C++ or Python.

This project consciously explores a different path. The choice of tools is deliberate, risky, and experimental — I do not yet know if the library will ever fully work. The goal is to investigate whether a more declarative, high-level API can coexist with the constraints of C.


## Project status-bar 📊

Current main status: 🚀 Beginning of development - v0.1

**Legend:**
- ✅ 🟢 done
- ⏳ 🟡 in progress
- ❌ 🔴 not started
- 🧪 experimental
- ⚠️ questionable decision

**Roadmap:**

1. 🏗️ Designing core structures ✅  
2. ✏️ Designing stubs for basic methods and their helper wrappers — macros, enums (constructor, setter) ⏳  
3. 📏 Implementing table boundary conditions ✅  
4. 🛠️ Implementing table constructor ✅  
5. 🔧 Implementing table setter using varargs and void* ⏳  

   5.1 📝 Implementing error handler for input part ✅  
   5.2 ⚙️ Implementing main internal logic ✅  
   5.3 🔍 Implementing helper function to check capacity ⏳  
   5.4 🔄 Implementing helper function for resize + rehash 🧪  
   5.5 🔑 Implementing helper hash functions for int (Knuth) and string (DJB2) keys ✅  
   5.6 🛡️ Implementing structure to validate hashes and values before insertion 🧪  
   5.7 📐 Implementing helper function to check boundary indices ✅  
   5.8 ↪️ Implementing helper function for linear probing of repeated hashes ❌  
   5.9 🗝️ Implementing function to insert a key of any type into the data structure ⏳  
   5.10 💾 Implementing function to insert a value of any type into the data structure ⏳  
   5.11 🧩 Implementing functions, macros, and generics for wrapping setter into a universal form, e.g. ❌  
   
6. 🔍 Implementing getters ❌  
7. 🧮 Implementing helper functions (check size, length, etc.) ❌  
8. 🧱 Module assembly ❌  
9. 🧪 Testing ❌  
10. 🏁 Release ❌   


## How you can help

I would be glad to get assistance with:

1. Evaluating the idea  
2. Tips on working with `varargs`  
3. Hashing and probing algorithms  
4. Checking the correctness of memory allocations
