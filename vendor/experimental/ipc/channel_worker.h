#ifndef __EXPERIMENTAL_IPC_CHANNEL_WORKER_H__
#define __EXPERIMENTAL_IPC_CHANNEL_WORKER_H__

#include "ipc/ipc_channel_proxy.h"

namespace demo {

class ChannelWorker : public IPC::Listener, public IPC::Sender {
public:
    struct Options {
        IPC::ChannelHandle handle;
        IPC::Channel::Mode mode;
        scoped_refptr<base::SingleThreadTaskRunner> runner;
    };
    ChannelWorker();
    virtual ~ChannelWorker();
public:
    bool Initialize(const Options& options);
    bool OnMessageReceived(const IPC::Message& message) override;
    void OnChannelConnected(int32_t peer_pid) override;
    void OnChannelError() override;
    bool Send(IPC::Message* msg) override;
    void Close();
private:
    class ChannelWorkerImpl;
    scoped_ptr<ChannelWorkerImpl> _impl;
    DISALLOW_COPY_AND_ASSIGN(ChannelWorker);
};

} // namespace demo
#endif // __EXPERIMENTAL_IPC_CHANNEL_WORKER_H__
