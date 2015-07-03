#!/usr/bin/env python
import os
import subprocess
from waflib import Logs
from waflib.Configure import conf

###########################################################################
# Project specific options used by waf
###########################################################################

VERSION = '0.0.1'
APPNAME = 'NewsFlash'

top = '.'
out = 'tmp'
default_prefix = '.'

###########################################################################
# Project specific options used by our tasks
###########################################################################

# Set the master output type to either 'Program', 'StLib', 'DLib'
PROJECT_TYPE      = 'Program'

# Specify the source files to be compiled
SRCFILES          = ['src/**/*.cpp', 'src/**/*.c']

# Specify the windows resource files to be compiled (Win plat only)
WINRES            = ['res/**/*.rc']

# Additional include paths
INCLUDES          = ['deps/*/include']

# List of search paths for static libraries
# Search paths should follow the /path/to/lib/{ARCHITECTURE}/{VARIANT} convention
STLIBPATH         = ['deps/*/lib']

# List of static library names to use without prefix or extension
STLIB             = ['ole32', 'gdi32', 'advapi32', 'user32', 'shell32', 'kernel32']

# List of common defines for all build variants
DEFINES           = ['ASIO_STANDALONE']

# Specify the default build variant if none given in the command line (Debug, Release)
DEF_BUILD_VARIANT = 'Release'

# Specify the default build architecture (x86, x64)
DEF_BUILD_ARCHITECTURE = 'x86'

#=-
# Function helpers bound to the configure context
#=-

# Retrieves the output file basename according to the project type
@conf
def get_output_name(ctx):
    return {
               'Program': APPNAME,
               'StLib': APPNAME.lower(),
               'DLib': APPNAME
           }[PROJECT_TYPE]

# List of defines in the form ['key=value']
@conf
def get_defines(ctx, build_variant):
    defines = []
    common_defines = [
                         'UNICODE',
                         '_UNICODE',
                         '_CRT_SECURE_NO_WARNINGS',
                         'WINVER=0x0600',
                         '_WIN32_WINNT=0x0600'
                     ]

    variant_defines = {
                          'Debug'  : ['_DEBUG'],
                          'Release': ['NDEBUG']
                      }[build_variant]

    defines.extend(common_defines)
    defines.extend(variant_defines)
    defines.extend(DEFINES)
    return defines
   
# Retrieves the compiler flags for the given compiler name and build variant
@conf
def get_compiler_flags(ctx, compiler_name, build_variant):
    # Common flags for all build variants
    base_flags = {
                     'msvc'   : ['/nologo', '/EHsc', '/W4'],
                     'g++'    : ['-Wall', '-Wextra', '-pedantic', '-std=c++11'],
                     'gcc'    : ['-Wall', '-Wextra', '-pedantic'],
                     'clang++': ['-Wall', '-Wextra', '-pedantic', '-std=c++11'],
                     'clang'  : ['-Wall', '-Wextra', '-pedantic']
                 }

    variant_specific_flags = \
    {
        'Release': 
        {
            'msvc'   : ['/MT', '/O2'],
            'g++'    : ['-O2'],
            'gcc'    : ['-O2'],
            'clang++': ['-O2'],
            'clang'  : ['-O2']
        },
        'Debug':
        {
            'msvc'   : ['/MTd', '/Zi', '/Od', '/FS', '/Fd' + ctx.get_output_name()],
            'g++'    : ['-g', '-Og'],
            'gcc'    : ['-g', '-Og'],
            'clang++': ['-g', '-O0'],
            'clang'  : ['-g', '-O0']
        }
    }

    flags = []
    flags.extend(base_flags[compiler_name])
    flags.extend(variant_specific_flags[build_variant][compiler_name])
    return flags

# Same as above for the linker
@conf
def get_linker_flags(ctx, compiler_name, build_variant):
    # Common flags for all build variants
    base_flags = {
                     #'msvc'   : ['/nologo', '/manifest', '/subsystem:windows', '/entry:mainCRTStartup'],
                     #'g++'    : ['-static', '-static-libgcc', '-static-libstdc++', '-Wl,-subsystem,windows'],
                     #'clang++': ['-static', '-static-libgcc', '-static-libstdc++', '-Wl,-subsystem,windows'],
                     'msvc'   : ['/nologo', '/manifest'],
                     'g++'    : ['-static', '-static-libgcc', '-static-libstdc++'],
                     'clang++': ['-static', '-static-libgcc', '-static-libstdc++'],
                 }

    variant_specific_flags = \
    {
        'Release': 
        {
            'msvc'   : ['/incremental:NO'],
            'g++'    : [],
            'clang++': [],
        },
        'Debug':
        {
            'msvc'   : ['/debug', '/pdb:' + ctx.get_output_name()],
            'g++'    : [],
            'clang++': [],
        }
    }

    flags = []
    flags.extend(base_flags[compiler_name])
    flags.extend(variant_specific_flags[build_variant][compiler_name])
    return flags

# Run compiled executable if given the --run flag
def post(ctx):
    if ctx.cmd == 'install':
        if vars(ctx.options).get('run'):
            executable_path = ctx.get_tgen_by_name(ctx.get_output_name()).install_task.get_install_path()
            executable_name = ctx.get_tgen_by_name(ctx.get_output_name()).install_task.inputs[0].name
            executable_fullpath = executable_path + os.sep + executable_name
            print("Executing " + executable_fullpath)
            subprocess.Popen(executable_fullpath)     

#=-
# Hacks
#=-

# Replace execute_build function from BuildContext keeping the same functionality without the logs
from waflib.Build import BuildContext, Utils;
def execute_build_nolog(self):
    self.recurse([self.run_dir])
    self.pre_build()

    self.timer=Utils.Timer()
    try:
        self.compile()
    finally:
        if self.progress_bar == 1 and sys.stderr.isatty():
            c = len(self.returned_tasks) or 1
            m = self.progress_line(c, c, Logs.colors.BLUE,Logs.colors.NORMAL)
            Logs.info(m, extra={'stream': sys.stderr, 'c1': Logs.colors.cursor_off, 'c2': Logs.colors.cursor_on})
    self.post_build()
# Replace the function with ours
# WARNING: THIS MAY CAUSE PROBLEMS BETWEEN DIFFERENT WAF VERSIONS
BuildContext.execute_build = execute_build_nolog

# Add static lib pdb installs
from waflib.Tools import msvc
from waflib.TaskGen import after_method, feature
from waflib.Tools import ccroot
@after_method('apply_link')
@feature('c', 'cxx')
def install_stlib_pdbs(self):
    if self.env.CC_NAME != 'msvc'or not getattr(self, 'link_task', None):
        return

    is_static = isinstance(self.link_task,ccroot.stlink_task)
    if not is_static:
        return

    for f in self.env.LINKFLAGS:
        d = f.lower()
        if d[1:] == 'debug':
            pdbnode = self.link_task.outputs[0].change_ext('.pdb')
            self.link_task.outputs.append(pdbnode)

            if getattr(self, 'install_task', None):
                self.pdb_install_task = self.bld.install_files(self.install_task.dest, pdbnode, env=self.env)
            break

# Replace static link run string to use first output only because by default passes generated pdb too
from waflib import Task
from waflib.Tools import ccroot
ccroot.stlink_task.run = Task.compile_fun_noshell(ccroot.stlink_task.orig_run_str.replace('${TGT}', '${TGT[0].abspath()}'))[0]

#=-
# Common waf tagets
#=-

def init(ctx):
    from waflib.Options import options
    from waflib.Build import BuildContext, CleanContext, InstallContext, UninstallContext
    from waflib.Configure import ConfigurationContext
    for y in (BuildContext, CleanContext, InstallContext, UninstallContext, ConfigurationContext):
        name = y.__name__.replace('Context','').lower()
        class tmp(y):
            cmd = name
            variant = vars(options).get('arch') + vars(options).get('variant').lower().capitalize()

def options(opt):
    opt.add_option('--variant', action='store', default=DEF_BUILD_VARIANT, help='set the variant name')
    opt.add_option('--arch', action='store', default=DEF_BUILD_ARCHITECTURE, help='set the target architecture')
    if PROJECT_TYPE == 'Program':
        opt.add_option('--run', action='store_true', default=False, help='execute program after it is built')
    opt.load(['cxx', 'compiler_cxx', 'c', 'compiler_c'])

