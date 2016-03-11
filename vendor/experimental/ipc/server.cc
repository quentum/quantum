#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"

#include "names.h"
#include "message_classes.h"
#include "message_route.h"

#include "automatic_channel.h"
#include "channel_map.h"

namespace cif {

class Server : public AutomaticChannel::Listener {
private:
    enum ChannelType : int32_t {
        ControlChannel = 0x20160310,
        SubChannel
    };

public:
    Server() : _current(nullptr) {}

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
            IPC_BEGIN_MESSAGE_MAP(Server, message)
                IPC_MESSAGE_HANDLER(CreateRequestMsg, OnCreateRequest)
            IPC_END_MESSAGE_MAP()
        } else {
            IPC_BEGIN_MESSAGE_MAP(Server, message)
                IPC_MESSAGE_HANDLER(EchoMessage, OnEchoMessage)
            IPC_END_MESSAGE_MAP()
        }

        return true;
    }

    void OnChannelConnected(int32_t id, int32_t peer_pid) {
    }

    void OnChannelError(int32_t id) {
    }

private:
    void Start() {
        _quit_thread->Start();
        _quit_thread->message_loop()->PostTask(FROM_HERE,
                    base::Bind(&Server::Quit, base::Unretained(this)));

        {
            base::Thread::Options options;
            options.message_loop_type = base::MessageLoop::TYPE_IO;
            _io_thread->StartWithOptions(options);
        }

        {
            AutomaticChannel::Options options;
            options.id = static_cast<int32_t>(ChannelType::ControlChannel);
            options.name = MAIN_CHANNEL_NAME;
            options.mode = IPC::Channel::MODE_NAMED_SERVER;
            options.runner = _io_thread->task_runner();
            options.listener = this;
            options.automatic_options.enable = true;

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
            }
        }
        _main_loop->PostTask(FROM_HERE, base::Bind(&Server::Stop, base::Unretained(this)));
    }
private:
    void OnCreateRequest(const std::string& hello) {
        static int index = 0;
        std::string name = base::StringPrintf("%s_%d", SUB_CHANNEL_NAME_PREFIX, index);

        AutomaticChannel::Options options;
        options.id = static_cast<int32_t>(ChannelType::SubChannel) + index++;
        options.name = name;
        options.mode = IPC::Channel::MODE_NAMED_SERVER;
        options.runner = _io_thread->task_runner();
        options.listener = this;
        options.automatic_options.enable = true;

        AutomaticChannel* control_channel = new AutomaticChannel;
        control_channel->Initialize(options);
        _map.Add(control_channel);

        _current->Send(new CreateResponseMsg(0, name));
    }

    void OnEchoMessage(const std::string message) {
        // _current->Send(new EchoMessage(0, message));
    }

private:
    ChannelMap _map;
    scoped_ptr<base::MessageLoop> _main_loop;
    scoped_ptr<base::Thread> _quit_thread;
    scoped_ptr<base::Thread> _io_thread;
    AutomaticChannel* _current;
    DISALLOW_COPY_AND_ASSIGN(Server);
};

}

int main(int argc, char** argv) {
    base::CommandLine::Init(argc, argv);
    base::AtExitManager atexit_manager;

    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    InitLogging(settings);
    logging::SetLogItems(true, true, true, true);

    cif::Server server;
    server.Initialize();
    server.Run();
    return 0;
}
