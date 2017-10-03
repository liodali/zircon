// Copyright 2016 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#pragma once

#include <stdint.h>

#include <kernel/spinlock.h>
#include <zircon/types.h>
#include <fbl/canary.h>
#include <fbl/intrusive_double_list.h>
#include <fbl/mutex.h>
#include <object/state_observer.h>

class Handle;

class CookieJar {
public:
    CookieJar() : scope_(ZX_KOID_INVALID), cookie_(0) {}
    zx_koid_t scope_;
    uint64_t cookie_;
};

class StateTracker {
public:
    StateTracker(zx_signals_t signals = 0u) : signals_(signals) { }

    StateTracker(const StateTracker& o) = delete;
    StateTracker& operator=(const StateTracker& o) = delete;

    // Add an observer.
    void AddObserver(StateObserver* observer, const StateObserver::CountInfo* cinfo);

    // Remove an observer (which must have been added).
    void RemoveObserver(StateObserver* observer);

    // Called when observers of the handle's state (e.g., waits on the handle) should be
    // "cancelled", i.e., when a handle (for the object that owns this StateTracker) is being
    // destroyed or transferred. Returns true if at least one observer was found.
    bool Cancel(Handle* handle);

    // Like Cancel() but issued via via zx_port_cancel().
    bool CancelByKey(Handle* handle, const void* port, uint64_t key);

    // Notify others of a change in state (possibly waking them). (Clearing satisfied signals or
    // setting satisfiable signals should not wake anyone.)
    void UpdateState(zx_signals_t clear_mask, zx_signals_t set_mask);

    zx_signals_t GetSignalsState() { return signals_; }

    using ObserverList = fbl::DoublyLinkedList<StateObserver*, StateObserverListTraits>;

    // Accessors for CookieJars
    // These live with the state tracker so they can make use of the state tracker's
    // lock (since not all objects have their own locks, but all Dispatchers that are
    // cookie-capable have state trackers)
    zx_status_t SetCookie(CookieJar* cookiejar, zx_koid_t scope, uint64_t cookie);
    zx_status_t GetCookie(CookieJar* cookiejar, zx_koid_t scope, uint64_t* cookie);
    zx_status_t InvalidateCookie(CookieJar *cookiejar);

private:
    // Returns flag kHandled if one of the observers have been signaled.
    StateObserver::Flags UpdateInternalLocked(ObserverList* obs_to_remove, zx_signals_t signals) TA_REQ(lock_);

    fbl::Canary<fbl::magic("STRK")> canary_;

    zx_signals_t signals_;
    fbl::Mutex lock_;

    // Active observers are elements in |observers_|.
    ObserverList observers_ TA_GUARDED(lock_);
};
