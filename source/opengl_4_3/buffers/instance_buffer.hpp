PIKANGO_IMPL(instance_buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
};

PIKANGO_NEW(instance_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::instance_buffer_impl);
    auto ibi = pikango_internal::object_write_access(handle);
    ibi->buffer_size = 0;
    return handle;
}

PIKANGO_DELETE(instance_buffer)
{ 

};

size_t pikango::get_buffer_size(instance_buffer_handle target)
{
    auto bi = pikango_internal::object_read_access(target);
    return bi->buffer_size;
}

void pikango::assign_buffer_memory(
    instance_buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    assign_buffer_memory_generic<instance_buffer_handle>(
        target, 
        memory_block_size_bytes, 
        memory_profile, 
        access_profile
    );
}

void pikango::write_buffer(instance_buffer_handle target, size_t data_size_bytes, void* data)
{
    write_buffer_memory_generic<instance_buffer_handle>(target, data_size_bytes, data);
}

void pikango::write_buffer_region(instance_buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    write_buffer_memory_region_generic<instance_buffer_handle>(target, data_size_bytes, data, data_offset_bytes);
}
