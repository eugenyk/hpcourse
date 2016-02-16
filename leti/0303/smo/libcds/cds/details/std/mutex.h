//$$CDS-header$$

#ifndef __CDS_DETAILS_STD_MUTEX_H
#define __CDS_DETAILS_STD_MUTEX_H

//@cond

#include <cds/details/defs.h>

#ifdef CDS_CXX11_STDLIB_MUTEX
#   include <mutex>
    namespace cds_std {
        using std::mutex ;
        using std::recursive_mutex ;
        using std::unique_lock ;
    }
#else
#   include <boost/thread/mutex.hpp>
#   include <boost/thread/recursive_mutex.hpp>
    namespace cds_std {
        using boost::mutex ;
        using boost::recursive_mutex ;
        using boost::unique_lock ;
    }
#endif

//@endcond

#endif // #ifndef __CDS_DETAILS_STD_MUTEX_H
