#pragma once
#include <map>

PIKANGO_IMPL(frame_buffer)
{
    GLuint id = 0;
    ~frame_buffer_impl();
};

PIKANGO_NEW(frame_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::frame_buffer_impl);

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<frame_buffer_handle>(args[0]);
        auto fbi = pikango_internal::obtain_handle_object(handle);
        glGenFramebuffers(1, &fbi->id);
    };

    enqueue_task(func, {handle}, pikango::queue_type::general);
    return handle;
}

pikango_internal::frame_buffer_impl::~frame_buffer_impl()
{
    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteFramebuffers(1, &id);
    };

    if (id != 0)
        enqueue_task(func, {id}, pikango::queue_type::general);
}

pikango::frame_buffer_handle create_default_framebuffer_handle()
{
    auto handle = pikango_internal::make_handle(new pikango_internal::frame_buffer_impl);
    auto fbi = pikango_internal::obtain_handle_object(handle);
    fbi->id = 0;
    return handle;
}

size_t pikango::get_max_framebuffer_color_buffers_attachments()
{
    return max_color_attachments;
}
