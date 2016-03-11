{
  'targets': [
    {
      'target_name': 'ipc_client',
      'type': 'executable',
      'sources': [
        'message_generater.cc',
        'automatic_channel.cc',
        'channel_map.cc',
        'client.cc',
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../../ipc/ipc.gyp:ipc'
      ]
    },
    {
      'target_name': 'ipc_server',
      'type': 'executable',
      'sources': [
        'message_generater.cc',
        'automatic_channel.cc',
        'channel_map.cc',
        'server.cc',
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../../ipc/ipc.gyp:ipc'
      ]
    },
    {
      'target_name': 'ipc_all',
      'type': 'none',
      'dependencies': [
        'ipc_client',
        'ipc_server'
      ]
    }
  ]
}
