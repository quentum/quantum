#include <iostream>

#include "base/at_exit.h"

#include "names.h"
#include "messages.h"
#include "channel_worker.h"
#include "channel_department.h"

namespace demo {

class MainChannelWorker : public ChannelWorker {
public:
    MainChannelWorker() {}
    ~MainChannelWorker() {}
public:
    typedef base::Callback<void(const std::string&)> OnCreateSubChannel;
    void SetCreateCallback(OnCreateSubChannel callback) {
        _create_callback = callback;
    }

    bool OnMessageReceived(const IPC::Message& message) override {
        IPC_BEGIN_MESSAGE_MAP(MainChannelWorker, message)
            IPC_MESSAGE_HANDLER(ChannelCreateResponsetMessage, OnCreateChannelResponseMessage)
        IPC_END_MESSAGE_MAP()
        return true;
    }

    void OnChannelConnected(int32_t peer_pid) override {
        ChannelWorker::OnChannelConnected(peer_pid);
        Send(new ChannelCreateRequestMessage(0, "i am a client"));
    }
private:
    void OnCreateChannelResponseMessage(const std::string& name) {
        LOG(INFO) << "sub channel name received:" << name;
        _create_callback.Run(name);
        Close();
    }
private:
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
private:
    void OnEchoMessage(const std::string& message) {
        LOG(INFO) << "echo message: " << message;
    }
};

class Client {
public:
    Client() : _worker(nullptr) {}
    ~Client() {}
public:
    void Run() {
        _loop.reset(new base::MessageLoop);
        this->Start();
        _loop->Run();
    }

    void Start() {
        _monitoring.reset(new base::Thread("monitoring std input thread"));
        _monitoring->Start();
        _monitoring->message_loop()->PostTask(FROM_HERE, base::Bind(&Client::MonitoringQuit, base::Unretained(this)));

        _department.reset(new ChannelDepartment(MAIN_CHANNEL_NAME));
        _department->Start();

        ChannelWorker::Options options;
        options.handle.name = MAIN_CHANNEL_NAME;
        options.mode = IPC::Channel::MODE_NAMED_CLIENT;
        options.runner = _department->task_runner();
        MainChannelWorker* mainWorker = new MainChannelWorker();
        mainWorker->Initialize(options);
        mainWorker->SetCreateCallback(base::Bind(&Client::OnCreateSubChannel, base::Unretained(this)));
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
        options.mode = IPC::Channel::MODE_NAMED_CLIENT;
        options.runner = _department->task_runner();

        _worker = new EchoChannelWorker();
        _worker->Initialize(options);
        _department->AddWorker(_worker);
    }

    void MonitoringQuit() {
        std::string cmd;
        while(true) {
            std::cin >> cmd;
            if (cmd == "quit") {
                break;
            } else {
                _worker->Send(new ChannelEchoMessage(0, cmd));
            }
        }
        _loop->PostTask(FROM_HERE, base::Bind(&Client::Stop, base::Unretained(this)));
    }
private:
    scoped_ptr<base::MessageLoop> _loop;
    scoped_ptr<ChannelDepartment> _department;
    scoped_ptr<base::Thread> _monitoring;
    EchoChannelWorker* _worker;
};

}

int main(int argc, char** argv) {
    base::AtExitManager atexit_manager;
    demo::Client client;
    client.Run();
    return 0;
}
