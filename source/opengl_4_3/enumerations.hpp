#pragma once

//delete
#include "pikango/pikango.hpp"
#include "glad/glad.h"

GLuint get_buffer_usage_flag(pikango::buffer_memory_profile bmp, pikango::buffer_access_profile bap)
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

GLuint get_primitive(pikango::draw_primitive prim)
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

GLuint get_data_type(pikango::data_types type)
{
    switch (type)
    {
    case pikango::data_types::float32: 
    case pikango::data_types::vec2f32:
    case pikango::data_types::vec3f32:
    case pikango::data_types::vec4f32:
        return GL_FLOAT;

    case pikango::data_types::int32:   
        return GL_INT;
    }
    //will never happen
    return GL_FLOAT;
}

//returns 3 for vec3
//returns 4 for vec4
//etc
GLuint get_elements_in_data_type(pikango::data_types type)
{
    switch (type)
    {
    case pikango::data_types::float32: return 1;
    case pikango::data_types::int32:   return 1;
    
    case pikango::data_types::vec2f32: return 2;
    case pikango::data_types::vec3f32: return 3;
    case pikango::data_types::vec4f32: return 4;
    }
    //will never happen
    return 1;
}
