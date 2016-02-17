{
  'targets': [
    {
      'target_name': 'vendor',
      'type': 'none',
      'dependencies': [
        'gin/gin.gyp:gin',
        'node/node.gyp:node',
        'base/base.gyp:base_i18n',
        'native-mate/native_mate.gyp:native_mate',
      ]
    }
  ]
}
