#include "channel_worker.h"

namespace demo {

class ChannelWorker::ChannelWorkerImpl {
public:
    ChannelWorkerImpl() {}
    ~ChannelWorkerImpl() {}

public:
    bool Initialize(const Options& options, IPC::Listener* listener) {
        _proxy = IPC::ChannelProxy::Create(options.handle, options.mode, listener, options.runner);
        return true;
    }

    bool Send(IPC::Message* msg) {
        return _proxy->Send(msg);
    }

    void Close() {
        _proxy->Close();
    }
private:
    scoped_ptr<IPC::ChannelProxy> _proxy;
};


ChannelWorker::ChannelWorker() : _impl(new ChannelWorkerImpl){}

ChannelWorker::~ChannelWorker() {}

bool ChannelWorker::Initialize(const Options& options) {
    return _impl->Initialize(options, this);
}

bool ChannelWorker::OnMessageReceived(const IPC::Message& message) {
    return true;
}

void ChannelWorker::OnChannelConnected(int32_t peer_pid) {
    LOG(INFO) << "channel connected with pid:" << peer_pid;
}

void ChannelWorker::OnChannelError() {
    LOG(INFO) << "channel error";
}

bool ChannelWorker::Send(IPC::Message* msg) {
    return _impl->Send(msg);
}

void ChannelWorker::Close() {
    LOG(INFO) << "ChannelWorker close";
    _impl->Close();
}
}
