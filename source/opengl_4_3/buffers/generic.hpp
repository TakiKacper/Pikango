template <class impl_type>
void destroy_buffer_generic(impl_type* _this)
{
    auto func = [](std::vector<std::any> args)
    {
        auto id = std::any_cast<GLuint>(args[0]);
        glDeleteBuffers(1, &id);
    };

    if (_this->id != 0)
        enqueue_task(func, {_this->id}, pikango::queue_type::general);
}

template<class handle_type>
void assign_buffer_memory_generic(
    handle_type target,
    size_t memory_block_size_bytes, 
    pikango::buffer_memory_profile memory_profile, 
    pikango::buffer_access_profile access_profile
)
{
    auto bi = pikango_internal::object_write_access(target);

    //if size == 0
    //todo error

    bi->buffer_size    = memory_block_size_bytes;
    bi->memory_profile = memory_profile;
    bi->access_profile = access_profile;

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto size   = std::any_cast<size_t>(args[1]);
        auto memory = std::any_cast<pikango::buffer_memory_profile>(args[2]);
        auto access = std::any_cast<pikango::buffer_access_profile>(args[3]);

        auto bi = pikango_internal::object_write_access(handle);
        if (bi->id == 0)
            glGenBuffers(1, &bi->id);

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, get_buffer_usage_flag(memory, access));   
    };
    
    record_task(func, {target, memory_block_size_bytes, memory_profile, access_profile});
}

template<class handle_type>
void write_buffer_memory_generic(
    handle_type target, 
    size_t data_size_bytes, 
    void* data
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto size = std::any_cast<size_t>(args[1]);
        auto data = std::any_cast<void*>(args[2]);

        auto bi = pikango_internal::object_read_access(handle);
        
        //if (bi->id == 0)
        //todo error
        
        //if (size > vbi->buffer_size)
        //todo error

        //if (data == nullptr)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, data);
    };
    
    record_task(func, {target, data_size_bytes, data});
}

template<class handle_type>
void write_buffer_memory_region_generic(
    handle_type target, 
    size_t data_size_bytes,
    void* data, 
    size_t data_offset_bytes
)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<handle_type>(args[0]);
        auto size   = std::any_cast<size_t>(args[1]);
        auto data   = std::any_cast<void*>(args[2]);
        auto offset = std::any_cast<size_t>(args[3]);

        auto bi = pikango_internal::object_read_access(handle);
        
        //if (bi->id == 0)
        //todo error
        
        //if (size + offset > bi->buffer_size)
        //todo error

        //if (data == nullptr)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, bi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
    };
    
    record_task(func, {target, data_size_bytes, data, data_offset_bytes});
}