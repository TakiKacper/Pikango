PIKANGO_IMPL(pixel_shader)
{
    GLuint id = 0;
    ~pixel_shader_impl();
};

PIKANGO_NEW(pixel_shader)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::pixel_shader_impl);
    return handle;
};

pikango_internal::pixel_shader_impl::~pixel_shader_impl()
{
    delete_shader_generic(this);
}

void pikango::compile_pixel_shader(pixel_shader_handle target, const std::string& source)
{
    enqueue_task(compile_shader_part_task<pixel_shader_handle, GL_FRAGMENT_SHADER>, {target, source}, pikango::queue_type::general);
}
