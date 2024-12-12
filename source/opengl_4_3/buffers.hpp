/*
    Implementations
*/

PIKANGO_IMPL(vertex_buffer)
{
    GLuint id = 0;
    size_t buffer_size = 0;
    pikango::buffer_memory_profile memory_profile;
    pikango::buffer_access_profile access_profile;
};

PIKANGO_NEW(vertex_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::vertex_buffer_impl);
    auto vbi = pikango_internal::object_write_access(handle);
    vbi->buffer_size = 0;
    return handle;
}

PIKANGO_DELETE(vertex_buffer)
{ 

};

/*
    Methods
*/

size_t pikango::get_buffer_size(vertex_buffer_handle target)
{
    auto vbi = pikango_internal::object_read_access(target);
    return vbi->buffer_size;
}

void pikango::assign_buffer_memory(
    vertex_buffer_handle target,
    size_t memory_block_size_bytes, 
    buffer_memory_profile memory_profile, 
    buffer_access_profile access_profile
)
{
    auto vbi = pikango_internal::object_write_access(target);

    //if size == 0
    //todo error

    vbi->buffer_size    = memory_block_size_bytes;
    vbi->memory_profile = memory_profile;
    vbi->access_profile = access_profile;

    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::vertex_buffer_handle>(args[0]);
        auto size   = std::any_cast<size_t>(args[1]);
        auto memory = std::any_cast<buffer_memory_profile>(args[2]);
        auto access = std::any_cast<buffer_access_profile>(args[3]);

        auto vbi = pikango_internal::object_write_access(handle);
        if (vbi->id == 0)
            glGenBuffers(1, &vbi->id);

        glBindBuffer(GL_COPY_WRITE_BUFFER, vbi->id);
        glBufferData(GL_COPY_WRITE_BUFFER, size, nullptr, get_buffer_usage_flag(memory, access));   
    };
    enqueue_task(func, {target, memory_block_size_bytes, memory_profile, access_profile});
}

void pikango::write_buffer(vertex_buffer_handle target, size_t data_size_bytes, void* data)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::vertex_buffer_handle>(args[0]);
        auto size = std::any_cast<size_t>(args[1]);
        auto data = std::any_cast<void*>(args[2]);

        auto vbi = pikango_internal::object_read_access(handle);
        
        //if (vbi->id == 0)
        //todo error
        
        //if (size > vbi->buffer_size)
        //todo error

        //if (data == nullptr)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, vbi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, 0, size, data);
    };
    
    enqueue_task(func, {target, data_size_bytes, data});
}

void pikango::write_buffer_region(vertex_buffer_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes)
{
    auto func = [](std::vector<std::any> args)
    {
        auto handle = std::any_cast<pikango::vertex_buffer_handle>(args[0]);
        auto size = std::any_cast<size_t>(args[1]);
        auto data = std::any_cast<void*>(args[2]);
        auto offset = std::any_cast<size_t>(args[3]);

        auto vbi = pikango_internal::object_read_access(handle);
        
        //if (vbi->id == 0)
        //todo error
        
        //if (size + offset > vbi->buffer_size)
        //todo error

        //if (data == nullptr)
        //todo error

        glBindBuffer(GL_COPY_WRITE_BUFFER, vbi->id);
        glBufferSubData(GL_COPY_WRITE_BUFFER, offset, size, data);
    };
    
    enqueue_task(func, {target, data_size_bytes, data, data_offset_bytes});
}
