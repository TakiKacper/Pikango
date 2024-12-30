template<GLenum gl_attachment_type>
void attach_framebuffer_buffer_generic(
    pikango::frame_buffer_handle target,
    pikango::texture_2d_handle attachment,
    size_t index
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto target = std::any_cast<pikango::frame_buffer_handle>(args[0]);
        auto attachment = std::any_cast<pikango::texture_2d_handle>(args[1]);
        auto index = std::any_cast<size_t>(args[2]);

        auto fbi = pikango_internal::object_read_access(target);
        auto ti = pikango_internal::object_read_access(attachment);

        glBindFramebuffer(GL_FRAMEBUFFER, fbi->id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, gl_attachment_type + index, GL_TEXTURE_2D, ti->id, 0);
    };

    enqueue_task(func, {target}, pikango::queue_type::general);
}

template<GLenum gl_attachment_type>
void detach_framebuffer_buffer_generic(
    pikango::frame_buffer_handle target,
    size_t index
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto target = std::any_cast<pikango::frame_buffer_handle>(args[0]);
        auto index = std::any_cast<size_t>(args[1]);

        auto fbi = pikango_internal::object_read_access(target);

        glBindFramebuffer(GL_FRAMEBUFFER, fbi->id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, gl_attachment_type + index, GL_TEXTURE_2D, 0, 0);
    };

    enqueue_task(func, {target}, pikango::queue_type::general);
}