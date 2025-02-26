#pragma once

GLenum get_buffer_usage_flag(pikango::buffer_memory_profile bmp, pikango::buffer_access_profile bap)
{
    switch (bmp)
    {
    case pikango::buffer_memory_profile::rare_write_rare_read:
        switch (bap)
        {
        case pikango::buffer_access_profile::cpu_to_gpu:
            return GL_STREAM_DRAW;
        case pikango::buffer_access_profile::gpu_to_cpu:
            return GL_STREAM_READ;
        case pikango::buffer_access_profile::gpu_to_gpu:
            return GL_STREAM_COPY;
        }
    case pikango::buffer_memory_profile::rare_write_often_read:
        switch (bap)
        {
        case pikango::buffer_access_profile::cpu_to_gpu:
            return GL_STATIC_DRAW;
        case pikango::buffer_access_profile::gpu_to_cpu:
            return GL_STATIC_READ;
        case pikango::buffer_access_profile::gpu_to_gpu:
            return GL_STATIC_COPY;
        }
    case pikango::buffer_memory_profile::often_write_often_read:
        switch (bap)
        {
        case pikango::buffer_access_profile::cpu_to_gpu:
            return GL_DYNAMIC_DRAW;
        case pikango::buffer_access_profile::gpu_to_cpu:
            return GL_DYNAMIC_READ;
        case pikango::buffer_access_profile::gpu_to_gpu:
            return GL_DYNAMIC_COPY;
        }
    }
    //will never happen
    return GL_DYNAMIC_DRAW;
}

GLenum get_primitive(pikango::draw_primitive prim)
{
    switch (prim)
    {
    case pikango::draw_primitive::points:           return  GL_POINTS;
    case pikango::draw_primitive::lines:            return  GL_LINES;
    case pikango::draw_primitive::lines_loop:       return  GL_LINE_LOOP;
    case pikango::draw_primitive::line_strip:       return  GL_LINE_STRIP;
    case pikango::draw_primitive::traingles:        return  GL_TRIANGLES;
    case pikango::draw_primitive::traingles_strip:  return  GL_TRIANGLE_STRIP;
    }
    //will never happen
    return GL_TRIANGLES;
}

GLenum get_data_type(pikango::data_type type)
{
    switch (type)
    {
    case pikango::data_type::float32: 
    case pikango::data_type::vec2f32:
    case pikango::data_type::vec3f32:
    case pikango::data_type::vec4f32:
        return GL_FLOAT;

    case pikango::data_type::int32:   
    case pikango::data_type::vec2i32:
    case pikango::data_type::vec3i32:
    case pikango::data_type::vec4i32:
        return GL_INT;
    }
    //will never happen
    return GL_FLOAT;
}

//returns 3 for vec3
//returns 4 for vec4
//etc
GLuint get_elements_in_data_type(pikango::data_type type)
{
    switch (type)
    {
    case pikango::data_type::float32: return 1;
    case pikango::data_type::int32:   return 1;
    
    case pikango::data_type::vec2f32: 
    case pikango::data_type::vec2i32: 
        return 2;

    case pikango::data_type::vec3f32: 
    case pikango::data_type::vec3i32: 
        return 3;

    case pikango::data_type::vec4f32: 
    case pikango::data_type::vec4i32: 
        return 4;
    }
    //will never happen
    return 1;
}

GLenum get_texture_sized_format(pikango::texture_sized_format format)
{
    switch (format) {
        case pikango::texture_sized_format::r8:       return GL_R8;
        case pikango::texture_sized_format::r16:      return GL_R16;

        case pikango::texture_sized_format::rg8:      return GL_RG8;
        case pikango::texture_sized_format::rg16:     return GL_RG16;

        case pikango::texture_sized_format::r3_g3_b2: return GL_R3_G3_B2;
        case pikango::texture_sized_format::rgb4:     return GL_RGB4;
        case pikango::texture_sized_format::rgb5:     return GL_RGB5;
        case pikango::texture_sized_format::rgb8:     return GL_RGB8;
        case pikango::texture_sized_format::rgb10:    return GL_RGB10;
        case pikango::texture_sized_format::rgb12:    return GL_RGB12;

        case pikango::texture_sized_format::rgba2:    return GL_RGBA2;
        case pikango::texture_sized_format::rgba4:    return GL_RGBA4;
        case pikango::texture_sized_format::rgba8:    return GL_RGBA8;
        case pikango::texture_sized_format::rgba12:   return GL_RGBA12;
        case pikango::texture_sized_format::rgba16:   return GL_RGBA16;
        case pikango::texture_sized_format::rgba32f:  return GL_RGBA32F;
    }
    //will never happen
    return GL_RGBA32F;
}