def configure(conf):
    # Set the compiler preference order
    from waflib.Tools.compiler_cxx import cxx_compiler
    from waflib.Tools.compiler_c import c_compiler
    cxx_compiler['win32'] = ['clang++', 'msvc', 'g++']
    c_compiler['win32']   = ['clang', 'msvc', 'gcc']

    # Setup variables for all build variants
    for y in 'x86 x64'.split():
        for x in 'Debug Release'.split():
            conf.setenv(y + x)
            print("Configuring {0} enviroment for {1} architecture".format(x, y))

            # Set the msvc compiler architecture to be loaded if choosen compiler is msvc
            conf.env.MSVC_TARGETS = [y]

            # Load the waf tools needed
            tools = ['cxx', 'compiler_cxx', 'c', 'compiler_c']
            if conf.srcnode.ant_glob(WINRES):
                tools.append('winres')
            conf.load(tools)

            # Set the compilation and linkage flags
            conf.env.DEFINES   = conf.get_defines(x)
            conf.env.CXXFLAGS  = conf.get_compiler_flags(conf.env.COMPILER_CXX, x)
            conf.env.CFLAGS    = conf.get_compiler_flags(conf.env.COMPILER_CC, x)
            conf.env.LINKFLAGS = conf.get_linker_flags(conf.env.COMPILER_CXX, x)
            conf.env.STLIB     = STLIB
            conf.env.append_value('INCLUDES', [p.abspath() for p in conf.srcnode.ant_glob(INCLUDES, dir=True, src=False)])
            conf.env.append_value('STLIBPATH', \
                ["{LIBFOLDER}/{ARCH}/{VARIANT}".format(LIBFOLDER=p.abspath(), ARCH=y, VARIANT=x) for p in conf.srcnode.ant_glob(STLIBPATH, dir=True, src=False)])
            # Disable the shared lib boundary flag because it conflicts with static linking flags
            conf.env.SHLIB_MARKER = ''
            # Set the install directories
            conf.env.BINDIR = '{PREFIX}/bin/{ARCH}/{VARIANT}'.format(ARCH=y, VARIANT=x, PREFIX=conf.env.PREFIX)
            conf.env.LIBDIR = '{PREFIX}/lib/{ARCH}/{VARIANT}'.format(ARCH=y, VARIANT=x, PREFIX=conf.env.PREFIX)

    #= Set empty enviroment to default one =#
    # Change working enviroment to option default
    conf.setenv(DEF_BUILD_VARIANT.lower())
    # Save a reference
    defenv = conf.get_env()
    # Change working enviroment to empty one
    conf.setenv('')
    # Set current enviroment to the one in the saved reference above
    conf.set_env(defenv)

def build(bld):
    # Add postinstall action
    bld.add_post_fun(post)

    # Print info message in case no variant was given
    if bld.cmd == 'build':
        current_variant = vars(bld.options).get('variant')
        current_architecture = vars(bld.options).get('arch')
        Logs.info("Building {variant} variant for {arch} architecture".format(variant=current_variant, arch=current_architecture))
        Logs.info("Using env with name {0}".format(bld.variant))

    # Gather all the source files
    source_files = bld.srcnode.ant_glob(SRCFILES)

    # Gather all the win resource files
    resource_files = bld.srcnode.ant_glob(WINRES)
    if resource_files:
        source_files.extend(resource_files)

    # Set task features according to project type
    feat = ['cxx', 'c']
    feat.extend({'Program': ['cxxprogram', 'cprogram'], 'StLib': ['cxxstlib', 'cstlib'], 'DLib': ['cxxshlib', 'cshlib']}[PROJECT_TYPE])

    # Set target name according to project type
    tgt_name = bld.get_output_name()

    # Generate build task
    bld(
        features        =   feat,
        source          =   source_files,
        target          =   tgt_name,
        install_path    =   {'Program': "${BINDIR}", 'StLib': "${LIBDIR}", 'DLib': "${BINDIR}"}[PROJECT_TYPE]
    )

def dist(ctx):
    ctx.base_name = APPNAME.lower() + '_' + VERSION
    ctx.algo      = 'zip'
    ctx.excl      = ['**/.waf-1*', '**/*~', '**/*.pyc', '**/*.swp', '**/.lock-w*', out]

