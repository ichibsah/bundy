// Copyright (C) 2012  Internet Systems Consortium, Inc. ("ISC")
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
// REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
// LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include <exceptions/exceptions.h>
#include <auth/datasrc_config.h>
#include <cc/data.h>

#include "test_datasrc_clients_mgr.h"

#include <cassert>

namespace bundy {
namespace auth {
namespace datasrc_clientmgr_internal {

// Define static DataSrcClientsBuilder member variables.
bool FakeDataSrcClientsBuilder::started = false;
std::list<Command>* FakeDataSrcClientsBuilder::command_queue = NULL;
std::list<FinishedCallbackPair>* FakeDataSrcClientsBuilder::callback_queue =
    NULL;
std::list<Command> FakeDataSrcClientsBuilder::command_queue_copy;
std::list<FinishedCallbackPair> FakeDataSrcClientsBuilder::callback_queue_copy;
TestCondVar* FakeDataSrcClientsBuilder::cond = NULL;
TestCondVar FakeDataSrcClientsBuilder::cond_copy;
TestMutex* FakeDataSrcClientsBuilder::queue_mutex = NULL;
bundy::datasrc::ClientListMapPtr*
    FakeDataSrcClientsBuilder::clients_map = NULL;
TestMutex* FakeDataSrcClientsBuilder::map_mutex = NULL;
TestMutex FakeDataSrcClientsBuilder::queue_mutex_copy;
bool FakeDataSrcClientsBuilder::thread_waited = false;
FakeDataSrcClientsBuilder::ExceptionFromWait
FakeDataSrcClientsBuilder::thread_throw_on_wait =
    FakeDataSrcClientsBuilder::NOTHROW;
int FakeDataSrcClientsBuilder::wakeup_fd = -1;

template<>
data::ConstElementPtr
TestDataSrcClientsBuilder::doNoop() {
    ++queue_mutex_->noop_count;
    switch (queue_mutex_->throw_from_noop) {
    case TestMutex::NONE:
        break;                  // no throw
    case TestMutex::EXCLASS:
        bundy_throw(Exception, "test exception");
    case TestMutex::INTEGER:
        throw 42;
    case TestMutex::INTERNAL:
        bundy_throw(InternalCommandError, "internal error, should be ignored");
    }

    // It returns a fixed constant so the test can examine it.
    return (data::Element::create(true));
}
} // namespace datasrc_clientmgr_internal

template<>
void
TestDataSrcClientsMgrBase::cleanup() {
    using namespace datasrc_clientmgr_internal;
    // Make copy of some of the manager's member variables and reset the
    // corresponding pointers.  The currently pointed objects are in the
    // manager object, which are going to be invalidated.

    FakeDataSrcClientsBuilder::command_queue_copy = command_queue_;
    FakeDataSrcClientsBuilder::command_queue =
        &FakeDataSrcClientsBuilder::command_queue_copy;
    FakeDataSrcClientsBuilder::queue_mutex_copy = queue_mutex_;
    FakeDataSrcClientsBuilder::queue_mutex =
        &FakeDataSrcClientsBuilder::queue_mutex_copy;
    FakeDataSrcClientsBuilder::cond_copy = cond_;
    FakeDataSrcClientsBuilder::cond =
        &FakeDataSrcClientsBuilder::cond_copy;
    FakeDataSrcClientsBuilder::callback_queue_copy =
        *FakeDataSrcClientsBuilder::callback_queue;
    FakeDataSrcClientsBuilder::callback_queue =
        &FakeDataSrcClientsBuilder::callback_queue_copy;
}

template<>
void
TestDataSrcClientsMgrBase::reconfigureHook() {
    using namespace datasrc_clientmgr_internal;

    // Simply replace the local map, ignoring bogus config value.
    assert(command_queue_.front().id == RECONFIGURE);
    try {
        clients_map_ = configureDataSource(command_queue_.front().params);
    } catch (...) {}
}

} // namespace auth
} // namespace bundy

// Local Variables:
// mode: c++
// End:
