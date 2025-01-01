#include "pikango/pikango.hpp"

/*
    Util
*/

static bool operator==(const pikango::rectangle& a, const pikango::rectangle& b)
{
    return (
        a.ax == b.ax && 
        a.ay == b.ay && 
        a.bx == b.bx && 
        a.by == b.by
    );
}

static bool operator!=(const pikango::rectangle& a, const pikango::rectangle& b)
{
    return !(a == b);
}

/*
    Private Globals
*/

namespace pikango_internal
{
    pikango::error_notification_callback error_callback = nullptr;

    void log_error(const char* text)
    {
        if (error_callback != nullptr)
            error_callback(text);
        else
            abort();
    }
}

/*
    API Implementation
*/

#ifdef PIKANGO_OPENGL_4_3
    #include "opengl_4_3/pikango_impl.hpp"
#else
    #error No Pikango implementation specified. See file: pikango.cpp
#endif

/*
    Implementation
*/

void pikango::set_error_notification_callback(error_notification_callback callback)
{
    pikango_internal::error_callback = callback;
}

size_t pikango::size_of(data_type dt)
{
    switch (dt)
    {
    case data_type::float32: return 4;
    case data_type::int32:   return 4;
    case data_type::vec2f32: return 4 * 2;
    case data_type::vec3f32: return 4 * 3;
    case data_type::vec4f32: return 4 * 4;
    }

    //will never be reached
    return 0;
}

#define IMPLEMENT_DESTRUCTOR(name)  \
    template<>                      \
    void pikango_internal::implementations_destructor(name##_impl* impl) \
    {                                                                    \
        delete impl;                                                     \
    }
    
IMPLEMENT_DESTRUCTOR(graphics_pipeline);
//IMPLEMENT_DESTRUCTOR(compute_pipeline);
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
