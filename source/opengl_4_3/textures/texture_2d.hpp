PIKANGO_IMPL(texture_2d)
{
    GLuint id     = 0;
    size_t width  = 0;
    size_t height = 0;

    ~texture_2d_impl();
};

pikango_internal::texture_2d_impl::~texture_2d_impl()
{
    if (id != 0)
    {
        auto func = [](std::vector<std::any> args)
        {
            auto id = std::any_cast<GLuint>(args[0]);
            glDeleteTextures(1, &id);
        };
        enqueue_task(func, {id});
    }
}

PIKANGO_NEW(texture_2d)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_2d_impl);
    auto ti = pikango_internal::object_write_access(handle);
    ti->id = 0;
    return handle;
};

PIKANGO_DELETE(texture_2d)
{

};

void pikango::write_texture(
    texture_2d_handle target, 
    texture_format source_format, 
    texture_format inner_format,
    size_t width, 
    size_t height, 
    void* pixel_data
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_2d_handle>(args[0]);
        auto source_format = std::any_cast<GLuint>(args[1]);
        auto inner_format = std::any_cast<GLuint>(args[2]);
        auto width = std::any_cast<size_t>(args[3]);
        auto height = std::any_cast<size_t>(args[4]);
        auto data = std::any_cast<void*>(args[5]);

        auto ti = pikango_internal::object_write_access(handle);

        if (ti->id == 0)
            glGenTextures(1, &ti->id);
        glBindTexture(GL_TEXTURE_2D, ti->id);

        auto source_data_type = inner_format == GL_DEPTH24_STENCIL8 ? GL_UNSIGNED_INT_24_8 : GL_UNSIGNED_BYTE;

        glTexImage2D(GL_TEXTURE_2D, 0, inner_format, width, height, 0, source_format, source_data_type, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    };
    enqueue_task(func, {target, get_texture_format(source_format), get_texture_format(inner_format), width, height, pixel_data});
}

void pikango::set_texture_wraping(texture_2d_handle target, texture_wraping x, texture_wraping y)
{
    set_texture_wraping_2d_generic<texture_2d_handle, GL_TEXTURE_2D>(target, x, y);
}

void pikango::set_texture_filtering(texture_2d_handle target, texture_filtering magnifying, texture_filtering minifying, texture_filtering mipmap)
{
    set_texture_filtering_generic<texture_2d_handle, GL_TEXTURE_2D>(target, magnifying, minifying, mipmap);
}

void pikango::bind_texture_to_pool(texture_2d_handle target, size_t index)
{
    bind_texture_to_pool_generic<texture_2d_handle, GL_TEXTURE_2D>(target, index);
}
