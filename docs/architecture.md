# Core Architecture Concepts

- **Library State**, is a collective term for all the states hidden in the library implemetation
- **Resource** is any ``x`` that can be created with ``pikango::new_x`` function
- **Handle** is pikango' alternative of ``std::shared_ptr``. It automatically releases handled resource when last handle pointing to the resource goes out of scope, or gets empty due to ``handle::~handle()`` call.
- **Queue** is an object to which **command buffers** can be **submitted**, causing their later **execution**
- **Command Buffer** is a type of resource to which **commands** can be **recorded**
- **Command** is a type of pikango function. Commands instead of executing right after being called on the cpu **record** themselfs into the current **command buffer**, and execute during this buffer **execution**.

# General Graphics Concepts

- **Buffer** is a type of resource, representing block of memory on the GPU.
    - **Vertex Buffer** is type of buffer for storing verticies
    - **Index Buffer** is type of buffer for storing indicies
    - **Instance Buffer** is type of buffer for storing *instances* attributes
    - **Uniform Buffer** is type of buffer for storing **shaders** uniforms
- **Texture** is a type of resource, representing block of memory on the GPU + information on how to *sample** it.


# Code Flow




![Execution Diagram](res/execution_diagram.png)