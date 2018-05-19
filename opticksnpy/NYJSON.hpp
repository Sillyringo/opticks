#pragma once


// https://useyourloaf.com/blog/disabling-clang-compiler-warnings/

#ifdef __clang__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-braces"

#elif defined(__GNUC__) || defined(__GNUG__)
#elif defined(_MSC_VER)
#endif


//#include "YoctoGL/ext/json.hpp"
#include "ext/json.hpp"



#ifdef __clang__

#pragma clang diagnostic pop

#elif defined(__GNUC__) || defined(__GNUG__)
#elif defined(_MSC_VER)
#endif





