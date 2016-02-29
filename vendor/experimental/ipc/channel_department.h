#ifndef __EXPERIMENTAL_IPC_CHANNEL_DEPARTMENT_H__
#define __EXPERIMENTAL_IPC_CHANNEL_DEPARTMENT_H__

#include "base/threading/thread.h"
#include "ipc/ipc_channel_proxy.h"

namespace demo {

class ChannelWorker;

class ChannelDepartment {
public:
    ChannelDepartment(const std::string& name);
    ~ChannelDepartment();
public:
    void Start();
    void Stop();
public:
    scoped_refptr<base::SingleThreadTaskRunner> task_runner() const;
public:
    void AddWorker(ChannelWorker* worker);
    void RemoveWorker(ChannelWorker* worker);
private:
    class ChannelDepartmentImpl;
    scoped_ptr<ChannelDepartmentImpl> _impl;
    DISALLOW_COPY_AND_ASSIGN(ChannelDepartment);
};

}   // namespace demo
#endif // __EXPERIMENTAL_IPC_CHANNEL_DEPARTMENT_H__
