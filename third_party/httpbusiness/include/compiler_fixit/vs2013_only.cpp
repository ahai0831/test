#include <rxcpp/rx.hpp>
#include <speed_counter.hpp>

/// Static singleton source file.
/// Just use it below VS2013, and just use it in binary project(*.exe, *.dll)
/// Just to fix some feature that VS2013 unsupported.

static struct fix_vs2013_static_feature_in_cpp11 {
  fix_vs2013_static_feature_in_cpp11() {
#ifdef RXCPP_RX_HPP
    /// schedulers\rx-currentthread.hpp
    const auto make_current_thread_fix =
        rxcpp::schedulers::make_current_thread();

    /// schedulers\rx-newthread.hpp
    const auto make_new_thread_fix = rxcpp::schedulers::make_new_thread();

//     /// schedulers\rx-eventloop.hpp
//     const auto make_event_loop_fix = rxcpp::schedulers::make_event_loop();

    /// schedulers\rx-immediate.hpp
    const auto make_immediate_fix = rxcpp::schedulers::make_immediate();

    /// rx-coordination.hpp
    const auto identity_immediate_fix = rxcpp::identity_immediate();

    const auto identity_current_fix = rxcpp::identity_current_thread();

//     const auto serialize_event_loop_fix = rxcpp::serialize_event_loop();

    const auto serialize_new_thread_fix = rxcpp::serialize_new_thread();

//     /// rx-observe_on.hpp
//     const auto observe_on_event_loop_fix = rxcpp::observe_on_event_loop();

    const auto observe_on_new_thread_fix = rxcpp::observe_on_new_thread();

//     /// subjects\rx-synchronize.hpp
//     const auto synchronize_event_loop_fix = rxcpp::synchronize_event_loop();

    const auto synchronize_new_thread_fix = rxcpp::synchronize_new_thread();

    /// rx-scheduler.hpp
	const auto shared_empty_fix = rxcpp::schedulers::detail::shared_empty();

#endif

#ifdef _SPEED_COUNTER_H
    const auto get_worker_fix = httpbusiness::default_worker::get_worker();
#endif
  }
} fix_vs2013;
