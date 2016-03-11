#ifndef __EXPERIMENTAL_IPC_CHANNEL_MAP_H__
#define __EXPERIMENTAL_IPC_CHANNEL_MAP_H__

#include "base/macros.h"
#include "base/id_map.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread.h"

namespace cif {

class AutomaticChannel;

class ChannelMap {
public:
    ChannelMap();
    ~ChannelMap();

public:
    typedef IDMap<AutomaticChannel> ChannelIDMap;
    typedef ChannelIDMap::const_iterator const_iterator;
    typedef ChannelIDMap::iterator iterator;

    const_iterator Iterator() const;
    iterator Iterator();

    bool empty() const {return _channels.IsEmpty();}
    size_t size() const {return _channels.size();}

public:
    void Add(AutomaticChannel* channel);
    AutomaticChannel* Get(int32_t id) const;
    void Remove(AutomaticChannel* channel);
    void Clear();

private:
    ChannelIDMap _channels;
    DISALLOW_COPY_AND_ASSIGN(ChannelMap);
};

}   // namespace demo
#endif // __EXPERIMENTAL_IPC_CHANNEL_MAP_H__
