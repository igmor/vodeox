#ifndef __TYPES_H
#define __TYPES_H

#if defined(__linux__) || defined(__APPLE__)
    /// A 64-bit signed integer.
    typedef long long int64;

    /// A 64-bit unsigned integer.
    typedef unsigned long long uint64;

#else
#error "Unknown architecture"
#endif

#endif
