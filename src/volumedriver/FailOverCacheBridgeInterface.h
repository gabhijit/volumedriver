// Copyright (C) 2016 iNuron NV
//
// This file is part of Open vStorage Open Source Edition (OSE),
// as available from
//
//      http://www.openvstorage.org and
//      http://www.openvstorage.com.
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License v3 (GNU AGPLv3)
// as published by the Free Software Foundation, in version 3 as it comes in
// the LICENSE.txt file of the Open vStorage OSE distribution.
// Open vStorage is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY of any kind.

#ifndef VD_FAILOVER_CACHE_BRIDGE_INTERFACE_H
#define VD_FAILOVER_CACHE_BRIDGE_INTERFACE_H

#include "FailOverCacheBridgeCommon.h"
#include "FailOverCacheMode.h"
#include "failovercache/fungilib/Mutex.h"

#include "SCOProcessorInterface.h"

#include <boost/thread/future.hpp>

#include <youtils/Logging.h>

namespace volumedriver
{

namespace failovercache
{
class ClientInterface;
}

class FailOverCacheBridgeInterface
{
public:
    static std::unique_ptr<FailOverCacheBridgeInterface>
    create(const FailOverCacheMode mode,
           const LBASize lba_size,
           const ClusterMultiplier cluster_multiplier,
           const size_t max_entries,
           const std::atomic<unsigned>& write_trigger);

    virtual ~FailOverCacheBridgeInterface() = default;

    using DegradedFun = std::function<void() noexcept>;

    virtual void
    initialize(DegradedFun) = 0;

    virtual void
    destroy(SyncFailOverToBackend) = 0;

    virtual boost::future<void>
    addEntries(const std::vector<ClusterLocation>&,
               uint64_t start_address,
               const uint8_t* data) = 0;

    virtual bool
    backup() = 0;

    virtual void
    newCache(std::unique_ptr<failovercache::ClientInterface>) = 0;

    virtual boost::chrono::milliseconds
    getDefaultRequestTimeout() const
    {
        return boost::chrono::milliseconds(60000);
    }

    virtual void
    setRequestTimeout(const boost::chrono::seconds) = 0;

    virtual void
    setBusyLoopDuration(const boost::chrono::microseconds) = 0;

    virtual void
    removeUpTo(const SCO& sconame) = 0;

    virtual boost::future<void>
    Flush() = 0;

    virtual void
    Clear() = 0;

    virtual uint64_t
    getSCOFromFailOver(SCO sconame,
                       SCOProcessorFun processor) = 0;

    virtual FailOverCacheMode
    mode() const = 0;

    size_t
    max_entries() const
    {
        return max_entries_;
    }

protected:
    explicit FailOverCacheBridgeInterface(const size_t max_entries)
        : max_entries_(max_entries)
    {}

private:
    DECLARE_LOGGER("FailOverCacheBridgeInterface");

    const size_t max_entries_ ;
};

} // namespace volumedriver

#endif // VD_FAILOVER_CACHE_BRIDGE_INTERFACE_H