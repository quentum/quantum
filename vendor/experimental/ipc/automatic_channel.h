#ifndef __EXPERIMENTAL_IPC_AUTOMATIC_CHANNEL_H__
#define __EXPERIMENTAL_IPC_AUTOMATIC_CHANNEL_H__

#include "ipc/ipc_channel.h"

namespace base {
class SingleThreadTaskRunner;
}

namespace cif {

// 具有自动重连功能的Channel
class AutomaticChannel {
public:
    AutomaticChannel();
    ~AutomaticChannel();

public:
    struct Options;
    bool Initialize(const Options& options);
    bool Send(IPC::Message* message);
    void Close();

public:
    int32_t GetID() const;

public:
    class Listener {
    public:
        virtual ~Listener() {}
        virtual bool OnMessageReceived(int32_t id, const IPC::Message& message) = 0;
        virtual void OnChannelConnected(int32_t id, int32_t peer_pid) {}
        virtual void OnChannelError(int32_t id) {}
        virtual void OnBadMessageReceived(int32_t id, const IPC::Message& message) {}
    };

    struct AutomaticOptions {
        // 是否自动重连， 默认值 true
        bool enable;
        // 检查通道是否联通的时间间隔，单位毫秒， 默认值2000
        int interval;
        // 最大空闲时间, 单位毫秒， 默认值5000
        int max_idle_time;
        // 心跳包超时时间, 单位毫秒， 默认值3000
        int heartbeat_timeout;
        // 心跳包发送次数， 默认值3
        int heartbeat_times;
        // 设置默认值
        AutomaticOptions();
    };

    struct Options {
        // 通道ID, 必须显式设置
        int32_t id;
        // 通道名称, 必须显式设置
        std::string name;
        // 通道模式, 必须显式设置
        IPC::Channel::Mode mode;
        // 通道需要允许哪个在的TaskRunner中, 必须显式设置
        scoped_refptr<base::SingleThreadTaskRunner> runner;
        // 通道信息监视器, 必须显式设置
        Listener* listener;
        // 自动重连选项
        AutomaticOptions automatic_options;
        // 设置默认值
        Options();
    };

private:
    class AutomaticChannelImpl;
    scoped_ptr<AutomaticChannelImpl> _impl;
    DISALLOW_COPY_AND_ASSIGN(AutomaticChannel);
};

} // namespace cif
#endif // __EXPERIMENTAL_IPC_AUTOMATIC_CHANNEL_H__
