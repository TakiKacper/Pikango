PIKANGO_IMPL(texture_buffer)
{
    GLuint id;

    GLenum type;
    GLenum format;

    size_t dim1;
    size_t dim2;
    size_t dim3;

    size_t mipmap;

    ~texture_buffer_impl();
};

PIKANGO_NEW(texture_buffer)
{
    auto tbi = new pikango_internal::texture_buffer_impl;

    tbi->id = 0;

    tbi->type = get_texture_type(info.type);
    tbi->format = get_texture_sized_format(info.memory_format);

    tbi->dim1 = info.dim1;
    tbi->dim2 = info.dim2;
    tbi->dim3 = info.dim3;

    tbi->mipmap = info.mipmap_layers;

    auto handle = pikango_internal::make_handle(tbi);

    auto func = [](std::vector<std::any> args)
    {
        auto handle  = std::any_cast<texture_buffer_handle>(args[0]);
        auto tbi = pikango_internal::obtain_handle_object(handle);

        glGenTextures(1, &tbi->id);
        glBindTexture(tbi->type, tbi->id);

        switch (tbi->type)
        {
        case GL_TEXTURE_1D:
            glTexStorage1D(tbi->type, tbi->mipmap, tbi->format, tbi->dim1);
            break;

        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
            glTexStorage2D(tbi->type, tbi->mipmap, tbi->format, tbi->dim1, tbi->dim2);
            break; 

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glTexStorage3D(tbi->type, tbi->mipmap, tbi->format, tbi->dim1, tbi->dim2, tbi->dim3);
            break;

        case GL_TEXTURE_CUBE_MAP:
            glTexStorage3D(tbi->type, tbi->mipmap, tbi->format, tbi->dim1, tbi->dim2, 6);
            break;
        }
    };

    enqueue_task(func, {handle}, pikango::queue_type::general);
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

void pikango::cmd::write_texture_buffer(
    texture_buffer_handle   target,
    size_t                  mipmap_layer,
    texture_source_format   source_format,
    void*                   data,
    size_t                  off_1,
    size_t                  off_2,
    size_t                  off_3,
    size_t                  dim1,
    size_t                  dim2,
    size_t                  dim3
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

        auto dim1 = std::any_cast<size_t>(args[7]);
        auto dim2 = std::any_cast<size_t>(args[8]);
        auto dim3 = std::any_cast<size_t>(args[9]);

        auto tbi = pikango_internal::obtain_handle_object(handle);

        constexpr static GLuint cubemap_faces[] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };

        switch (tbi->type)
        {
        case GL_TEXTURE_1D:
            glBindTexture(tbi->type, tbi->id);
            glTexSubImage1D(
                tbi->type, mipmap, 
                off_1, 
                dim1, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;

        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
            glBindTexture(tbi->type, tbi->id);
            glTexSubImage2D(
                tbi->type, mipmap, 
                off_1, off_2, 
                dim1, dim2, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;

        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
            glBindTexture(tbi->type, tbi->id);
            glTexSubImage3D(
                tbi->type, mipmap, 
                off_1, off_2, off_3, 
                dim1, dim2, dim3, 
                format, GL_UNSIGNED_BYTE, data
            );
            break;

        case GL_TEXTURE_CUBE_MAP:
            auto cubemap_face = cubemap_faces[off_3 % 6];
            glBindTexture(cubemap_face, tbi->id);
            glTexSubImage2D(
                cubemap_face, mipmap, 
                off_1, off_2, 
                dim1, dim2, 
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
        dim1,
        dim2,
        dim3
    });
}
