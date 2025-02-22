PIKANGO_IMPL(texture_buffer)
{
    GLuint id = 0;
    GLuint texture_type = 0;

    size_t width = 0;
    size_t height = 0;
    size_t depth = 0;

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
#include <iostream>
void pikango::cmd::assign_texture_buffer_memory(
    texture_buffer_handle   target,
    texture_type            type,
    texture_sized_format    memory_format,
    size_t                  mipmap_layers,
    size_t                  array_layers,
    size_t                  width,
    size_t                  height,
    size_t                  depth
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_buffer_handle>(args[0]);
        auto type   = std::any_cast<texture_type>(args[1]);

        auto format = std::any_cast<GLuint>(args[2]);

        size_t mipmap = std::any_cast<size_t>(args[3]);
        size_t array  = std::any_cast<size_t>(args[4]);

        size_t width  = std::any_cast<size_t>(args[5]);
        size_t height = std::any_cast<size_t>(args[6]);
        size_t depth  = std::any_cast<size_t>(args[7]);

        auto tbi = pikango_internal::obtain_handle_object(handle);

        if (tbi->id == 0) glGenTextures(1, &tbi->id);

        //pikango_internal::log_error(infoLog);

        switch (type)
        {
        case pikango::texture_type::texture_1d:
            if (array <= 1)
            {
                glBindTexture(GL_TEXTURE_1D, tbi->id);
                glTexStorage1D(GL_TEXTURE_1D, mipmap, format, width);
                tbi->texture_type = GL_TEXTURE_1D;
            }
            else
            {
                glBindTexture(GL_TEXTURE_1D_ARRAY, tbi->id);
                glTexStorage2D(GL_TEXTURE_1D_ARRAY, mipmap, format, width, array);
                tbi->texture_type = GL_TEXTURE_1D_ARRAY;
            }
            break;
        case pikango::texture_type::texture_2d:
            if (array <= 1)
            {
                glBindTexture(GL_TEXTURE_2D, tbi->id);
                glTexStorage2D(GL_TEXTURE_2D, mipmap, format, width, height);
                tbi->texture_type = GL_TEXTURE_2D;
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D_ARRAY, tbi->id);
                glTexStorage3D(GL_TEXTURE_2D_ARRAY, mipmap, format, width, height, array);
                tbi->texture_type = GL_TEXTURE_2D_ARRAY;
            }
            break;
        case pikango::texture_type::texture_3d:
            glBindTexture(GL_TEXTURE_3D, tbi->id);
            glTexStorage3D(GL_TEXTURE_3D, mipmap, format, width, height, depth);
            tbi->texture_type = GL_TEXTURE_3D;
            break;
        case pikango::texture_type::texture_cubemap:
            glBindTexture(GL_TEXTURE_CUBE_MAP, tbi->id);
            glTexStorage3D(GL_TEXTURE_CUBE_MAP, mipmap, format, width, height, 6);
            tbi->texture_type = GL_TEXTURE_CUBE_MAP;
            break;
        }
    };

    record_task(func, {
        target,
        type,
        get_texture_sized_format(memory_format),
        mipmap_layers,
        array_layers,
        width,
        height,
        depth
    });
}

void pikango::cmd::write_texture_buffer(
    texture_buffer_handle   target,
    size_t                  mipmap_layer,
    size_t                  array_layer,
    texture_source_format   source_format,
    size_t                  data_width,
    size_t                  data_height,
    size_t                  data_depth,
    void*                   data,
    size_t                  write_offset_width,
    size_t                  write_offset_height,
    size_t                  write_offset_depth
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_buffer_handle>(args[0]);

        auto mipmap = std::any_cast<size_t>(args[1]);
        auto array = std::any_cast<size_t>(args[2]);

        auto format = std::any_cast<GLuint>(args[3]);

        auto width = std::any_cast<size_t>(args[4]);
        auto height = std::any_cast<size_t>(args[5]);
        auto depth = std::any_cast<size_t>(args[6]);

        auto data = std::any_cast<void*>(args[7]);

        auto offset_x = std::any_cast<size_t>(args[8]);
        auto offset_y = std::any_cast<size_t>(args[9]);
        auto offset_z = std::any_cast<size_t>(args[10]);

        auto tbi = pikango_internal::obtain_handle_object(handle);

        //todo
        glBindTexture(GL_TEXTURE_2D, tbi->id);
        glTexSubImage2D(GL_TEXTURE_2D, mipmap, offset_x, offset_y, width, height, format, GL_UNSIGNED_BYTE, data);
    };

    record_task(func, {
        target,
        mipmap_layer,
        array_layer,
        get_texture_source_format(source_format),
        data_width,
        data_height,
        data_depth,
        data,
        write_offset_width,
        write_offset_height,
        write_offset_depth
    });
}
