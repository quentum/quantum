#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"

#include "names.h"
#include "messages.h"
#include "channel_worker.h"
#include "channel_department.h"

namespace demo {

class MainChannelWorker : public ChannelWorker {
public:
    MainChannelWorker() : _index(0) {}
    ~MainChannelWorker() {}
public:
    typedef base::Callback<void(const std::string&)> OnCreateSubChannel;
    void SetCreateCallback(OnCreateSubChannel callback) {
        _create_callback = callback;
    }

    bool OnMessageReceived(const IPC::Message& message) override {
        IPC_BEGIN_MESSAGE_MAP(MainChannelWorker, message)
            IPC_MESSAGE_HANDLER(ChannelCreateRequestMessage, OnCreateChannelMessage)
        IPC_END_MESSAGE_MAP()
        return true;
    }
private:
    void OnCreateChannelMessage(const std::string& version) {
        LOG(INFO) << "create sub channel message received";

        std::string name = base::StringPrintf("%s_%d", SUB_CHANNEL_NAME_PREFIX, _index++);
        _create_callback.Run(name);
        Send(new ChannelCreateResponsetMessage(0, name));
    }
private:
    int _index;
    base::Callback<void(const std::string&)> _create_callback;
};

class EchoChannelWorker : public ChannelWorker {
public:
    EchoChannelWorker() {}
    ~EchoChannelWorker() {}
public:
    bool OnMessageReceived(const IPC::Message& message) override {
        IPC_BEGIN_MESSAGE_MAP(EchoChannelWorker, message)
            IPC_MESSAGE_HANDLER(ChannelEchoMessage, OnEchoMessage)
        IPC_END_MESSAGE_MAP()
        return true;
    }

    void OnChannelError() override {
        ChannelWorker::OnChannelError();
    }
private:
    void OnEchoMessage(const std::string& message) {
        LOG(INFO) << "echo message received: " << message;
        Send(new ChannelEchoMessage(0, message));
    }
};

class Server {
public:
    Server() {}
    ~Server() {}
public:
    void Run() {
        _loop.reset(new base::MessageLoop);
        this->Start();
        _loop->Run();
    }

    void Start() {
        _monitoring.reset(new base::Thread("monitoring std input thread"));
        _monitoring->Start();
        _monitoring->message_loop()->PostTask(FROM_HERE, base::Bind(&Server::MonitoringQuit, base::Unretained(this)));

        _department.reset(new ChannelDepartment(MAIN_CHANNEL_NAME));
        _department->Start();

        ChannelWorker::Options options;
        options.handle.name = MAIN_CHANNEL_NAME;
        options.mode = IPC::Channel::MODE_NAMED_SERVER;
        options.runner = _department->task_runner();
        MainChannelWorker* mainWorker = new MainChannelWorker();
        mainWorker->Initialize(options);
        mainWorker->SetCreateCallback(base::Bind(&Server::OnCreateSubChannel, base::Unretained(this)));
        _department->AddWorker(mainWorker);
    }

    void Stop() {
        _department->Stop();
        _loop->QuitWhenIdle();
    }
private:
    void OnCreateSubChannel(const std::string& name) {
        LOG(INFO) << "create sub channel: " << name;

        ChannelWorker::Options options;
        options.handle.name = name;
        options.mode = IPC::Channel::MODE_NAMED_SERVER;
        options.runner = _department->task_runner();

        EchoChannelWorker* worker = new EchoChannelWorker();
        worker->Initialize(options);
        _department->AddWorker(worker);
    }

    void MonitoringQuit() {
        std::string cmd;
        while(true) {
            std::cin >> cmd;
            if (cmd == "quit") {
                break;
            }
        }
        _loop->PostTask(FROM_HERE, base::Bind(&Server::Stop, base::Unretained(this)));
    }
private:
    scoped_ptr<base::MessageLoop> _loop;
    scoped_ptr<ChannelDepartment> _department;
    scoped_ptr<base::Thread> _monitoring;
};

}

int main(int argc, char** argv) {
    base::CommandLine::Init(argc, argv);
    base::AtExitManager atexit_manager;

    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    InitLogging(settings);

    demo::Server server;
    server.Run();
    return 0;
}
