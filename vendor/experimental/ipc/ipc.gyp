{
  'targets': [
    {
      'target_name': 'ipc_posix_client',
      'type': 'executable',
      'sources': [
        'message_generater.cc',
        'channel_department.cc',
        'channel_worker.cc',
        'client.cc'
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../../ipc/ipc.gyp:ipc'
      ]
    },
    {
      'target_name': 'ipc_posix_server',
      'type': 'executable',
      'sources': [
        'message_generater.cc',
        'channel_department.cc',
        'channel_worker.cc',
        'server.cc'
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../../ipc/ipc.gyp:ipc'
      ]
    },
    {
      'target_name': 'ipc_posix_all',
      'type': 'none',
      'dependencies': [
        'ipc_posix_client',
        'ipc_posix_server'
      ]
    }
  ]
}
