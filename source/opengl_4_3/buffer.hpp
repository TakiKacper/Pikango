PIKANGO_IMPL(buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
    ~buffer_impl();
};

PIKANGO_NEW(buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::buffer_impl);
    return handle;
}

pikango_internal::buffer_impl::~buffer_impl()
{
    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteBuffers(1, &id);
    };

    if (id != 0) enqueue_task(func, {id}, pikango::queue_type::general);
}

size_t pikango::get_buffer_size(buffer_handle target)
{
    auto bi = pikango_internal::obtain_handle_object(target);
    return bi->buffer_size;
}

void pikango::cmd::assign_buffer_memory(
    buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    auto bi = pikango_internal::obtain_handle_object(target);

    bi->buffer_size    = memory_block_size_bytes;
    bi->memory_profile = memory_profile;
    bi->access_profile = access_profile;

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<buffer_handle>(args[0]);
        auto size   = std::any_cast<size_t>(args[1]);
        auto memory = std::any_cast<pikango::buffer_memory_profile>(args[2]);
        auto access = std::any_cast<pikango::buffer_access_profile>(args[3]);

        auto bi = pikango_internal::obtain_handle_object(handle);
        if (bi->id == 0)
            glGenBuffers(1, &bi->id);

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, get_buffer_usage_flag(memory, access));   
    };
    
    record_task(func, {target, memory_block_size_bytes, memory_profile, access_profile});
}

void pikango::cmd::write_buffer(buffer_handle target, size_t data_size_bytes, void* data)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<buffer_handle>(args[0]);
        auto size = std::any_cast<size_t>(args[1]);
        auto data = std::any_cast<void*>(args[2]);

        auto bi = pikango_internal::obtain_handle_object(handle);

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, data);
    };
    
    record_task(func, {target, data_size_bytes, data});
}

void pikango::cmd::write_buffer_region(buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<buffer_handle>(args[0]);
        auto size   = std::any_cast<size_t>(args[1]);
        auto data   = std::any_cast<void*>(args[2]);
        auto offset = std::any_cast<size_t>(args[3]);

        auto bi = pikango_internal::obtain_handle_object(handle);

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
    };
    
    record_task(func, {target, data_size_bytes, data, data_offset_bytes});
}

void pikango::cmd::copy_buffer_to_buffer(
    buffer_handle source, 
    buffer_handle destination,
    size_t read_offset, 
    size_t read_size, 
    size_t write_offset
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto source         = std::any_cast<buffer_handle>(args[0]);
        auto destination    = std::any_cast<buffer_handle>(args[1]);
        auto read_offset    = std::any_cast<size_t>(args[2]);
        auto read_size      = std::any_cast<size_t>(args[3]);
        auto write_offset   = std::any_cast<size_t>(args[4]);

        auto sbi = pikango_internal::obtain_handle_object(source);
        auto dbi = pikango_internal::obtain_handle_object(destination);

        glBindBuffer(GL_COPY_READ_BUFFER, sbi->id);
        glBindBuffer(GL_COPY_WRITE_BUFFER, dbi->id);

        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, read_offset, write_offset, read_size);
    };

    record_task(func, {source, destination, read_offset, read_size, write_offset});
}
