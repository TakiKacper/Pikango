struct pikango_internal::buffer_impl
{
    GLuint id;
    size_t buffer_size;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
    ~buffer_impl();
};

pikango::buffer_handle pikango::new_buffer(const buffer_create_info& info)
{
    auto bi = new pikango_internal::buffer_impl;

    bi->id = 0;
    bi->buffer_size    = info.buffer_size_bytes;
    bi->memory_profile = info.memory_profile;
    bi->access_profile = info.access_profile;

    auto handle = pikango_internal::make_handle(bi);

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<buffer_handle>(args[0]);
        auto bi = pikango_internal::obtain_handle_object(handle);

        glGenBuffers(1, &bi->id);

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferData(GL_COPY_WRITE_BUFFER, bi->buffer_size, nullptr, get_buffer_usage_flag(bi->memory_profile, bi->access_profile));   
    };
    
    enqueue_task(func, {handle}, pikango::queue_type::general);
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

void pikango::cmd::bind_vertex_buffer(buffer_handle vertex_buffer, size_t binding)
{
    auto func = [](std::vector<std::any> args)
    {
        auto vertex_buffer = std::any_cast<buffer_handle>(args[0]);
        auto binding       = std::any_cast<size_t>(args[1]);

        cmd_bindings::vertex_buffers.at(binding) = 
            pikango_internal::obtain_handle_object(vertex_buffer)->id;
        cmd_bindings::vertex_buffers_changed = true;
    };

    record_task(func, {vertex_buffer, binding});
}

void pikango::cmd::bind_index_buffer(buffer_handle index_buffer)
{
    auto func = [](std::vector<std::any> args)
    {
        auto index_buffer = std::any_cast<buffer_handle>(args[0]);

        cmd_bindings::index_buffer = pikango_internal::obtain_handle_object(index_buffer)->id;
        cmd_bindings::index_buffer_changed = true;
    };

    record_task(func, {index_buffer});
}

void pikango::cmd::bind_uniform_buffer(
    buffer_handle uniform_buffer,
    size_t slot,
    size_t size,
    size_t offset
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto uniform_buffer = std::any_cast<buffer_handle>(args[0]);
        auto slot   = std::any_cast<size_t>(args[1]);
        auto offset = std::any_cast<size_t>(args[2]);
        auto size   = std::any_cast<size_t>(args[3]);

        auto ubi = pikango_internal::obtain_handle_object(uniform_buffer);

        glBindBufferRange(
            GL_UNIFORM_BUFFER, 
            slot, 
            ubi->id, 
            offset, 
            size
        );
    };

    record_task(func, {uniform_buffer, slot, offset, size});
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
