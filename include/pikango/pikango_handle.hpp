#pragma once
#include <atomic>

namespace pikango_internal
{
    template<class T>
    void implementations_destructor(T*);

    template<class handled_object>
    class handle
    {
    private:
        struct meta_block
        {
            bool valid = true;
            std::atomic<uint64_t> refs;
        };

        handled_object* object;
        meta_block* meta;

    public:
        handle() : meta(nullptr), object(nullptr) {};
        handle(const handle& other) : 
            meta(other.meta), 
            object(other.object)
        {
            if (meta != nullptr)
                meta->refs++;
        }
        ~handle()
        {
            if (meta != nullptr)
            {
                meta->refs--;
                if (meta->refs == 0)
                {
                    implementations_destructor(object);
                    delete meta;
                }
            }
        }
    };
}
