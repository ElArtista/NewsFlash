import os
import glob
import logging
import inspect

def CSharpSolutionFile(filepath):
    logger = logging.getLogger( __name__ )
    logger.info('Got filepath value "' + filepath + '" in CSharpSolutionFile function')
    for root, dirs, files in os.walk(filepath):
        for file in files:
            print root + "\\" + file
            if len(glob.glob(root + "\\" + '*.sln')) == 1:
                return root + "\\" + file


def FlagsForFile(filename, **kwargs):
    data = kwargs['client_data']
    filetype = data['&filetype']

    flags = [
                '-Wall',
                '-Wextra'
            ]
    
    lang_specific_flags = \
    {
        'cpp': ['-xc++', '-std=c++11'],
        'c'  : ['-xc']
    }

    flags.extend(lang_specific_flags[filetype])

    includes = glob.glob(os.path.dirname(os.path.realpath(inspect.getfile(inspect.currentframe()))) + '/deps/*/include')
    defines  = ['ASIO_STANDALONE', '_WIN32_WINNT=0x0600', 'WINVER=0x0600']

    for i in includes:
        flags.append('-I' + i)
    for d in defines:
        flags.append('-D' + d)

    return {
        'flags': flags,
        'do_cache': True
    }
