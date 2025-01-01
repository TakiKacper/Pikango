PIKANGO_IMPL(vertex_shader)
{
    GLuint id = 0;
    ~vertex_shader_impl();
};

PIKANGO_NEW(vertex_shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::vertex_shader_impl);
    return handle;
};

pikango_internal::vertex_shader_impl::~vertex_shader_impl()
{
    delete_shader_generic(this);
}

void pikango::compile_vertex_shader(vertex_shader_handle target, const std::string& source)
{
    enqueue_task(compile_shader_part_task<vertex_shader_handle, GL_VERTEX_SHADER>, {target, source}, pikango::queue_type::general);
}