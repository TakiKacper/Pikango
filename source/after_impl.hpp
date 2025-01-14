#define IMPLEMENT_DESTRUCTOR(name)  \
    template<>                      \
    void pikango_internal::implementations_destructor(name##_impl* impl) \
    {                                                                    \
        delete impl;                                                     \
    }
    
IMPLEMENT_DESTRUCTOR(graphics_pipeline);
//IMPLEMENT_DESTRUCTOR(compute_pipeline);
IMPLEMENT_DESTRUCTOR(resources_descriptor);
IMPLEMENT_DESTRUCTOR(vertex_shader);
IMPLEMENT_DESTRUCTOR(pixel_shader);
IMPLEMENT_DESTRUCTOR(geometry_shader);
IMPLEMENT_DESTRUCTOR(command_buffer);
IMPLEMENT_DESTRUCTOR(fence);
IMPLEMENT_DESTRUCTOR(vertex_buffer);
IMPLEMENT_DESTRUCTOR(index_buffer);
IMPLEMENT_DESTRUCTOR(instance_buffer);
IMPLEMENT_DESTRUCTOR(uniform_buffer);
IMPLEMENT_DESTRUCTOR(texture_1d)
IMPLEMENT_DESTRUCTOR(texture_2d)
IMPLEMENT_DESTRUCTOR(texture_3d)
IMPLEMENT_DESTRUCTOR(texture_cube)
//IMPLEMENT_DESTRUCTOR(texture_1d_array);
//IMPLEMENT_DESTRUCTOR(texture_2d_array);
//IMPLEMENT_DESTRUCTOR(renderbuffer);
IMPLEMENT_DESTRUCTOR(frame_buffer);
