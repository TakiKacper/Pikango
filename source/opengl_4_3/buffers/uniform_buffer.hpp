PIKANGO_IMPL(uniform_buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
    ~uniform_buffer_impl();
};

PIKANGO_NEW(uniform_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::uniform_buffer_impl);
    return handle;
}

PIKANGO_DELETE(uniform_buffer)
{ 

};

pikango_internal::uniform_buffer_impl::~uniform_buffer_impl()
{
    destroy_buffer_generic(this);
}

size_t pikango::get_buffer_size(uniform_buffer_handle target)
{
    auto bi = pikango_internal::object_read_access(target);
    return bi->buffer_size;
}

void pikango::assign_buffer_memory(
    uniform_buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    assign_buffer_memory_generic<uniform_buffer_handle>(
        target, 
        memory_block_size_bytes, 
        memory_profile, 
        access_profile
    );
}

void pikango::write_buffer(uniform_buffer_handle target, size_t data_size_bytes, void* data)
{
    write_buffer_memory_generic<uniform_buffer_handle>(target, data_size_bytes, data);
}

void pikango::write_buffer_region(uniform_buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    write_buffer_memory_region_generic<uniform_buffer_handle>(target, data_size_bytes, data, data_offset_bytes);
}

void pikango::bind_uniform_buffer_to_pool(uniform_buffer_handle target, size_t pool_index)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<uniform_buffer_handle>(args[0]);
        auto index = std::any_cast<size_t>(args[1]);

        auto bi = pikango_internal::object_read_access(handle);
        glBindBufferBase(GL_UNIFORM_BUFFER, index, bi->id);
    };
    record_task(func, {target, pool_index});
}
