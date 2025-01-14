#include "pikango/pikango.hpp"
#include "before_impl.hpp"

#ifdef PIKANGO_OPENGL_4_3
    #include "opengl_4_3/pikango_impl.hpp"
#else
    #error No Pikango implementation specified.
#endif

#include "after_impl.hpp"
