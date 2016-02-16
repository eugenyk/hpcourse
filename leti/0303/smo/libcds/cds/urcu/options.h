//$$CDS-header$$

#ifndef _CDS_URCU_OPTIONS_H
#define _CDS_URCU_OPTIONS_H

#include <cds/details/defs.h>

namespace cds { namespace opt {

    /// [type-option] RCU check deadlock option setter
    /**
        The RCU containers can checks if a deadlock between read-side critical section
        and \p synchronize call is possible.
        This option specifies a policy for checking this situation.
        Possible \p Type is:
        - opt::v::rcu_no_check_deadlock - no deadlock checking
        - opt::v::rcu_assert_deadlock - call \p assert in debug mode only
        - opt::v::rcu_throw_deadlock - throw an \p cds::urcu::rcu_deadlock exception when a deadlock
            is encountered

        Usually, the default \p Type for this option is opt::v::rcu_throw_deadlock.
    */
    template <typename Type>
    struct rcu_check_deadlock
    {
        //@cond
        template <typename Base> struct pack: public Base
        {
            typedef Type rcu_check_deadlock     ;
        };
        //@endcond
    };

    namespace v {
        /// \ref opt::rcu_check_deadlock option value: no deadlock checking
        struct rcu_no_check_deadlock {};

        /// \ref opt::rcu_check_deadlock option value: call \p assert in debug mode only
        struct rcu_assert_deadlock {};

        /// \ref opt::rcu_check_deadlock option value: throw a cds::urcu::rcu_deadlock exception when a deadlock detected
        struct rcu_throw_deadlock {};
    }
}}  // namespace cds::opt


#endif  // #ifndef _CDS_URCU_OPTIONS_H