GLenum get_format_shader(pikango::shader_type type)
{
    switch (type)
    {
        case pikango::shader_type::vertex: return GL_VERTEX_SHADER;
        case pikango::shader_type::pixel: return GL_FRAGMENT_SHADER;
        case pikango::shader_type::geometry: return GL_GEOMETRY_SHADER;
    }
    //will never happen
    return GL_VERTEX_SHADER;
}

GLenum get_texture_source_format(pikango::texture_source_format format) {
    switch (format) {
        case pikango::texture_source_format::r:        return GL_RED;
        case pikango::texture_source_format::rg:       return GL_RG;
        case pikango::texture_source_format::rgb:      return GL_RGB;
        case pikango::texture_source_format::rgba:     return GL_RGBA;
    }
    //will never happen
    return GL_RED;
}

GLenum get_texture_wraping(pikango::texture_wraping wraping)
{
    switch (wraping)
    {
        case pikango::texture_wraping::repeat:          return GL_REPEAT;
        case pikango::texture_wraping::mirror_repeat:   return GL_MIRRORED_REPEAT;
        case pikango::texture_wraping::clamp_coords:    return GL_CLAMP_TO_BORDER; 
        case pikango::texture_wraping::clamp_texture:   return GL_CLAMP_TO_EDGE;
    }
    //will never happen
    return GL_REPEAT;
}

GLenum get_texture_filtering(pikango::texture_filtering filtering)
{
    switch (filtering)
    {
        case pikango::texture_filtering::linear: return GL_LINEAR;
        case pikango::texture_filtering::nearest: return GL_NEAREST;
    }
    //will never happen
    return GL_LINEAR;
}

GLenum combine_min_filters(pikango::texture_filtering min, pikango::texture_filtering mipmap)
{
    if      (min == pikango::texture_filtering::linear && mipmap == pikango::texture_filtering::linear)
        return GL_LINEAR_MIPMAP_LINEAR;
    else if (min == pikango::texture_filtering::linear && mipmap == pikango::texture_filtering::nearest)
        return GL_LINEAR_MIPMAP_NEAREST;
    else if (min == pikango::texture_filtering::nearest && mipmap == pikango::texture_filtering::linear)
        return GL_NEAREST_MIPMAP_LINEAR;
    else if (min == pikango::texture_filtering::nearest && mipmap == pikango::texture_filtering::nearest)
        return GL_NEAREST_MIPMAP_NEAREST;
    
    //will never happen
    return GL_LINEAR_MIPMAP_LINEAR;
}

GLenum get_rasterization_fill(pikango::rasterization_polygon_fill_mode mode)
{
    switch (mode)
    {
    case pikango::rasterization_polygon_fill_mode::fill_shape:
        return GL_FILL;
    case pikango::rasterization_polygon_fill_mode::fill_edges:
        return GL_LINE; 
    case pikango::rasterization_polygon_fill_mode::fill_vertices:
        return GL_POINT; 
    }
    //will never happen
    return GL_FILL;
}

GLenum get_front_face(pikango::rasterization_culling_front_face face)
{
    switch (face)
    {
    case pikango::rasterization_culling_front_face::clockwise:
        return GL_CW;
    case pikango::rasterization_culling_front_face::couter_clockwise:
        return GL_CCW;
    }
    //will never happen
    return GL_CCW;
}

GLenum get_culling_mode(pikango::rasterization_culling_mode mode)
{
    switch (mode)
    {
    case pikango::rasterization_culling_mode::none:
        return GL_NONE;
    case pikango::rasterization_culling_mode::front:
        return GL_FRONT;
    case pikango::rasterization_culling_mode::back:
        return GL_BACK;
    case pikango::rasterization_culling_mode::front_and_back:
        return GL_FRONT_AND_BACK;
    }
    //will never happen
    return GL_NONE;
}
