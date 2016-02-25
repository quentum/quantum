{
    'includes': [
        'modules.gypi',
    ],
    'targets': [
        {
            'target_name': 'vendor',
            'type': 'none',
            'conditions': [
                ['have_basic_modules=="true"', {
                    'dependencies': [
                        'ipc/ipc.gyp:ipc',
                    ]
                }],
                ['have_node_modules=="true"', {
                    'dependencies': [
                        'gin/gin.gyp:gin',
                        'node/node.gyp:node',
                        'base/base.gyp:base_i18n',
                        'native-mate/native_mate.gyp:native_mate',
                    ]
                }]
            ],
        }
    ]
}
