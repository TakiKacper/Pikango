#pragma once
#ifndef PIKANGO_HPP
#define PIKANGO_HPP

#include <vector>
#include <string>
#include <functional>
#include <any>

/*
    Enumerations
*/

namespace pikango
{
    enum class draw_primitive
    {
        points, 
        lines, lines_loop, line_strip,
        traingles, traingles_strip
    };

    enum class buffer_memory_profile
    {
        rare_write_rare_read,   //data is rarely in use
        rare_write_often_read,  //data is often overwritten and often used
        often_write_often_read  //data is rarely overwritten and often used
    };

    enum class buffer_access_profile
    {
        cpu_to_gpu, //application writes data to gpu
        gpu_to_cpu, //gpu data is read by application 
        gpu_to_gpu  //gpu data is read by gpu
    };

    enum class data_types
    {
        int32,
        float32,
        vec2f32,
        vec3f32,
        vec4f32
    };
    size_t size_of(data_types dt);

    enum class texture_filtering
	{
		nearest, linear
	};

    enum class texture_wraping
	{
		repeat, 
        mirror_repeat,
		clamp_coords, 
        clamp_texture
	};

    enum class texture_format
	{
		depth,  depth_stencil,	depth24_stencil8,
		r,		r8, r16,
		rg,		rg8, rg16,
		rgb,	r3_g3_b2, rgb4, rgb5, rgb8, rgb10, rgb12, 
		rgba,	rgba2, rgba4, rgba8, rgba12, rgba16, rgba32f
	};
}

/*
    Handles
*/

#include "pikango_handle.hpp"

/*
This macro for name = xyz would create folowing objects:

    pikango_internal::xyz_impl  (forward)
    
    pikango::xyz_handle         (handle specialization)
    pikango::new_xyz            (function)
    pikango::delete_xyz         (function)
*/
#define PIKANGO_HANDLE_FWD(name)     \
    namespace pikango_internal   \
    {                            \
        struct name##_impl;      \
    }                            \
                                 \
    namespace pikango            \
    {                            \
        using name##_handle = pikango_internal::handle<pikango_internal::name##_impl>;    \
        name##_handle new_##name ();            \
        void delete_##name (name##_handle);     \
    }

PIKANGO_HANDLE_FWD(vertex_buffer);
PIKANGO_HANDLE_FWD(index_buffer);
PIKANGO_HANDLE_FWD(instance_buffer);
PIKANGO_HANDLE_FWD(uniform_buffer);

PIKANGO_HANDLE_FWD(data_layout);

PIKANGO_HANDLE_FWD(graphics_shader);

PIKANGO_HANDLE_FWD(texture_1d);
PIKANGO_HANDLE_FWD(texture_2d);
PIKANGO_HANDLE_FWD(texture_3d);
PIKANGO_HANDLE_FWD(texture_cube);
PIKANGO_HANDLE_FWD(texture_1d_array);
PIKANGO_HANDLE_FWD(texture_2d_array);

PIKANGO_HANDLE_FWD(renderbuffer);
PIKANGO_HANDLE_FWD(frame_buffer);

#undef PIKANGO_HANDLE_FWD

/*
    Library Functions
*/
namespace pikango
{
    using error_notification_callback = void(*)(const char* notification);

    std::string initialize_library_cpu();
    std::string initialize_library_gpu();
    std::string terminate();

    template<class T>
    bool handle_good(const pikango_internal::handle<T>& handle)
    {
        return !pikango_internal::is_empty(handle);
    }

    const char* get_used_shading_language_name();

    void wait_all_tasks_completion();
    void wait_all_current_tasks_completion();

    void set_error_notification_callback(error_notification_callback callback);
}

/*
    Buffers Methods
*/

