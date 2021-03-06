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

#ifndef DATASRC_ZONE_DATA_LOADER_H
#define DATASRC_ZONE_DATA_LOADER_H 1

#include <datasrc/exceptions.h>
#include <datasrc/memory/zone_data.h>
#include <datasrc/zone_iterator.h>
#include <dns/dns_fwd.h>
#include <util/memory_segment.h>

#include <utility>

namespace bundy {
namespace datasrc {
class DataSourceClient;

namespace memory {

/// \brief Zone is invalid exception.
///
/// This is thrown if an invalid zone would be created during
/// \c loadZoneData().
struct ZoneValidationError : public ZoneLoaderException {
    ZoneValidationError(const char* file, size_t line, const char* what) :
        ZoneLoaderException(file, line, what)
    {}
};

/// \brief Utility class for loading/updating a zone from a given source.
///
/// Some of the methods are defined as virtual so that tests can mock the
/// behavior of class.  For other purposes this class shouldn't be derived.
class ZoneDataLoader {
protected:
    ZoneDataLoader() : impl_(NULL) {}

public:
    class ZoneDataLoaderImpl;

    /// \brief Constructor for loading from a file.
    ///
    /// \param mem_sgmt The memory segment.
    /// \param rrclass The RRClass.
    /// \param zone_name The name of the zone that is being loaded.
    /// \param zone_file Filename which contains the zone data for \c zone_name.
    /// \param old_data If non-NULL, zone data currently being used.  Also
    /// in that case, its origin name must be equal to \c zone_name.
    ZoneDataLoader(util::MemorySegment& mem_sgmt,
                   const dns::RRClass& rrclass,
                   const dns::Name& zone_name,
                   const std::string& zone_file,
                   ZoneData* old_data = NULL);

    /// \brief Constructor for loading from a given data source.
    ///
    /// Most of the parameters are the same as the other version.
    /// The caller is responsible for keeping \c datasrc_client valid while
    /// the constructed \c ZoneDataLoader is used.  This should be the case
    /// in the main usage, but test code could easily break the assumption.
    ///
    /// \param datasrc_client A client for the data source from which new
    /// zone data should be loaded.
    ZoneDataLoader(util::MemorySegment& mem_sgmt,
                   const dns::RRClass& rrclass,
                   const dns::Name& zone_name,
                   const DataSourceClient& datasrc_client,
                   ZoneData* old_data = NULL);

    /// Destructor.
    virtual ~ZoneDataLoader();

    typedef std::pair<ZoneData*, bool> LoadResult;

    /// \brief Return whether the passed ZoneData on construction will be reused
    ///
    /// This is determined at the time of construction, so once constructed
    /// this method always returns the same value.
    ///
    /// \throw None
    ///
    /// \return true if the zone data passed to constructor is used for load;
    /// otherwise false.
    virtual bool isDataReused() const;

    /// \brief Create and return a ZoneData instance populated from the
    /// source passed on construction.
    ///
    /// This is actually a shortcut of call to \c loadIncremental(0) and
    /// returning the result of \c getLoadedData().
    ///
    /// \return A \c ZoneData containing zone data loaded from the source.
    virtual ZoneData* load();

    /// \brief Incrementally load zone data into memory from the source passed
    /// on construction.
    ///
    /// It loads the specified up to the number of "items" and returns whether
    /// the load is completed or not.  The meaning of "items" vary depending on
    /// how the loader was constructed: If it's constructed with a master zone
    /// file, it's roughly equals to the number of RRs in the file; if it's
    /// constructed with a data source client, it's the number of RRsets
    /// provided via underlying zone iterator or journal reader (the constructor
    /// chooses an appropriate one).
    ///
    /// If the specified limit is 0, this method continues the load until
    /// it's completed.
    ///
    /// \throw ZoneDataUpdater::AddError Invalid or inconsistent data found.
    /// \throw EmptyZone If an empty zone would be created
    /// \throw bundy::Unexpected An empty RRset is given from the source
    ///        (shouldn't happen, but possible for a buggy data source
    ///         implementation).
    ///
    /// \param count_limit the number of items to be loaded in this call.
    /// If 0, it means unlimited.
    /// \return true if the load is completed with this call; false otherwise.
    virtual bool loadIncremental(size_t count_limit);

    /// \brief Returns the ZoneData as a result of load.
    ///
    /// It returns a pointer to ZoneData built in \c load() or
    /// \c loadIncremental() method.  If it's called before or in the middle
    /// of the load, it returns NULL.  If it's called after successful
    /// completion of the load, the return value must be non-NULL.
    ///
    /// \throw None.
    /// \return A pointer to the loaded ZoneData.
    virtual ZoneData* getLoadedData() const;

    /// \brief Complete any remaining loading task that deferred in load().
    ///
    /// Specifically, if load() found diffs of zone data to be applied later,
    /// this method now completes the task.  Therefore if the updated data
    /// are shared by multiple threads, call to this method must be protected
    /// in a critical section.
    ///
    /// This method propagates any internal exceptions.  The only expected
    /// exception in normal condition is MemorySegmentMapped.  The caller is
    /// expected to catch it and re-try calling this method after resetting
    /// the segment.  In this case this method provides strong exception
    /// guarantee.
    ///
    /// For all other cases it does not provide strong exception guarantee; if
    /// it throws, the passed data might have been modified and may not be
    /// recoverable to the original data.  In general, this API assumes data
    /// applied in this method have been validated at a higher layer and this
    /// method does not fail except for unexpected system errors, which would
    /// normally be considered a fatal condition.  So the caller is expected to
    /// consider the data invalid and rebuild it from the scratch on exception.
    ///
    /// In any case, this method does not destroy the given data.  It's the
    /// caller's responsibility.
    ///
    /// \throw MemorySegmentMapped shared-type memory segment was remapped
    /// (see above).
    /// \throw Other (see above)
    ///
    /// \param update_data Currently available zone data (usually the returned
    /// pointer from load()).
    /// \return final zone data after applying any deferred operation.  This
    /// can be different from the passed pointer, but is never NULL.
    virtual ZoneData* commit(ZoneData* update_data);

private:
    ZoneDataLoaderImpl* impl_;
};
} // namespace memory
} // namespace datasrc
} // namespace bundy

#endif // DATASRC_ZONE_DATA_LOADER_H

// Local Variables:
// mode: c++
// End:
