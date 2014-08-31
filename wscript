import sys
import os

APPNAME = 'jsonhandle'
VERSION = '0.5.2'

top = '.'
out = 'build' + os.sep + sys.platform

def options(ctx):
    ctx.load('compiler_cxx')

def configure(ctx):
    if sys.platform == 'win32':
        ctx.env['MSVC_TARGETS'] = ['x86']
        ctx.load('msvc')
    else:
        ctx.load('compiler_cxx')

def build(ctx):
    ctx.install_files('${PREFIX}/include', [
        'src/_JS0.h',
        'src/JsonHandle.h',
        ], relative_trick=False)

    ctx.stlib(
        source = [
            'src/_JS0.cpp',
            'src/JsonHandle.cpp'
        ],
        target = 'jsonhandle',
        install_path = '${PREFIX}/lib'
        )

# vim: set filetype=python softtabstop=4 expandtab shiftwidth=4 tabstop=4:
