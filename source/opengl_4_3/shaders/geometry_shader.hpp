PIKANGO_IMPL(geometry_shader)
{
    GLuint id = 0;
    ~geometry_shader_impl();
};

PIKANGO_NEW(geometry_shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::geometry_shader_impl);
    return handle;
};

pikango_internal::geometry_shader_impl::~geometry_shader_impl()
{
    delete_shader_generic(this);
}

void pikango::compile_geometry_shader(geometry_shader_handle target, const std::string& source)
{
    enqueue_task(compile_shader_task<geometry_shader_handle, GL_GEOMETRY_SHADER>, {target, source}, pikango::queue_type::general);
}
