PIKANGO_IMPL(vertex_buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
    ~vertex_buffer_impl();
};

PIKANGO_NEW(vertex_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::vertex_buffer_impl);
    return handle;
}

pikango_internal::vertex_buffer_impl::~vertex_buffer_impl()
{
    delete_buffer_generic(this);
}

size_t pikango::get_buffer_size(vertex_buffer_handle target)
{
    auto bi = pikango_internal::obtain_handle_object(target);
    return bi->buffer_size;
}

void pikango::cmd::assign_buffer_memory(
    vertex_buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    assign_buffer_memory_generic<vertex_buffer_handle>(
        target, 
        memory_block_size_bytes, 
        memory_profile, 
        access_profile
    );
}

void pikango::cmd::write_buffer(vertex_buffer_handle target, size_t data_size_bytes, void* data)
{
    write_buffer_memory_generic<vertex_buffer_handle>(target, data_size_bytes, data);
}

void pikango::cmd::write_buffer_region(vertex_buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    write_buffer_memory_region_generic<vertex_buffer_handle>(target, data_size_bytes, data, data_offset_bytes);
}
