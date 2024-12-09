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
