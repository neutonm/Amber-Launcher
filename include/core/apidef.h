#ifndef __SAPIDEF_H_
#define __SAPIDEF_H_

/*
 * ----------------------------------------------------------------------------
 * Define export and import symbols
 * ----------------------------------------------------------------------------
 */

#ifdef _WIN32
    #ifdef BUILD_SHARED_LIBS
        #ifdef BUILD_DLL
            #define CAPI __declspec(dllexport)
        #else
            #define CAPI __declspec(dllimport)
        #endif
    #else
        #define CAPI
    #endif
#else
    #if __GNUC__ >= 4
        #define CAPI __attribute__((visibility("default")))
    #else
        #define CAPI
    #endif
#endif

#endif
