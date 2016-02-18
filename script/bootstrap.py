#!/usr/bin/env python

import sys
import os
import util

SOURCE_ROOT = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))

def main():
    os.chdir(SOURCE_ROOT)
    if sys.platform != 'win32':
        update_clang()
    update_submodules()

def update_clang():
    util.execute_stdout([os.path.join('script', 'update-clang.sh')])

def update_submodules():
    util.execute_stdout(['git', 'submodule', 'sync'])
    util.execute_stdout(['git', 'submodule', 'update', '--init', '--recursive'])

if __name__ == '__main__':
    sys.exit(main())
