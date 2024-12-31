PIKANGO_IMPL(command_buffer)
{
    pikango::queue_type target_queue_type;
    std::vector<recorded_task> tasks;
};

PIKANGO_NEW(command_buffer)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::command_buffer_impl);
    return handle;
}

void pikango::configure_command_buffer(command_buffer_handle target, queue_type target_queue_type)
{
    auto cbi = pikango_internal::object_write_access(target);
    cbi->target_queue_type = target_queue_type;
}

void pikango::begin_command_buffer_recording(command_buffer_handle target)
{
    recording_command_buffer = target;
}

void pikango::end_command_buffer_recording(command_buffer_handle target)
{
    recording_command_buffer = {};
}

void pikango::clear_command_buffer(command_buffer_handle target)
{
    auto cbi = pikango_internal::object_write_access(target);
    cbi->tasks.clear();
}
