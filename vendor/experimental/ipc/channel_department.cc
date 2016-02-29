#include "channel_department.h"
#include "channel_worker.h"

namespace demo {

class ChannelDepartment::ChannelDepartmentImpl {
public:
    ChannelDepartmentImpl(const std::string& name) : _name(name) {}
    ~ChannelDepartmentImpl() {
        std::vector<ChannelWorker*>::iterator iter = _workers.begin();
        for(; iter != _workers.end(); ++iter) {
            (*iter)->Close();
            delete *iter;
        }
        _workers.clear();
    }
public:
    void Start() {
        DCHECK(_thread.get() == nullptr);
        base::Thread::Options options;
        options.message_loop_type = base::MessageLoop::TYPE_IO;
        _thread.reset(new base::Thread(_name));
        _thread->StartWithOptions(options);
    }

    void Stop() {
        DCHECK(_thread.get() != nullptr);
        _thread->StopSoon();
    }

    scoped_refptr<base::SingleThreadTaskRunner> task_runner() const {
        return _thread->task_runner();
    }

    void AddWorker(ChannelWorker* worker) {
        _workers.push_back(worker);
    }

    void RemoveWorker(ChannelWorker* worker) {
        std::vector<ChannelWorker*>::iterator iter = std::find(_workers.begin(), _workers.end(), worker);
        if (iter != _workers.end()) {
            _workers.erase(iter);
        }
        delete worker;
    }
private:
    std::string _name;
    scoped_ptr<base::Thread> _thread;
    std::vector<ChannelWorker*> _workers;
};

ChannelDepartment::ChannelDepartment(const std::string& name):
        _impl(new ChannelDepartmentImpl(name)) {
}

ChannelDepartment::~ChannelDepartment() {
}

void ChannelDepartment::Start() {
    _impl->Start();
}

void ChannelDepartment::Stop() {
    _impl->Stop();
}

scoped_refptr<base::SingleThreadTaskRunner> ChannelDepartment::task_runner() const {
    return _impl->task_runner();
}

void ChannelDepartment::AddWorker(ChannelWorker* worker) {
    _impl->AddWorker(worker);
}

void ChannelDepartment::RemoveWorker(ChannelWorker* worker) {
    _impl->RemoveWorker(worker);
}

}