#define BUFFER_METHODS(buffer_name) \
    size_t get_buffer_size(buffer_name##_handle target); \
    void assign_buffer_memory(buffer_name##_handle target, size_t memory_block_size_bytes, buffer_memory_profile memory_profile, buffer_access_profile access_profile);   \
    void write_buffer(buffer_name##_handle target, size_t data_size_bytes, void* data);   \
    void write_buffer_region(buffer_name##_handle target, size_t data_size_bytes, void* data, size_t data_offset_bytes);   \

namespace pikango
{
    BUFFER_METHODS(vertex_buffer);
    BUFFER_METHODS(index_buffer);
    BUFFER_METHODS(instance_buffer);
    BUFFER_METHODS(uniform_buffer);

    size_t get_uniform_pool_size();
    void bind_uniform_buffer_to_pool(uniform_buffer_handle target, size_t pool_index);
}

#undef BUFFER_METHODS

/*
    Data Layout Methods
*/

namespace pikango
{
    size_t get_layout_size(data_layout_handle target);
    size_t get_layout_size_with_stride(data_layout_handle target);
    void assign_data_layout(data_layout_handle target, std::vector<data_types> layout, size_t layouts_stride);
}

/*
    Shaders Methods
*/

namespace pikango
{
    struct shader_part_vertex;
    struct shader_part_geometry;
    struct shader_part_pixel;

    shader_part_vertex*     compile_shader_part_vertex(const std::string& source);
    shader_part_geometry*   compile_shader_part_geometry(const std::string& source);
    shader_part_pixel*      compile_shader_part_pixel(const std::string& source);

    void free_shader_part_vertex(shader_part_vertex* spv);
    void free_shader_part_geometry(shader_part_geometry* spg);
    void free_shader_part_pixel(shader_part_pixel* spp);

    void link_graphics_shader(
        graphics_shader_handle target, 
        const shader_part_vertex* spv, 
        const shader_part_geometry* spg, 
        const shader_part_pixel* spp
    );

    void bind_shader_sampler_to_pool(
        graphics_shader_handle target,
        const std::string& sampler_access,
        size_t pool_index
    );
    
    void bind_shader_uniform_to_pool(
        graphics_shader_handle target,
        const std::string& uniform_access,
        size_t pool_index
    );
}

/*
    Textures Methods
*/

namespace pikango
{
    size_t get_texture_pool_size();

    void bind_texture_to_pool(texture_1d_handle target, size_t pool_index);
    void bind_texture_to_pool(texture_2d_handle target, size_t pool_index);
    void bind_texture_to_pool(texture_3d_handle target, size_t pool_index);
    void bind_texture_to_pool(texture_cube_handle target, size_t pool_index);
    void bind_texture_to_pool(texture_1d_array_handle target, size_t pool_index);
    void bind_texture_to_pool(texture_2d_array_handle target, size_t pool_index);

    void write_texture(
        texture_1d_handle target, 
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        void* pixel_data
    );

    void write_texture(
        texture_2d_handle target, 
        texture_format source_format, 
        texture_format inner_format,
        size_t width, 
        size_t height, 
        void* pixel_data
    );

    void write_texture(
        texture_3d_handle target, 
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        size_t height, 
        size_t depth, 
        void* pixel_data
    );

    void write_texture(
        texture_cube_handle target, 
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        void* top, 
        void* bottom, 
        void* left, 
        void* right, 
        void* front,
        void* back
    );

    void write_texture(
        texture_1d_array_handle target,
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        std::vector<void*> pixel_data
    );
    void write_texture(
        texture_2d_array_handle target,
        texture_format source_format, 
        texture_format inner_format, 
        size_t width, 
        size_t height, 
        std::vector<void*> pixel_data
    );


    void set_texture_wraping(texture_1d_handle target, texture_wraping x);
    void set_texture_wraping(texture_2d_handle target, texture_wraping x, texture_wraping y);
    void set_texture_wraping(texture_3d_handle target, texture_wraping x, texture_wraping y, texture_wraping z);

    void set_texture_wraping(texture_cube_handle target, texture_wraping x, texture_wraping y, texture_wraping z);

    void set_texture_wraping(texture_1d_array_handle target, texture_wraping x);
    void set_texture_wraping(texture_2d_array_handle target, texture_wraping x, texture_wraping y);


    void set_texture_filtering(
        texture_1d_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying,
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_2d_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_3d_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_cube_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_1d_array_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );

    void set_texture_filtering(
        texture_2d_array_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );
}


/*
    Frame Buffer Methods
*/

namespace pikango
{
    size_t get_max_framebuffer_color_buffers_amount();

    void attach_framebuffer_color_buffer(
        frame_buffer_handle target,
        texture_2d_handle attachment,
        unsigned int slot
    );

    void attach_framebuffer_depth_buffer(
        frame_buffer_handle target,
        texture_2d_handle attachment
    );

    void attach_framebuffer_stencil_buffer(
        frame_buffer_handle target,
        texture_2d_handle attachment
    );
    

    void detach_framebuffer_color_buffer(
        frame_buffer_handle target,
        unsigned int slot
    );

    void detach_framebuffer_depth_buffer(frame_buffer_handle target);
    void detach_framebuffer_stencil_buffer(frame_buffer_handle target);


    texture_2d_handle get_framebuffer_color_buffer(
        frame_buffer_handle target,
        unsigned int slot
    );

    texture_2d_handle get_framebuffer_depth_buffer(frame_buffer_handle target);
    texture_2d_handle get_framebuffer_stencil_buffer(frame_buffer_handle target);
};


/*
    Drawing
*/

namespace pikango
{
    struct draw_target_args
    {
        frame_buffer_handle     frame_buffer;
    };

    struct draw_vertices_args
    {
        draw_primitive          primitive = draw_primitive::traingles;
        size_t                  first_vertex_index = 0;
        size_t                  vertices_count = 0;

        vertex_buffer_handle    vertex_buffer;
        data_layout_handle      vertex_layout;

        graphics_shader_handle  graphics_shader;
    };

    struct draw_indexed_args
    {
        draw_primitive          primitive = draw_primitive::traingles;
        size_t                  indicies_count = 0;

        vertex_buffer_handle    vertex_buffer;
        data_layout_handle      vertex_layout;
        
        index_buffer_handle     index_buffer;

        graphics_shader_handle  graphics_shader;
    };

    struct draw_instanced_args
    {
        size_t                  instances_count = 0;
        instance_buffer_handle  instance_buffer;
        data_layout_handle      instance_layout;
    };

    void draw_vertices(
        draw_target_args&   dta, 
        draw_vertices_args& dva
    );

    void draw_vertices_instanced(
        draw_target_args&   dta, 
        draw_vertices_args& dva,
        draw_instanced_args dia
    );

    void draw_indexed(
        draw_target_args&   dta, 
        draw_indexed_args&  dia
    );

    void draw_indexed_instanced(
        draw_target_args&   dta, 
        draw_indexed_args&  dia, 
        draw_instanced_args dia2
    );
}

/*
    Api Specific
*/

#ifdef PIKANGO_OPENGL_4_3
namespace pikango
{
    using opengl_thread_task = void(*)(std::vector<std::any>);
    void OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args);
    frame_buffer_handle OPENGL_ONLY_get_default_frame_buffer();
}
#endif

#endif
