#include "channel_map.h"
#include "automatic_channel.h"

namespace cif {

ChannelMap::ChannelMap() {
}

ChannelMap::~ChannelMap() {
    Clear();
}

ChannelMap::const_iterator ChannelMap::Iterator() const {
    return const_iterator(const_cast<ChannelIDMap*>(&_channels));
}

ChannelMap::iterator ChannelMap::Iterator() {
    return iterator(&_channels);
}

void ChannelMap::Add(AutomaticChannel* channel) {
    _channels.AddWithID(channel, channel->GetID());
}

AutomaticChannel* ChannelMap::Get(int32_t id) const {
    return _channels.Lookup(id);
}

void ChannelMap::Remove(AutomaticChannel* channel) {
    channel->Close();
    _channels.Remove(channel->GetID());
}

void ChannelMap::Clear() {
    iterator iter = Iterator();
    while(!iter.IsAtEnd()) {
        AutomaticChannel* channel = iter.GetCurrentValue();
        iter.Advance();
        channel->Close();
    }

    _channels.Clear();
}

}
