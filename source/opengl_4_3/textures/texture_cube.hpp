PIKANGO_IMPL(texture_cube)
{
    GLuint id = 0;
    size_t width = 0;

    ~texture_cube_impl();
};

PIKANGO_NEW(texture_cube)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::texture_cube_impl);
    auto ti = pikango_internal::object_write_access(handle);
    ti->id = 0;
    return handle;
};

pikango_internal::texture_cube_impl::~texture_cube_impl()
{
    delete_texture(this);
}

void pikango::cmd::write_texture(
    texture_cube_handle target, 
    texture_format source_format, 
    texture_format inner_format, 
    size_t width, 
    void* top, 
    void* bottom, 
    void* left, 
    void* right, 
    void* front,
    void* back
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<texture_cube_handle>(args[0]);
        auto texture_size = std::any_cast<size_t>(args[1]);
        auto source_format = std::any_cast<GLuint>(args[2]);
        auto inner_format = std::any_cast<GLuint>(args[3]);
        auto top = std::any_cast<void*>(args[4]);
        auto bottom = std::any_cast<void*>(args[5]);
        auto left = std::any_cast<void*>(args[6]);
        auto right = std::any_cast<void*>(args[7]);
        auto front = std::any_cast<void*>(args[8]);
        auto back = std::any_cast<void*>(args[9]);

        auto ti = pikango_internal::object_write_access(handle);

        if (ti->id == 0)
            glGenTextures(1, &ti->id);
        glBindTexture(GL_TEXTURE_CUBE_MAP, ti->id);

        GLenum face_targets[6] = {
            GL_TEXTURE_CUBE_MAP_POSITIVE_Y,  // Top
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,  // Bottom
            GL_TEXTURE_CUBE_MAP_NEGATIVE_X,  // Left
            GL_TEXTURE_CUBE_MAP_POSITIVE_X,  // Right
            GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,  // Front
            GL_TEXTURE_CUBE_MAP_POSITIVE_Z   // Back
        };

        void* faces[] = {top, bottom, left, right, front, back};

        for (int i = 0; i < 6; ++i)
            glTexImage2D(face_targets[i], 0, inner_format, texture_size, texture_size, 0, source_format, GL_UNSIGNED_BYTE, faces[i]);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    };

    record_task(func, {target, get_texture_format(source_format), get_texture_format(inner_format), width, top, bottom, left, right, front, back});
}

void pikango::cmd::bind_texture_to_pool(texture_cube_handle target, size_t index)
{
    bind_texture_to_pool_generic<texture_cube_handle, GL_TEXTURE_CUBE_MAP>(target, index);
}

void pikango::set_texture_wraping(texture_cube_handle target, texture_wraping x, texture_wraping y, texture_wraping z)
{
    set_texture_wraping_3d_generic<texture_cube_handle, GL_TEXTURE_CUBE_MAP>(target, x, y, z);
}

void pikango::set_texture_filtering(texture_cube_handle target, texture_filtering magnifying, texture_filtering minifying, texture_filtering mipmap)
{
    set_texture_filtering_generic<texture_cube_handle, GL_TEXTURE_CUBE_MAP>(target, magnifying, minifying, mipmap);
}
