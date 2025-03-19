struct pikango_internal::texture_sampler_impl
{
    GLuint id = 0;
    ~texture_sampler_impl();
};

pikango::texture_sampler_handle pikango::new_texture_sampler(const texture_sampler_create_info& info)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_sampler_impl);

    auto func = [](std::vector<std::any>& args)
    {
        auto handle = std::any_cast<pikango::texture_sampler_handle>(args[0]);
        auto info   = std::any_cast<texture_sampler_create_info>(args[1]);

        auto tsi = pikango_internal::obtain_handle_object(handle);

        glGenSamplers(1, &tsi->id);

        glSamplerParameteri(tsi->id, GL_TEXTURE_WRAP_S, get_texture_wraping(info.wraping_x));
        glSamplerParameteri(tsi->id, GL_TEXTURE_WRAP_T, get_texture_wraping(info.wraping_y));
        glSamplerParameteri(tsi->id, GL_TEXTURE_WRAP_R, get_texture_wraping(info.wraping_z));

        glSamplerParameteri(tsi->id, GL_TEXTURE_MAG_FILTER, get_texture_filtering(info.magnifying_filter));
        glSamplerParameteri(tsi->id, GL_TEXTURE_MIN_FILTER, combine_min_filters(info.minifying_filter, info.mipmap_filter));
    };

    enqueue_task(func, {handle, info}, pikango::queue_type::general);
    return handle;
};

pikango_internal::texture_sampler_impl::~texture_sampler_impl()
{
    auto func = [](std::vector<std::any>& args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteSamplers(1, &id);
    };

    if (id != 0) enqueue_task(func, {id}, pikango::queue_type::general);
}
