#include "automatic_channel.h"

#include "base/thread_task_runner_handle.h"
#include "ipc/ipc_channel_proxy.h"

#include "message_control.h"

namespace cif {

AutomaticChannel::AutomaticOptions::AutomaticOptions() :
        enable(true),
        interval(2000),
        max_idle_time(5000),
        heartbeat_timeout(3000),
        heartbeat_times(3) {
}

//////////////////////////////////////////////////////////////////////////////////////////////

AutomaticChannel::Options::Options() :
        id(0),
        mode(IPC::Channel::MODE_NAMED_SERVER),
        listener(nullptr) {
}

//////////////////////////////////////////////////////////////////////////////////////////////

class ChannelWatcher {
public:
    class Delegate : public IPC::Sender {
    public:
        virtual ~Delegate() { }
    public:
        virtual void OnCloseAck() = 0;
        virtual void OnChannelSuddenClose() = 0;
        virtual void OnChannelTimeout() = 0;
    };

    struct Options {
        AutomaticChannel::AutomaticOptions basic;
        std::string name;
    };

public:
    ChannelWatcher(Delegate* delegate) :
            _delegate(delegate),
            _state(ChannelState::Initialized),
            _heartbeat_times(0),
            _passive(true) {
        DCHECK(_delegate);
    }

    ~ChannelWatcher() { }

public:
    bool Initialize(const Options& options) {
        DCHECK(!options.name.empty());
        _options = options;
        return true;
    }

    bool Close() {
        _passive = false;
        if (_state != ChannelState::Initialized) {
            LOG(INFO) << "[" << _options.name << "]" << "on close function, send close message";
            return _delegate->Send(new ChannelCloseMessage);
        }

        return false;
    }

public:
    bool OnMessageReceived(const IPC::Message& message) {
        LOG(INFO) << "[" << _options.name << "]" << "message received";
        UpdateTime();

        IPC_BEGIN_MESSAGE_MAP(ChannelWatcher, message)
            IPC_MESSAGE_HANDLER(ChannelCloseMessage, OnCloseMessageReceived)
            IPC_MESSAGE_HANDLER(ChannelCloseAckMessage, OnCloseAckMessageReceived)
            IPC_MESSAGE_HANDLER(ChannelHeartbeatPing, OnHeartbeatPingMessageReceived)
            IPC_MESSAGE_HANDLER(ChannelHeartbeatPong, OnHeartbeatPongMessageReceived)
        IPC_END_MESSAGE_MAP()

        return true;
    }

    void OnChannelConnected() {
        LOG(INFO) << "[" << _options.name << "]" << "channel connected";
        _state = ChannelState::Working;
        if (_options.basic.enable) {
            UpdateTime();
            _timer.Start(FROM_HERE,
                         base::TimeDelta::FromMilliseconds(_options.basic.interval),
                         base::Bind(&ChannelWatcher::OnTimer, base::Unretained(this)));
         }
    }

    void OnChannelError() {
        if (_passive) {
            LOG(INFO) << "[" << _options.name << "]" << "channel suddenly closed";
            if (_options.basic.enable) {
                _delegate->OnChannelSuddenClose();
            }
        } else {
            LOG(INFO) << "[" << _options.name << "]" << "channel closed by user";
        }

        Reset();
    }

private:
    void Reset() {
        _passive = true;
        if (_options.basic.enable) {
            _timer.Stop();
        }
        _state = ChannelState::Initialized;
        _heartbeat_times = 0;
    }

    void UpdateTime() {
        LOG(INFO) << "[" << _options.name << "]" << "update time to " << _time.ToInternalValue();
        _time = base::TimeTicks::Now();
    }

    void OnTimer() {
        base::TimeDelta delta = base::TimeTicks::Now() - _time;
        switch(_state) {
        case ChannelState::Initialized:
            NOTREACHED() << "[" << _options.name << "]" << "OnTimer fired impossible reached here!";
            break;
        case ChannelState::Working:
            if (delta.InMilliseconds() > _options.basic.max_idle_time) {
                UpdateTime();
                LOG(INFO) << "[" << _options.name << "]" << "No message received, send a heartbeat!";
                _delegate->Send(new ChannelHeartbeatPing);
                _state = ChannelState::Heartbeat;
            }
            break;
        case ChannelState::Heartbeat:
            if (delta.InMilliseconds() > _options.basic.heartbeat_timeout) {
                if (++_heartbeat_times >= _options.basic.heartbeat_times) {
                    _state = ChannelState::HeartbeatTimeout;
                    LOG(INFO) << "[" << _options.name << "]" << "heartbeat time grow to " << _heartbeat_times;
                } else {
                    UpdateTime();
                    LOG(INFO) << "[" << _options.name << "]" << "No message received, send a heartbeat!";
                    _delegate->Send(new ChannelHeartbeatPing);
                }
            }
            break;
        case ChannelState::HeartbeatTimeout:
            LOG(INFO) << "[" << _options.name << "]" << "heartbeat timeout, finally!!!";
            _delegate->OnChannelTimeout();
            Reset();
            break;
        }
    }

