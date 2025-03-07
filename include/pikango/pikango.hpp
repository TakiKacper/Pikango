#pragma once
#ifndef PIKANGO_HPP
#define PIKANGO_HPP

#include <vector>
#include <array>
#include <unordered_map>
#include <set>

#include <string>

#include <any>
#include <variant>

/*
    HANDLES
*/

#include "pikango_handle.hpp"

namespace pikango
{
    template <class handled_object>
    bool is_empty(const pikango_internal::handle<handled_object>& handle)
    {
        return pikango_internal::is_empty(handle);
    }
    
    template <class handled_object>
    size_t handle_hash(const pikango_internal::handle<handled_object>& handle)
    {
        return pikango_internal::handle_hash(handle);
    }
}

/*
This macro for name = xyz would create folowing objects:

    pikango_internal::xyz_impl  (forward)
    
    pikango::xyz_handle         (handle specialization)
    pikango::new_xyz            (function)
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
    }

PIKANGO_HANDLE_FWD(graphics_pipeline);
//PIKANGO_HANDLE_FWD(compute_pipeline)
PIKANGO_HANDLE_FWD(command_buffer);
PIKANGO_HANDLE_FWD(fence);
PIKANGO_HANDLE_FWD(shader);
PIKANGO_HANDLE_FWD(frame_buffer);
PIKANGO_HANDLE_FWD(buffer);
PIKANGO_HANDLE_FWD(texture_sampler);
PIKANGO_HANDLE_FWD(texture_buffer);

#undef PIKANGO_HANDLE_FWD

/*
    ENUMERATIONS
*/

namespace pikango
{
    enum class draw_primitive : unsigned char
    {
        points, 
        lines, lines_loop, line_strip,
        traingles, traingles_strip
    };

    enum class buffer_memory_profile : unsigned char
    {
        rare_write_rare_read,   //data is rarely in use
        rare_write_often_read,  //data is often overwritten and often used
        often_write_often_read  //data is rarely overwritten and often used
    };

    enum class buffer_access_profile : unsigned char
    {
        cpu_to_gpu, //application writes data to gpu
        gpu_to_cpu, //gpu data is read by application 
        gpu_to_gpu  //gpu data is read by gpu
    };

    enum class data_type : unsigned char
    {
        int32,
        vec2i32,
        vec3i32,
        vec4i32,

        float32,
        vec2f32,
        vec3f32,
        vec4f32
    };
    size_t size_of(data_type dt);

    enum class shader_type : unsigned char
    {
        vertex,
        pixel,
        geometry
    };

    enum class texture_type : unsigned char
    {
        texture_1d, 
        texture_2d, 
        texture_3d,
        texture_cubemap,
        texture_1d_array,
        texture_2d_array
    };

    enum class texture_filtering : unsigned char
	{
		nearest, linear
	};

    enum class texture_wraping : unsigned char
	{
		repeat, 
        mirror_repeat,
		clamp_coords, 
        clamp_texture
	};

    enum class texture_source_format : unsigned char
    {
        r,
        rg,
        rgb,
        rgba
    };

    enum class texture_sized_format : unsigned char
	{
		r8, r16,
		rg8, rg16,
		r3_g3_b2, rgb4, rgb5, rgb8, rgb10, rgb12, 
		rgba2, rgba4, rgba8, rgba12, rgba16, rgba32f
	};

    enum class rasterization_culling_mode : unsigned char
    {
        none,
        front,
        back,
        front_and_back,
    };

    enum class rasterization_culling_front_face : unsigned char
    {
        couter_clockwise,
        clockwise
    };

    enum class rasterization_polygon_fill_mode : unsigned char
    {
        fill_shape, 
        fill_edges, 
        fill_vertices
    };

    enum class depth_compare_operator : unsigned char
    {
        nerer,
        less,
        equal,
        less_or_equal,
        greater,
        not_equal,
        greater_or_equal,
        always
    };
}

/*
    STRUCTS
*/

namespace pikango
{
    //Struct for representing rectangles
    //it describes them at two points in a coordinate system: a and b
    //a being the lower-left point  (offset)
    //b being the upper-right point (extend)
    //  |----------B
    //  |----------|
    //  A----------|
    struct rectangle
    {
        int ax;
        int ay;

        int bx;
        int by;

        rectangle() : ax(0), ay(0), bx(400), by(400) {};
        rectangle(int _ax, int _ay, int _bx, int _by)
            : ax(_ax), ay(_ay), bx(_bx), by(_by) {};
    };

    struct vertex_attribute_config
    {
        size_t      binding;
        size_t      location;

        data_type   type;
        size_t      stride;
        size_t      offset;

        bool        per_instance;
    };

    struct vertex_layout_pipeline_config
    {
        std::vector<vertex_attribute_config> attributes;
    };

    struct graphics_shaders_pipeline_config
    {
        shader_handle vertex_shader;
        shader_handle pixel_shader;
        shader_handle geometry_shader;
    };

    struct rasterization_pipeline_config
    {
        bool enable_culling = false;

        rasterization_polygon_fill_mode   polygon_fill = rasterization_polygon_fill_mode::fill_shape;
        rasterization_culling_mode        culling_mode = rasterization_culling_mode::none;
        rasterization_culling_front_face  culling_front_face = rasterization_culling_front_face::couter_clockwise;

        float line_width = 1.0f;
    };

    struct depth_stencil_pipeline_config
    {
        bool enable_depth_test = false;
        bool enable_depth_write = false;
    };

    struct graphics_pipeline_config
    {
        vertex_layout_pipeline_config     vertex_layout_config;
        graphics_shaders_pipeline_config  shaders_config;
        rasterization_pipeline_config     rasterization_config;
        depth_stencil_pipeline_config     depth_stencil_config;
    };

