#include "ipc/ipc_message_macros.h"

#define IPC_MESSAGE_START ShellMsgStart

IPC_MESSAGE_ROUTED(ChannelCreateRequestMessage,
                   std::string) // Version Info
IPC_MESSAGE_ROUTED(ChannelCreateResponsetMessage,
                   std::string) // New Pipe Name

IPC_MESSAGE_ROUTED(ChannelEchoMessage,
                   std::string) // For Test, echo message
