clWrapper
=========

OpenCL Wrapper for C++ to provide easier access to context and queue management, 
memory allocation/deallocation and kernel invoking.

###Features

1. Memory buffers will automatically deallocate themselves when they go out of scope.
2. Buffer types are templated to show what type they are expected to contain.
3. Buffers can be set to automatically retrieve contents to host storage when they get modified by a kernel or to operate manually.
4. Events are also automatically created to allow for all memory syncing to be performed in a seperate queue automatically which allows for automatic asynchronous memory transfers wherever possible.
5. Includes second project which uses Boost.Test to verify wrapper is working properly.

