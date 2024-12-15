#include "pikango/pikango.hpp"

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

size_t pikango::size_of(data_types dt)
{
    switch (dt)
    {
    case data_types::float32: return 4;
    case data_types::int32:   return 4;
    case data_types::vec2f32: return 4 * 2;
    case data_types::vec3f32: return 4 * 3;
    case data_types::vec4f32: return 4 * 4;
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


IMPLEMENT_DESTRUCTOR(vertex_buffer);
IMPLEMENT_DESTRUCTOR(index_buffer);
IMPLEMENT_DESTRUCTOR(data_layout);
IMPLEMENT_DESTRUCTOR(graphics_shader);
IMPLEMENT_DESTRUCTOR(texture_1d)
IMPLEMENT_DESTRUCTOR(texture_2d)
IMPLEMENT_DESTRUCTOR(texture_3d)
IMPLEMENT_DESTRUCTOR(texture_cube)
IMPLEMENT_DESTRUCTOR(frame_buffer);
