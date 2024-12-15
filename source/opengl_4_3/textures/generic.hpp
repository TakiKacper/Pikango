template<class handle_type>
inline void bind_texture_to_pool_generic(handle_type target, size_t index)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto index = std::any_cast<size_t>(args[1]);

        auto ti = pikango_internal::object_read_access(handle);

        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, ti->id);
    };
    enqueue_task(func, {target, index});
}

template<class handle_type, GLuint gl_texture_type>
inline void set_texture_filtering_generic(
    handle_type target, 
    pikango::texture_filtering magnifying, 
    pikango::texture_filtering minifying, 
    pikango::texture_filtering mipmap
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto mag_filter = std::any_cast<GLuint>(args[1]);
        auto min_filter = std::any_cast<GLuint>(args[2]);

        auto ti = pikango_internal::object_read_access(handle);

        glBindTexture(gl_texture_type, ti->id);
        glTexParameteri(gl_texture_type, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTexParameteri(gl_texture_type, GL_TEXTURE_MIN_FILTER, min_filter);
    };

    enqueue_task(func, {target, get_texture_filtering(magnifying), combine_min_filters(minifying, mipmap)});
}

template<class handle_type, GLuint gl_texture_type>
inline void set_texture_wraping_1d_generic(handle_type target, pikango::texture_wraping x)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto tw_x   = std::any_cast<GLuint>(args[1]);

        auto ti = pikango_internal::object_read_access(handle);

        glBindTexture(gl_texture_type, ti->id);
        glTexParameteri(gl_texture_type, GL_TEXTURE_WRAP_S, tw_x);
    };
    enqueue_task(func, {target, get_texture_wraping(x)});
}

template<class handle_type, GLuint gl_texture_type>
inline void set_texture_wraping_2d_generic(handle_type target, pikango::texture_wraping x, pikango::texture_wraping y)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto tw_x   = std::any_cast<GLuint>(args[1]);
        auto tw_y   = std::any_cast<GLuint>(args[2]);

        auto ti = pikango_internal::object_read_access(handle);

        glBindTexture(gl_texture_type, ti->id);
        glTexParameteri(gl_texture_type, GL_TEXTURE_WRAP_S, tw_x);
        glTexParameteri(gl_texture_type, GL_TEXTURE_WRAP_T, tw_y);
    };
    enqueue_task(func, {target, get_texture_wraping(x), get_texture_wraping(y)});
}

template<class handle_type, GLuint gl_texture_type>
inline void set_texture_wraping_3d_generic(handle_type target, pikango::texture_wraping x, pikango::texture_wraping y, pikango::texture_wraping z)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto tw_x   = std::any_cast<GLuint>(args[1]);
        auto tw_y   = std::any_cast<GLuint>(args[2]);
        auto tw_z   = std::any_cast<GLuint>(args[3]);

        auto ti = pikango_internal::object_read_access(handle);

        glBindTexture(gl_texture_type, ti->id);
        glTexParameteri(gl_texture_type, GL_TEXTURE_WRAP_S, tw_x);
        glTexParameteri(gl_texture_type, GL_TEXTURE_WRAP_T, tw_y);
        glTexParameteri(gl_texture_type, GL_TEXTURE_WRAP_R, tw_z);
    };
    enqueue_task(func, {target, get_texture_wraping(x), get_texture_wraping(y), get_texture_wraping(z)});
}