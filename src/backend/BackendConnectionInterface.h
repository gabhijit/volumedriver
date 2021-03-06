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

#ifndef BACKEND_CONNECTION_INTERFACE_H_
#define BACKEND_CONNECTION_INTERFACE_H_

#include "BackendException.h"
#include "BackendPolicyConfig.h"
#include "Condition.h"
#include "Namespace.h"

#include <iosfwd>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/filesystem.hpp>
#include <boost/intrusive/slist.hpp>
#include <boost/optional.hpp>

#include <youtils/Assert.h>
#include <youtils/BooleanEnum.h>
#include <youtils/CheckSum.h>
#include <youtils/FileUtils.h>
#include <youtils/Logging.h>
#include <youtils/FileDescriptor.h>
#include <youtils/UniqueObjectTag.h>

VD_BOOLEAN_ENUM(OverwriteObject);
VD_BOOLEAN_ENUM(ObjectMayNotExist);
VD_BOOLEAN_ENUM(InsistOnLatestVersion);
VD_BOOLEAN_ENUM(NamespaceMustNotExist);

namespace backend
{

class PartialReadCounter;

class BackendConnectionInterface
    : public boost::intrusive::slist_base_hook<>
{
protected:
    BackendConnectionInterface() = default;

 public:
    virtual ~BackendConnectionInterface() = default;

    virtual bool
    healthy() const = 0;

    void
    listNamespaces(std::list<std::string>& nspaces);

    void
    listObjects(const Namespace& nspace,
                std::list<std::string>& objects);

    bool
    namespaceExists(const Namespace& nspace);

    void
    createNamespace(const Namespace&,
                    const NamespaceMustNotExist = NamespaceMustNotExist::T);

    void
    deleteNamespace(const Namespace& nspace);

    void
    clearNamespace(const Namespace& nspace);

    void
    invalidate_cache(const boost::optional<Namespace>& nspace);

    void
    read(const Namespace& nspace,
         const boost::filesystem::path& destination,
         const std::string& name,
         const InsistOnLatestVersion insist_on_latest);

    std::unique_ptr<youtils::UniqueObjectTag>
    get_tag(const Namespace&,
            const std::string&);

    struct ObjectSlice
    {
        uint32_t size;
        uint64_t offset;
        uint8_t* buf;

        ObjectSlice(const uint32_t s,
                    const uint64_t o,
                    uint8_t* b)
            : size(s)
            , offset(o)
            , buf(b)
        {}

        ObjectSlice(const ObjectSlice& other)
            : size(other.size)
            , offset(other.offset)
            , buf(other.buf)
        {}

        bool
        operator<(const ObjectSlice& other) const
        {
            return offset < other.offset;
        }

        bool
        operator==(const ObjectSlice& other) const
        {
            return
                size == other.size and
                offset == other.offset and
                buf == other.buf;
        }

        bool
        operator!=(const ObjectSlice& other) const
        {
            return not operator==(other);
        }
    };

    // We want slices to be ordered by offset ...
    using ObjectSlices = std::set<ObjectSlice>;
    // ... and to be grouped by object.
    using PartialReads = std::map<std::string, ObjectSlices>;

    // Make this a std::function? On the upside this would allow lambdas, but then
    // again this also means an allocation.
    struct PartialReadFallbackFun
    {
        virtual ~PartialReadFallbackFun() = default;

        virtual youtils::FileDescriptor&
        operator()(const Namespace& nspace,
                   const std::string& object_name,
                   InsistOnLatestVersion) = 0;
    };

    PartialReadCounter
    partial_read(const Namespace& ns,
                 const PartialReads& partial_reads,
                 InsistOnLatestVersion,
                 PartialReadFallbackFun& fallback);

    void
    write(const Namespace&,
          const boost::filesystem::path&,
          const std::string&,
          const OverwriteObject = OverwriteObject::F,
          const youtils::CheckSum* = nullptr,
          const boost::shared_ptr<Condition>& = nullptr);

    std::unique_ptr<youtils::UniqueObjectTag>
    write_tag(const Namespace&,
              const boost::filesystem::path&,
              const std::string&,
              const youtils::UniqueObjectTag*,
              const OverwriteObject = OverwriteObject::T);

    std::unique_ptr<youtils::UniqueObjectTag>
    read_tag(const Namespace&,
             const boost::filesystem::path&,
             const std::string&);

    bool
    objectExists(const Namespace& nspace,
                 const std::string& name);

    void
    remove(const Namespace&,
           const std::string&,
           const ObjectMayNotExist = ObjectMayNotExist::F,
           const boost::shared_ptr<Condition>& = nullptr);

    uint64_t
    getSize(const Namespace& nspace,
            const std::string& name);

    youtils::CheckSum
    getCheckSum(const Namespace& nspace,
                const std::string& name);

public:
    virtual void
    listNamespaces_(std::list<std::string>& nspaces) = 0;

    virtual void
    listObjects_(const Namespace& nspace,
                 std::list<std::string>& objects) = 0;

    virtual bool
    namespaceExists_(const Namespace&) = 0;

    virtual void
    createNamespace_(const Namespace&,
                     const NamespaceMustNotExist = NamespaceMustNotExist::T) = 0;

    virtual void
    deleteNamespace_(const Namespace& nspace) = 0;

    virtual void
    clearNamespace_(const Namespace& nspace);

    virtual void
    invalidate_cache_(const boost::optional<Namespace>& nspace);

    virtual void
    read_(const Namespace&,
          const boost::filesystem::path& Destination,
          const std::string& name,
          const InsistOnLatestVersion insist_on_latest) = 0;

    virtual std::unique_ptr<youtils::UniqueObjectTag>
    get_tag_(const Namespace&,
             const std::string&) = 0;

    virtual bool
    partial_read_(const Namespace& ns,
                  const PartialReads& reads,
                  InsistOnLatestVersion,
                  PartialReadCounter&) = 0;

    virtual void
    write_(const Namespace&,
           const boost::filesystem::path&,
           const std::string&,
           const OverwriteObject = OverwriteObject::F,
           const youtils::CheckSum* = nullptr,
           const boost::shared_ptr<Condition>& = nullptr) = 0;

    virtual std::unique_ptr<youtils::UniqueObjectTag>
    write_tag_(const Namespace&,
               const boost::filesystem::path&,
               const std::string&,
               const youtils::UniqueObjectTag* old_tag,
               const OverwriteObject = OverwriteObject::T) = 0;

    virtual std::unique_ptr<youtils::UniqueObjectTag>
    read_tag_(const Namespace&,
              const boost::filesystem::path&,
              const std::string&) = 0;

    virtual bool
    objectExists_(const Namespace&,
                  const std::string& name) = 0;

    virtual void
    remove_(const Namespace&,
            const std::string&,
            const ObjectMayNotExist,
            const boost::shared_ptr<Condition>& = nullptr) = 0;

    virtual uint64_t
    getSize_(const Namespace&,
             const std::string& name) = 0;

    virtual youtils::CheckSum
    getCheckSum_(const Namespace&,
                 const std::string& name) = 0;

private:
    DECLARE_LOGGER("BackendConnectionInterface");
};

std::ostream&
operator<<(std::ostream&,
           const BackendConnectionInterface::ObjectSlice&);

std::ostream&
operator<<(std::ostream&,
           const BackendConnectionInterface::ObjectSlices&);

std::ostream&
operator<<(std::ostream&,
           const BackendConnectionInterface::PartialReads&);

}

#endif //!BACKEND_CONNECTION_INTERFACE_H_

// Local Variables: **
// mode: c++ **
// End: **
