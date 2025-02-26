PIKANGO_IMPL(texture_buffer)
{
    GLuint id = 0;
    GLuint texture_type = 0;

    size_t dim_1 = 0;
    size_t dim_2 = 0;
    size_t dim_3 = 0;

    size_t array_layers = 0;
    size_t mipmap_layers = 0;

    ~texture_buffer_impl();
};

PIKANGO_NEW(texture_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_buffer_impl);
    auto ti = pikango_internal::obtain_handle_object(handle);
    ti->id = 0;
    return handle;
};

pikango_internal::texture_buffer_impl::~texture_buffer_impl()
{
    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteTextures(1, &id);
    };

    if (id != 0) enqueue_task(func, {id}, pikango::queue_type::general);
}

void pikango::cmd::assign_texture_buffer_memory(
    texture_buffer_handle   target,
    texture_type            type,
    size_t                  mipmap_layers,
    texture_sized_format    memory_format,
    size_t                  dim_1,
    size_t                  dim_2,
    size_t                  dim_3
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto    handle  = std::any_cast<texture_buffer_handle>(args[0]);
        auto    type    = std::any_cast<texture_type>(args[1]);
        size_t  mipmap  = std::any_cast<size_t>(args[2]);
        auto    format  = std::any_cast<GLuint>(args[3]);

        size_t dim_1 = std::any_cast<size_t>(args[4]);
        size_t dim_2 = std::any_cast<size_t>(args[5]);
        size_t dim_3 = std::any_cast<size_t>(args[6]);

        auto tbi = pikango_internal::obtain_handle_object(handle);

        if (tbi->id == 0) glGenTextures(1, &tbi->id);

        switch (type)
        {
        case pikango::texture_type::texture_1d:
            glBindTexture(GL_TEXTURE_1D, tbi->id);
            glTexStorage1D(GL_TEXTURE_1D, mipmap, format, dim_1);
            tbi->texture_type = GL_TEXTURE_1D;
            break;

        case pikango::texture_type::texture_1d_array:
            glBindTexture(GL_TEXTURE_1D_ARRAY, tbi->id);
            glTexStorage2D(GL_TEXTURE_1D_ARRAY, mipmap, format, dim_1, dim_2);
            tbi->texture_type = GL_TEXTURE_1D_ARRAY;
            break;

        case pikango::texture_type::texture_2d:
            glBindTexture(GL_TEXTURE_2D, tbi->id);
            glTexStorage2D(GL_TEXTURE_2D, mipmap, format, dim_1, dim_2);
            tbi->texture_type = GL_TEXTURE_2D;
            break;

        case pikango::texture_type::texture_2d_array:
            glBindTexture(GL_TEXTURE_2D_ARRAY, tbi->id);
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipmap, format, dim_1, dim_2, dim_3);
            tbi->texture_type = GL_TEXTURE_2D_ARRAY;
            break;

        case pikango::texture_type::texture_3d:
            glBindTexture(GL_TEXTURE_3D, tbi->id);
            glTexStorage3D(GL_TEXTURE_3D, mipmap, format, dim_1, dim_2, dim_3);
            tbi->texture_type = GL_TEXTURE_3D;
            break;

        case pikango::texture_type::texture_cubemap:
            glBindTexture(GL_TEXTURE_CUBE_MAP, tbi->id);
            glTexStorage3D(GL_TEXTURE_CUBE_MAP, mipmap, format, dim_1, dim_2, 6);
            tbi->texture_type = GL_TEXTURE_CUBE_MAP;
            break;
        }
    };

    record_task(func, {
        target,
        type,
        mipmap_layers,
        get_texture_sized_format(memory_format),
        dim_1,
        dim_2,
        dim_3
    });
}

void pikango::cmd::write_texture_buffer(
    texture_buffer_handle   target,
    size_t                  mipmap_layer,
    texture_source_format   source_format,
    void*                   data,
    size_t                  off_1,
    size_t                  off_2,
    size_t                  off_3,
    size_t                  dim_1,
    size_t                  dim_2,
    size_t                  dim_3
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_buffer_handle>(args[0]);

        auto mipmap = std::any_cast<size_t>(args[1]);
        auto format = std::any_cast<GLuint>(args[2]);
        auto data   = std::any_cast<void*>(args[3]);

        auto off_1 = std::any_cast<size_t>(args[4]);
        auto off_2 = std::any_cast<size_t>(args[5]);
        auto off_3 = std::any_cast<size_t>(args[6]);

        auto dim_1 = std::any_cast<size_t>(args[7]);
        auto dim_2 = std::any_cast<size_t>(args[8]);
        auto dim_3 = std::any_cast<size_t>(args[9]);

        auto tbi = pikango_internal::obtain_handle_object(handle);

        constexpr static GLuint cubemap_faces[] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        switch (tbi->texture_type)
        {
        case GL_TEXTURE_1D:
            glBindTexture(tbi->texture_type, tbi->id);
            glTexSubImage1D(
                tbi->texture_type, mipmap, 
                off_1, 
                dim_1, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;

        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
            glBindTexture(tbi->texture_type, tbi->id);
            glTexSubImage2D(
                tbi->texture_type, mipmap, 
                off_1, off_2, 
                dim_1, dim_2, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glBindTexture(tbi->texture_type, tbi->id);
            glTexSubImage3D(
                tbi->texture_type, mipmap, 
                off_1, off_2, off_3, 
                dim_1, dim_2, dim_3, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;

        case GL_TEXTURE_CUBE_MAP:
            auto cubemap_face = cubemap_faces[off_3 % 6];
            glBindTexture(cubemap_face, tbi->id);
            glTexSubImage2D(
                cubemap_face, mipmap, 
                off_1, off_2, 
                dim_1, dim_2, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;
        }
    };

    record_task(func, {
        target,
        mipmap_layer,
        get_texture_source_format(source_format),
        data,
        off_1,
        off_2,
        off_3,
        dim_1,
        dim_2,
        dim_3
    });
}