    void OnCloseMessageReceived() {
        LOG(INFO) << "[" << _options.name << "]" << "received Close message";
        _passive = false;
        _delegate->Send(new ChannelCloseAckMessage);
    }

    void OnCloseAckMessageReceived() {
        LOG(INFO) << "[" << _options.name << "]" << "received Close ACK message";
        _passive = false;
        _delegate->OnCloseAck();
    }

    void OnHeartbeatPingMessageReceived() {
        _delegate->Send(new ChannelHeartbeatPong);
        LOG(INFO) << "[" << _options.name << "]" << "received heartbeat Ping message";
    }

    void OnHeartbeatPongMessageReceived() {
        _state = ChannelState::Working;
        _heartbeat_times = 0;
        LOG(INFO) << "[" << _options.name << "]" << "received heartbeat Pong message";
    }
private:
    enum ChannelState {
        Initialized,
        Working,
        Heartbeat,
        HeartbeatTimeout
    };

private:
    Delegate* _delegate;
    Options _options;
    ChannelState _state;
    int _heartbeat_times;
    bool _passive;
    base::TimeTicks _time;
    base::RepeatingTimer _timer;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class AutomaticChannel::AutomaticChannelImpl : public IPC::Listener,
                                               public ChannelWatcher::Delegate {
public:
    AutomaticChannelImpl() { }
    ~AutomaticChannelImpl() { }
public: // 对外接口实现
    bool Initialize(const Options& options) {
        DCHECK(options.id != 0
               && !options.name.empty()
               && options.mode != IPC::Channel::MODE_NONE
               && options.runner
               && options.listener);

        _options = options;

        ChannelWatcher::Options watch_options;
        watch_options.basic = options.automatic_options;
        watch_options.name = options.name;
        _watcher.reset(new ChannelWatcher(this));
        _watcher->Initialize(watch_options);

        return CreateChannel();
    }

    // IPC::Sender 接口实现
    bool Send(IPC::Message* msg) override {
        if (!_channel) {
            return false;
        }

        LOG(INFO) << "[" << _options.name << "]" << "Send Message";
        return _channel->Send(msg);
    }

    void Close() {
        if(_watcher->Close()) {
            return;
        }

        _channel->Close();
    }

    int32_t GetID() const {
        return _options.id;
    }

public: // IPC::Listener 接口实现
    bool OnMessageReceived(const IPC::Message& message) override {
        _watcher->OnMessageReceived(message);
        return _options.listener->OnMessageReceived(_options.id, message);
    }

    void OnChannelConnected(int32_t peer_pid) override {
        _watcher->OnChannelConnected();
        _options.listener->OnChannelConnected(_options.id, peer_pid);
    }

    void OnChannelError() override {
        _watcher->OnChannelError();
        _options.listener->OnChannelError(_options.id);
    }

    void OnBadMessageReceived(const IPC::Message& message) override {
        _options.listener->OnBadMessageReceived(_options.id, message);
    }

public: // ChannelWatcher::Delegate 接口实现
    void OnCloseAck() override {
        _channel->Close();
        delete this;
    }

    void OnChannelSuddenClose() override {
        OnChannelTimeout();
    }

    void OnChannelTimeout() override {
        _channel->Close();
        CreateChannel();
    }

private:
    bool CreateChannel() {
        IPC::ChannelHandle handle;
        handle.name = _options.name;
        _channel = IPC::ChannelProxy::Create(handle, _options.mode, this, _options.runner);
        return _channel.get() != nullptr;
    }

private:
    friend class ChannelList;


private:
    Options _options;
    scoped_ptr<IPC::ChannelProxy> _channel;
    scoped_ptr<ChannelWatcher> _watcher;
};

//////////////////////////////////////////////////////////////////////////////////////////////

AutomaticChannel::AutomaticChannel() : _impl(new AutomaticChannelImpl) {
}

AutomaticChannel::~AutomaticChannel() {
}

bool AutomaticChannel::Initialize(const Options& options) {
    return _impl->Initialize(options);
}

bool AutomaticChannel::Send(IPC::Message* message) {
    return _impl->Send(message);
}

void AutomaticChannel::Close() {
    _impl->Close();
}

int32_t AutomaticChannel::GetID() const {
    return _impl->GetID();
}

} // namespace cif
