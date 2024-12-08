#include "pikango/pikango.hpp"

/*
    Private Globals
*/

namespace pikango_internal
{
    pikango::error_notification_callback error_callback = nullptr;
}

/*
    API Implementation
*/

#ifdef PIKANGO_OPENGL_4_3
    #include "opengl_4_3/pikango_impl.hpp"

    void pikango::OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args)
    {
        opengl_tasks_queue.push({task, std::move(args)});
    }
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

#define IMPLEMENT_DESTRUCTOR(name)  \
    template<>                      \
    void pikango_internal::implementations_destructor(name##_impl* impl) \
    {                                                                    \
        delete impl;                                                     \
    }


IMPLEMENT_DESTRUCTOR(vertex_buffer);
IMPLEMENT_DESTRUCTOR(data_layout);
IMPLEMENT_DESTRUCTOR(graphics_shader);
IMPLEMENT_DESTRUCTOR(frame_buffer);
