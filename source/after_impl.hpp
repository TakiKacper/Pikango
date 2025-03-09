#define IMPLEMENT_DESTRUCTOR(name)  \
    template<>                      \
    void pikango_internal::implementations_destructor(name##_impl* impl) \
    {                                                                    \
        delete impl;                                                     \
    }
    
IMPLEMENT_DESTRUCTOR(graphics_pipeline);
//IMPLEMENT_DESTRUCTOR(compute_pipeline);
IMPLEMENT_DESTRUCTOR(shader);
IMPLEMENT_DESTRUCTOR(command_buffer);
IMPLEMENT_DESTRUCTOR(fence);
IMPLEMENT_DESTRUCTOR(buffer);
IMPLEMENT_DESTRUCTOR(texture_sampler);
IMPLEMENT_DESTRUCTOR(texture_buffer);
