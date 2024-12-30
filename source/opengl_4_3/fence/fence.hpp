PIKANGO_IMPL(fence)
{
    bool subbmitted = false;
    bool is_signaled = false;
    std::mutex mutex;
    std::condition_variable condition;
};

PIKANGO_NEW(fence)
{
    auto handle = pikango_internal::make_handle(new pikango_internal::fence_impl);
    return handle;
}

PIKANGO_DELETE(fence)
{ 

};
