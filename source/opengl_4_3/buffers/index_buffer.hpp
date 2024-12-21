PIKANGO_IMPL(index_buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
    ~index_buffer_impl();
};

PIKANGO_NEW(index_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::index_buffer_impl);
    return handle;
}

PIKANGO_DELETE(index_buffer)
{ 

};

pikango_internal::index_buffer_impl::~index_buffer_impl()
{
    destroy_buffer_generic(this);
}

size_t pikango::get_buffer_size(index_buffer_handle target)
{
    auto bi = pikango_internal::object_read_access(target);
    return bi->buffer_size;
}

void pikango::assign_buffer_memory(
    index_buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    assign_buffer_memory_generic<index_buffer_handle>(
        target, 
        memory_block_size_bytes, 
        memory_profile, 
        access_profile
    );
}

void pikango::write_buffer(index_buffer_handle target, size_t data_size_bytes, void* data)
{
    write_buffer_memory_generic<index_buffer_handle>(target, data_size_bytes, data);
}

void pikango::write_buffer_region(index_buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    write_buffer_memory_region_generic<index_buffer_handle>(target, data_size_bytes, data, data_offset_bytes);
}
