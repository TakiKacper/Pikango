#pragma once

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

void pikango::attach_to_frame_buffer(
    frame_buffer_handle         target, 
    texture_buffer_handle       attachment,
    framebuffer_attachment_type attachment_type, 
    size_t                      slot
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto frame_buffer       = std::any_cast<frame_buffer_handle>(args[0]);
        auto attachment         = std::any_cast<texture_buffer_handle>(args[1]);
        auto attachment_type    = std::any_cast<size_t>(args[2]);

        auto fbi = pikango_internal::obtain_handle_object(frame_buffer);
        auto ai  = pikango_internal::obtain_handle_object(attachment);

        glBindFramebuffer(GL_FRAMEBUFFER, fbi->id);
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, 
            attachment_type, 
            ai->texture_type, 
            ai->id, 
            0
        );
    };

    if (attachment_type != framebuffer_attachment_type::color) slot = 0;
    auto attachment_gl_type = get_framebuffer_attachment_type(attachment_type) + slot;

    enqueue_task(func, {target, attachment, attachment_gl_type}, pikango::queue_type::general);
}

pikango::frame_buffer_handle default_frame_buffer;

struct init_default_frame_buffer
{
    init_default_frame_buffer()
    {
        default_frame_buffer = pikango_internal::make_handle(new pikango_internal::frame_buffer_impl);
        auto fbi = pikango_internal::obtain_handle_object(default_frame_buffer);
        fbi->id = 0;
    }   
} init_default_frame_buffer_call;

pikango::frame_buffer_handle pikango::OPENGL_ONLY_get_default_frame_buffer()
{
    return default_frame_buffer;
}
