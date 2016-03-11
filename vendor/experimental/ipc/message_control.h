#include "ipc/ipc_message_macros.h"
#include "message_classes.h"

#define IPC_MESSAGE_START ChannelControlStart

IPC_MESSAGE_CONTROL(ChannelCloseMessage)

IPC_MESSAGE_CONTROL(ChannelCloseAckMessage)

IPC_MESSAGE_CONTROL(ChannelHeartbeatPing)

IPC_MESSAGE_CONTROL(ChannelHeartbeatPong)
