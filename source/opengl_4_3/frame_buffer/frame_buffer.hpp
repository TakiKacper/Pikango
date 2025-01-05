#pragma once
#include <map>

PIKANGO_IMPL(frame_buffer)
{
    GLuint id = 0;
    std::map<unsigned int, pikango::texture_2d_handle> color_attachments;
    pikango::texture_2d_handle depth_attachment;
    pikango::texture_2d_handle stencil_attachment;
    ~frame_buffer_impl();
};

PIKANGO_NEW(frame_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::frame_buffer_impl);

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<frame_buffer_handle>(args[0]);
        auto fbi = pikango_internal::object_write_access(handle);
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

pikango::frame_buffer_handle* create_default_framebuffer_handle()
{
    auto handle_ptr = pikango_internal::alloc_handle(new pikango_internal::frame_buffer_impl);
    auto fbi = pikango_internal::object_write_access(*handle_ptr);
    fbi->id = 0;
    return handle_ptr;
}

size_t pikango::get_max_framebuffer_color_buffers_attachments()
{
    return max_color_attachments;
}

void pikango::attach_framebuffer_color_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment,
    unsigned int index
)
{
    pikango_internal::object_write_access(target)->color_attachments.insert({index, attachment});
    attach_framebuffer_buffer_generic<GL_COLOR_ATTACHMENT0>(target, attachment, index);
}

void pikango::attach_framebuffer_depth_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment
)
{
    pikango_internal::object_write_access(target)->depth_attachment = attachment;
    attach_framebuffer_buffer_generic<GL_DEPTH_ATTACHMENT>(target, attachment, 0);
}

void pikango::attach_framebuffer_stencil_buffer(
    frame_buffer_handle target,
    texture_2d_handle attachment
)
{
    pikango_internal::object_write_access(target)->stencil_attachment = attachment;
    attach_framebuffer_buffer_generic<GL_STENCIL_ATTACHMENT>(target, attachment, 0);
}



void pikango::detach_framebuffer_color_buffer(
    frame_buffer_handle target,
    unsigned int index
)
{
    pikango_internal::object_write_access(target)->color_attachments.insert({index, {}});
    detach_framebuffer_buffer_generic<GL_COLOR_ATTACHMENT0>(target, index);
}

void pikango::detach_framebuffer_depth_buffer(frame_buffer_handle target)
{
    pikango_internal::object_write_access(target)->depth_attachment = {};
    detach_framebuffer_buffer_generic<GL_DEPTH_ATTACHMENT>(target, 0);
}

void pikango::detach_framebuffer_stencil_buffer(frame_buffer_handle target)
{
    pikango_internal::object_write_access(target)->stencil_attachment = {};
    detach_framebuffer_buffer_generic<GL_STENCIL_ATTACHMENT>(target, 0);
}



pikango::texture_2d_handle pikango::get_framebuffer_color_buffer(
    frame_buffer_handle target,
    unsigned int slot
)
{
    auto fbi = pikango_internal::object_read_access(target);
    auto itr = fbi->color_attachments.find(slot);

    if (itr == fbi->color_attachments.end())
        return {};
    return itr->second;
}

pikango::texture_2d_handle pikango::get_framebuffer_depth_buffer(frame_buffer_handle target)
{
    return pikango_internal::object_read_access(target)->depth_attachment;
}

pikango::texture_2d_handle pikango::get_framebuffer_stencil_buffer(frame_buffer_handle target)
{
    return pikango_internal::object_read_access(target)->stencil_attachment;
}
