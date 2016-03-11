#include "ipc/ipc_message_macros.h"
#include "message_classes.h"

#define IPC_MESSAGE_START ChannelMessageStart

IPC_MESSAGE_ROUTED(CreateRequestMsg,
                   std::string) // Version Info

IPC_MESSAGE_ROUTED(CreateResponseMsg,
                   std::string) // New Pipe Name

IPC_MESSAGE_ROUTED(EchoMessage,
                   std::string) // For Test, echo message