    struct sampled_texture
    {
        texture_sampler_handle sampler;
        texture_buffer_handle  buffer;
    };
}

/*
    FUNCTIONS
*/

//Library
namespace pikango
{
    using error_notification_callback = void(*)(const char* notification);

    struct initialize_library_cpu_settings
    {
        error_notification_callback error_callback = nullptr;
    };

    std::string initialize_library_cpu(const initialize_library_cpu_settings& settings);
    std::string initialize_library_gpu();
    std::string terminate();
}

#ifdef PIKANGO_OPENGL_4_3
namespace pikango
{
    using opengl_thread_task = void(*)(std::vector<std::any>);
    void OPENGL_ONLY_execute_on_context_thread(opengl_thread_task task, std::vector<std::any> args);
}
#endif

//Queues
namespace pikango
{
    enum class queue_type
    {
        general, compute, transfer
    };

    size_t get_queues_max_amount(queue_type type);
    void enable_queues(queue_type type, size_t amount);

    void wait_queue_empty(queue_type type, size_t queue_index);
    void wait_all_queues_empty();
    void submit_command_buffer(command_buffer_handle target, queue_type type, size_t queue_index);
    void submit_command_buffer_with_fence(command_buffer_handle target, queue_type type, size_t queue_index, fence_handle wait_fence);
}

//Getters
namespace pikango
{
    //Shaders
    const char* get_used_shading_language_name();

    //Framebuffers
    size_t get_max_framebuffer_color_buffers_attachments();
}

/*
    COMMNANDS AND UTILITY METHODS
*/

//Pipelines
namespace pikango
{
    void configure_graphics_pipeline(graphics_pipeline_handle target, graphics_pipeline_config& config);
}

//Command Buffer
namespace pikango
{
    void configure_command_buffer(command_buffer_handle target, queue_type target_queue_type);
    void begin_command_buffer_recording(command_buffer_handle target);
    void end_command_buffer_recording(command_buffer_handle target);
    void clear_command_buffer(command_buffer_handle target);
}

//Fences
namespace pikango
{
    void wait_fence(fence_handle target);
    void wait_multiple_fences(std::vector<fence_handle> targets);
}

//Buffer
namespace pikango
{
    size_t get_buffer_size(buffer_handle target);
}

namespace pikango::cmd
{
    void assign_buffer_memory(
        buffer_handle target, 
        size_t memory_block_size_bytes, 
        buffer_memory_profile memory_profile, 
        buffer_access_profile access_profile
    );

    void write_buffer(
        buffer_handle target, 
        size_t data_size_bytes, 
        void* data
    );

    void write_buffer_region(
        buffer_handle target, 
        size_t data_size_bytes, 
        void* data, 
        size_t data_offset_bytes
    );

    void copy_buffer_to_buffer(
        buffer_handle source, 
        buffer_handle destination,
        size_t read_offset, 
        size_t read_size, 
        size_t write_offset
    );
}

//Shaders Compilation
namespace pikango
{
    void compile_shader(shader_handle target, shader_type type, const std::string& source);
}

//Texture Samplers
namespace pikango
{
    void set_sampler_wraping(
        texture_sampler_handle target, 
        texture_wraping x, 
        texture_wraping y, 
        texture_wraping z
    );

    void set_sampler_filtering(
        texture_sampler_handle target, 
        texture_filtering magnifying, 
        texture_filtering minifying, 
        texture_filtering mipmap
    );
}

//Texture Storages
namespace pikango::cmd
{
    void assign_texture_buffer_memory(
        texture_buffer_handle   target,
        texture_type            type,
        size_t                  mipmap_layers,
        texture_sized_format    memory_format,
        size_t                  dim_1,
        size_t                  dim_2,
        size_t                  dim_3
    );

    void write_texture_buffer(
        texture_buffer_handle   target,
        size_t                  mipmap_layer,
        texture_source_format   source_format,
        void*                   data,
        size_t                  off_1,
        size_t                  off_2,
        size_t                  off_3,
        size_t                  dim_1,
        size_t                  dim_2,
        size_t                  dim_3
    );
}

//Framebuffers
namespace pikango
{
#ifdef PIKANGO_OPENGL_4_3
    frame_buffer_handle OPENGL_ONLY_get_default_frame_buffer();
#endif
};

/*
    DRAWING AND STATE
*/

//Binding
namespace pikango::cmd
{
    void bind_graphics_pipeline(graphics_pipeline_handle pipeline);
    
    void bind_frame_buffer(frame_buffer_handle frame_buffer);

    void bind_vertex_buffer(buffer_handle vertex_buffer, size_t binding);
    void bind_index_buffer(buffer_handle index_buffer);

    void bind_texture(
        texture_sampler_handle sampler,
        texture_buffer_handle buffer,
        size_t slot        
    );

    void bind_uniform_buffer(
        buffer_handle uniform_buffer,
        size_t slot,
        size_t size,
        size_t offset
    );
}

//Drawing Related Commands
namespace pikango::cmd
{
    void set_viewport(const rectangle& rect);
    void set_scissors(const rectangle& rect);
}

//Drawing
namespace pikango::cmd
{
    void draw_vertices(
        draw_primitive  primitive,

        size_t          vertices_count,
        size_t          vertices_buffer_offset_index,

        size_t          instances_count,
        size_t          instances_id_values_offset
    );

    void draw_indexed(
        draw_primitive  primitive,

        size_t          indices_count,
        size_t          indicies_buffer_offset_index,
        int32_t         indicies_values_offset,

        size_t          instances_count,
        size_t          instances_id_values_offset
    );
}

#endif
