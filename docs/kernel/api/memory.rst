Memory allocation
=================

Convolution provides two levels of memory allocation API - an automatic allocator API, and raw allocator APIs (one each for virtual and physical memory). Code should aim to use the automatic APIs unless absolutely necessary to use the raw APIs. If a use of the raw allocation APIs is common, but is not provided by any existing automatic APIs, try to extend (or create new) the existing APIs rather than using raw allocation APIs.

Automatic memory allocation
---------------------------
Two automatic allocator APIs are provided by Convolution, however :cpp:type:`memory::MemoryMap` should cover the majority of use cases.

.. Add new allocators here - preferably keep alphabetical order
.. doxygenclass:: memory::MemoryMap
    :members:
.. doxygenclass:: memory::KStack
    :members:

Virtual memory allocation
--------------------------

.. doxygenclass:: memory::IVirtualAllocator
    :members:
    :protected-members:

Current memory allocators built into the kernel:

.. Add new allocators here - preferably keep alphabetical order
.. doxygenclass:: memory::virtual_allocators::MonotonicAllocator
.. doxygenclass:: memory::virtual_allocators::TreeAllocator

Physical memory allocation
--------------------------
Physical memory allocation should be done through the generic allocation interface provided by :cpp:type:`memory::IPhysicalAllocator`, and any allocation wrapping APIs should be polymorphic. (See :cpp:type:`memory::MemoryMap` for an example implementation).
New memory allocators should be subclassed from :cpp:type:`memory::IPhysicalAllocator`, and implement the correct virtual functions.

.. doxygenclass:: memory::IPhysicalAllocator
    :members:
    :protected-members:

Current memory allocators built into the kernel:

.. Add new allocators here - preferably keep alphabetical order
.. doxygenclass:: memory::physical_allocators::BitmapAllocator
.. doxygenclass:: memory::physical_allocators::MonotonicAllocator
