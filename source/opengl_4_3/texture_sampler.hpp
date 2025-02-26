PIKANGO_IMPL(texture_sampler)
{
    GLuint id = 0;
    ~texture_sampler_impl();
};

PIKANGO_NEW(texture_sampler)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_sampler_impl);

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::texture_sampler_handle>(args[0]);
        auto ti = pikango_internal::obtain_handle_object(handle);
        glGenSamplers(1, &ti->id);
    };
    enqueue_task(func, {handle}, pikango::queue_type::general);

    return handle;
};

pikango_internal::texture_sampler_impl::~texture_sampler_impl()
{
    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteSamplers(1, &id);
    };

    if (id != 0) enqueue_task(func, {id}, pikango::queue_type::general);
}

void pikango::set_sampler_wraping(
    texture_sampler_handle target, 
    texture_wraping x, 
    texture_wraping y, 
    texture_wraping z
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_sampler_handle>(args[0]);
        auto tw_x   = std::any_cast<GLuint>(args[1]);
        auto tw_y   = std::any_cast<GLuint>(args[2]);
        auto tw_z   = std::any_cast<GLuint>(args[3]);

        auto tsi = pikango_internal::obtain_handle_object(handle);

        glSamplerParameteri(tsi->id, GL_TEXTURE_WRAP_S, tw_x);
        glSamplerParameteri(tsi->id, GL_TEXTURE_WRAP_T, tw_y);
        glSamplerParameteri(tsi->id, GL_TEXTURE_WRAP_R, tw_z);
    };

    enqueue_task(func, {target, get_texture_wraping(x), get_texture_wraping(y), get_texture_wraping(z)}, pikango::queue_type::general);
}

void pikango::set_sampler_filtering(
    texture_sampler_handle target, 
    texture_filtering magnifying, 
    texture_filtering minifying, 
    texture_filtering mipmap
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle     = std::any_cast<texture_sampler_handle>(args[0]);
        auto mag_filter = std::any_cast<GLuint>(args[1]);
        auto min_filter = std::any_cast<GLuint>(args[2]);

        auto tsi = pikango_internal::obtain_handle_object(handle);

        glSamplerParameteri(tsi->id, GL_TEXTURE_MAG_FILTER, mag_filter);
        glSamplerParameteri(tsi->id, GL_TEXTURE_MIN_FILTER, min_filter);
    };

    enqueue_task(func, {target, get_texture_filtering(magnifying), combine_min_filters(minifying, mipmap)}, pikango::queue_type::general);
}
