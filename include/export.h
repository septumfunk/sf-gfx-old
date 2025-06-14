#ifndef EXPORT_H
#define EXPORT_H

#ifdef _WIN32
#    ifdef SF_DYNAMIC
#        ifdef SF_EXPORTS
#            define EXPORT __declspec(dllexport)
#        else
#            define EXPORT __declspec(dllimport)
#        endif
#    else
#        define EXPORT
#    endif
#elif
#    define EXPORT
#endif

#endif
