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

#ifndef AUTH_DATASRC_CONFIG_H
#define AUTH_DATASRC_CONFIG_H

#include <cc/data.h>
#include <datasrc/client_list.h>

#include <boost/shared_ptr.hpp>

#include <utility>
#include <map>

/// \brief Configure data source client lists
///
/// This will hook into the data_sources module configuration and it will
/// return a new set (in the form of a shared pointer to map) of data source
/// client lists corresponding to the configuration.
///
/// This function is templated. This is simply because of easier testing.
/// You don't need to pay attention to it, use the configureDataSource
/// specialization instead.
///
/// \note In future we may want to make the reconfiguration more efficient
/// by only creating newly configured data and just moving the rest from
/// the running configuration if they are used in the new configuration
/// without any parameter change.  We could probably do it by passing
/// the old lists in addition to the new config, but further details are
/// still to be defined yet.  It will surely require changes in the
/// data source library, too.  So, right now, we don't introduce the
/// possibility in the function interface.  If and when we decide to introduce
/// the optimization, we'll extend the interface.
///
/// \param config The configuration value to parse. It is in the form
///     as an update from the config manager.
/// \return A map from RR classes to configured lists.
/// \throw ConfigurationError if the config element is not in the expected
///        format (A map of lists)
template<class List>
boost::shared_ptr<std::map<bundy::dns::RRClass,
                           boost::shared_ptr<List> > > // = ListMap below
configureDataSourceGeneric(const bundy::data::ConstElementPtr& config) {
    typedef boost::shared_ptr<List> ListPtr;
    typedef std::map<std::string, bundy::data::ConstElementPtr> Map;
    typedef std::map<bundy::dns::RRClass, ListPtr> ListMap;

    boost::shared_ptr<ListMap> new_lists(new ListMap);

    const Map& map(config->mapValue());
    for (Map::const_iterator it(map.begin()); it != map.end(); ++it) {
        const bundy::dns::RRClass rrclass(it->first);
        ListPtr list(new List(rrclass));
        list->configure(it->second, true);
        new_lists->insert(std::pair<bundy::dns::RRClass, ListPtr>(rrclass,
                                                                list));
    }

    return (new_lists);
}

/// \brief Concrete version of configureDataSource() for the
///     use with authoritative server implementation.
bundy::datasrc::ClientListMapPtr
configureDataSource(const bundy::data::ConstElementPtr& config);

#endif  // AUTH_DATASRC_CONFIG_H

// Local Variables:
// mode: c++
// End:
