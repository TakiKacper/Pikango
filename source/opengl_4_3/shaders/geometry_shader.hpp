PIKANGO_IMPL(geometry_shader)
{
    GLuint id = 0;
    shader_uniforms_to_descriptors_maping desc_mapping;
    ~geometry_shader_impl();
};

PIKANGO_NEW(geometry_shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::geometry_shader_impl);
    return handle;
};

pikango_internal::geometry_shader_impl::~geometry_shader_impl()
{
    delete_dangling_program_pipelines<offsetof(graphics_shaders_pipeline_config_impl_ptr_identifier, geometry_shader_impl_ptr)>(this);
    delete_shader_generic(this);
}

void pikango::compile_geometry_shader(geometry_shader_handle target, const std::string& source)
{
    enqueue_task(compile_shader_task<geometry_shader_handle, GL_GEOMETRY_SHADER>, {target, source}, pikango::queue_type::general);
}

void pikango::OPENGL_ONLY_link_shader_bindings_info(geometry_shader_handle target, OPENGL_ONLY_shader_bindings& bindings)
{
    OPENGL_ONLY_link_shader_bindings_info_generic(target, bindings);
}
