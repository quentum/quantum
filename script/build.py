#!/usr/bin/env python

import sys
import os
import argparse
import util
import ctypes

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
VENDOR_DIR = os.path.join(SOURCE_ROOT, 'vendor')

def main():
    args = parse_args()
    check_args(args)
    build_vendor(args)

def parse_args():
    parser = argparse.ArgumentParser(description='build vendor')
    parser.add_argument('--target-arch',                            \
                        action = 'store',                           \
                        choices = ['x86', 'x64'],                   \
                        default = util.default_arch(),              \
                        help = 'specify the arch to build for')
    parser.add_argument('--target',                                 \
                        action = 'store',                           \
                        nargs = '+',                                \
                        default = ['ipc'],                            \
                        help = 'specify the target to build')
    parser.add_argument('--configuration',                          \
                        action = 'store',                           \
                        nargs = '+',                                \
                        default = ['Debug'],                        \
                        help = 'build with Release or Debug')
    return parser.parse_args()

def check_args(args):
    if sys.platform == 'darwin' and args.target_arch == 'x86':
        raise Exception('do not support x86 on mac')

    configuration_len = len(args.configuration)
    if configuration_len > 2:
        raise Exception('configuration count must be 1 or 2')

    valid_configuration = ['Debug', 'Release']
    for configuration in args.configuration:
        if not configuration in valid_configuration:
            raise Exception('unknown configuration ' + configuration)

def build_vendor(args):
    os.chdir(VENDOR_DIR)
    ninja = get_ninja()
    for config in args.configuration:
        build_dir = os.path.join('out', config)
        if args.target_arch == 'x64' and sys.platform == 'win32':
            build_dir += '_x64'
        util.execute_stdout([ninja, '-C', build_dir] + args.target)

def get_ninja():
    ninja = os.path.join('tools', 'depot_tools', 'ninja')
    if sys.platform == 'win32':
        ninja += '.exe'
    return ninja

if __name__ == '__main__':
    sys.exit(main())
