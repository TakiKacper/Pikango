PIKANGO_IMPL(texture_1d)
{
    GLuint id     = 0;
    size_t width  = 0;

    ~texture_1d_impl();
};

pikango_internal::texture_1d_impl::~texture_1d_impl()
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

PIKANGO_NEW(texture_1d)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_1d_impl);
    auto ti = pikango_internal::object_write_access(handle);
    ti->id = 0;
    return handle;
};

PIKANGO_DELETE(texture_1d)
{

};

void pikango::write_texture(
    texture_1d_handle target, 
    texture_format source_format, 
    texture_format inner_format,
    size_t width, 
    void* pixel_data
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_1d_handle>(args[0]);
        auto source_format = std::any_cast<GLuint>(args[1]);
        auto inner_format = std::any_cast<GLuint>(args[2]);
        auto width = std::any_cast<size_t>(args[3]);
        auto data = std::any_cast<void*>(args[4]);

        auto ti = pikango_internal::object_write_access(handle);

        if (ti->id == 0)
            glGenTextures(1, &ti->id);
        glBindTexture(GL_TEXTURE_1D, ti->id);

        glTexImage1D(GL_TEXTURE_1D, 0, inner_format, width, 0, source_format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_1D);
    };
    enqueue_task(func, {target, get_texture_format(source_format), get_texture_format(inner_format), width, pixel_data});
}

void pikango::set_texture_wraping(texture_1d_handle target, texture_wraping x)
{
    set_texture_wraping_1d_generic<texture_1d_handle, GL_TEXTURE_1D>(target, x);
}

void pikango::set_texture_filtering(texture_1d_handle target, texture_filtering magnifying, texture_filtering minifying, texture_filtering mipmap)
{
    set_texture_filtering_generic<texture_1d_handle, GL_TEXTURE_1D>(target, magnifying, minifying, mipmap);
}

void pikango::bind_texture_to_pool(texture_1d_handle target, size_t index)
{
    bind_texture_to_pool_generic(target, index);
}