#pragma once

#if defined(_MSC_VER)
#if defined(WIN32) || defined(_WIN32)
        typedef char			WN_INT8;
        typedef unsigned char		WN_UINT8;
        typedef short			WN_INT16;
        typedef unsigned short		WN_UINT16;
        typedef int			WN_INT32;
        typedef unsigned int		WN_UINT32;
#ifndef _WIN32_WCE
        typedef long long		WN_INT64;
        typedef unsigned long long	WN_UINT64;
#endif
#else if defined(_WIN64)
        typedef char			WN_INT8;
        typedef unsigned char		WN_UINT8;
        typedef short			WN_INT16;
        typedef unsigned short		WN_UINT16;
        typedef int			WN_INT32;
        typedef unsigned int		WN_UINT32;
        typedef long 			WN_INT64;
        typedef unsigned long		WN_UINT64;
#endif
#endif	//_MSC_VER