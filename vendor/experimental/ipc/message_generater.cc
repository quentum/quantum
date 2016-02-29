#define IPC_MESSAGE_IMPL
#include "message_generater.h"

// Generate constructors.
#include "ipc/struct_constructor_macros.h"
#include "message_generater.h"

// Generate destructors.
#include "ipc/struct_destructor_macros.h"
#include "message_generater.h"

// Generate param traits write methods.
#include "ipc/param_traits_write_macros.h"
namespace IPC {
#include "message_generater.h"
}  // namespace IPC

// Generate param traits read methods.
#include "ipc/param_traits_read_macros.h"
namespace IPC {
#include "message_generater.h"
}  // namespace IPC

// Generate param traits log methods.
#include "ipc/param_traits_log_macros.h"
namespace IPC {
#include "message_generater.h"
}  // namespace IPC
