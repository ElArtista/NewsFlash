![ProjectBanner][banner]
# NewsFlash

NewsFlash is a generic Notification System for Windows, written in Native Win32 API.

## Table of Contents <a name="toc"/>
 * [Table Of Contents](#toc)
 * [Team Members](#team)
 * [Requirements](#requirements)
 * [Downloads](#downloads)
 * [Usage](#usage)
 * [Building](#building)
 * [Changelog](#changelog)
 * [Contributing](#contributing)
 * [Licensing](#licensing)

## Team Members <a name="team"/>
 * "Agorgianitis Loukas" <agorglouk@gmail.com>

## Requirements <a name="requirements"/>
 * For running the application, Windows 7 or newer are required
 * For building the application, any known Windows compiler (MSVC, MinGW, CLang) and Python 2.7 are required

## Downloads <a name="downloads"/>
 * TODO: Provide prebuilt binaries

## Usage <a name="usage"/>
 * TODO: Describe command line arguments and json API

## Building <a name="building"/>
 1. Clone the project and cd to the cloned directory.
 2. Run:
    ```
    python waf distclean configure --check-c-compiler=<CC> --check-cxx-compiler=<CXX>
    ```
    where CC and CXX combination can either be {msvc,msvc} | {gcc, g++} | {clang, clang++} and after that run:
    ```
    python waf clean build install --variant=<VARIANT> --arch=<ARCH>
    ```
    where VARIANT can be either Release|Debug and ARCH can be either x86|x64.
 3. Built binaries will reside in the ```bin\<ARCH>\<VARIANT>``` directory.

## Change Log <a name="changelog"/>
 * TODO: Track Major release history after first release

## Contributing <a name="contributing"/>
 * For bug fixes, any well checked pull requests are welcome

## Licensing <a name="licensing"/>
Read [LICENSE](LICENSE.md)  

Copyright (C) 2015 Agorgianitis Loukas <agorglouk@gmail.com>  
All rights reserved.

[banner]: doc/assets/NewsFlash.png
