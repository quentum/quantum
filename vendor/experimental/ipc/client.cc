#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"

#include "names.h"
#include "message_classes.h"
#include "message_route.h"

#include "automatic_channel.h"
#include "channel_map.h"

namespace cif {

class Client : public AutomaticChannel::Listener {
private:
    enum ChannelType : int32_t {
        ControlChannel = 0x20160310,
        SubChannel
    };

public:
    Client() : _current(nullptr) {}

    void Initialize() {
        _main_loop.reset(new base::MessageLoop);
        _quit_thread.reset(new base::Thread("QuitMonitorThread"));
        _io_thread.reset(new base::Thread("IOThread"));
    }

    void Run() {
        Start();
        _main_loop->Run();
    }

public:
    bool OnMessageReceived(int32_t id, const IPC::Message& message) {
        _current = _map.Get(id);

        ChannelType type = static_cast<ChannelType>(id);
        if (type == ChannelType::ControlChannel) {
            IPC_BEGIN_MESSAGE_MAP(Client, message)
                IPC_MESSAGE_HANDLER(CreateResponseMsg, OnCreateResponse)
            IPC_END_MESSAGE_MAP()
        } else {
            IPC_BEGIN_MESSAGE_MAP(Client, message)
                IPC_MESSAGE_HANDLER(EchoMessage, OnEchoMessage)
            IPC_END_MESSAGE_MAP()
        }

        return true;
    }

    void OnChannelConnected(int32_t id, int32_t peer_pid) {
        _current = _map.Get(id);
        ChannelType type = static_cast<ChannelType>(id);
        if (type == ChannelType::ControlChannel) {
            _current->Send(new CreateRequestMsg(0, "Hello World From Client!"));
        }
    }

    void OnChannelError(int32_t id) {
    }

private:
    void Start() {
        _quit_thread->Start();
        _quit_thread->message_loop()->PostTask(FROM_HERE,
                    base::Bind(&Client::Quit, base::Unretained(this)));

        {
            base::Thread::Options options;
            options.message_loop_type = base::MessageLoop::TYPE_IO;
            _io_thread->StartWithOptions(options);
        }

        {
            AutomaticChannel::Options options;
            options.id = static_cast<int32_t>(ChannelType::ControlChannel);
            options.name = MAIN_CHANNEL_NAME;
            options.mode = IPC::Channel::MODE_NAMED_CLIENT;
            options.runner = _io_thread->task_runner();
            options.listener = this;
            options.automatic_options.enable = false;

            AutomaticChannel* control_channel = new AutomaticChannel;
            control_channel->Initialize(options);
            _map.Add(control_channel);
        }
    }

    void Stop() {
        _map.Clear();
        _quit_thread->StopSoon();
        _io_thread->StopSoon();
        _main_loop->QuitWhenIdle();
    }

    void Quit() {
        std::string cmd;
        while(true) {
            std::cin >> cmd;
            if (cmd == "q") {
                break;
            } else {
                _main_loop->PostTask(FROM_HERE, base::Bind(&Client::SendEcho, base::Unretained(this), cmd));
            }
        }
        _main_loop->PostTask(FROM_HERE, base::Bind(&Client::Stop, base::Unretained(this)));
    }
private:
    void OnCreateResponse(const std::string& name) {
        _map.Remove(_current);

        AutomaticChannel::Options options;
        options.id = static_cast<int32_t>(ChannelType::SubChannel);
        options.name = name;
        options.mode = IPC::Channel::MODE_NAMED_CLIENT;
        options.runner = _io_thread->task_runner();
        options.listener = this;
        options.automatic_options.enable = false;

        AutomaticChannel* control_channel = new AutomaticChannel;
        control_channel->Initialize(options);
        _map.Add(control_channel);
    }

    void SendEcho(const std::string& message) {
        AutomaticChannel* channel = _map.Get(ChannelType::SubChannel);
        if (channel != nullptr) {
            channel->Send(new EchoMessage(0, message));
        }
    }

    void OnEchoMessage(const std::string message) {
        _current->Send(new EchoMessage(0, message));
    }

private:
    ChannelMap _map;
    scoped_ptr<base::MessageLoop> _main_loop;
    scoped_ptr<base::Thread> _quit_thread;
    scoped_ptr<base::Thread> _io_thread;
    AutomaticChannel* _current;
    DISALLOW_COPY_AND_ASSIGN(Client);
};

}

int main(int argc, char** argv) {
    base::CommandLine::Init(argc, argv);
    base::AtExitManager atexit_manager;

    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    InitLogging(settings);
    logging::SetLogItems(true, true, true, true);

    cif::Client client;
    client.Initialize();
    client.Run();
    return 0;
}
