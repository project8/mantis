=====================
Mantis DAQ in WIndows
=====================

Specifications
==============
* Software: Mantis branch Monarch3
* DAQ: NI-5122; using the NI_Scope driver


Required Software
=================

* Visual Studio
  * Installed version Visual Studio Community 2013 Update 4
* NI Drivers
  * Installed version: August 2014 drivers, Part 1 and Part 2; NI Scope 14.0
  * Select “Modular Instruments” -> “NI Scope" then Under “NI-Scope”, select "Visual Basic Support"
* Cygwin (already installed in Irene)
  * This will give you ssh, might be useful
  * If not installed, you need to make sure you have the following packages: OpenSSH, make, gcc, g++, nano/emacs/vim (or your favorite editor), patch
* GitHub for Windows
  * https://github-windows.s3.amazonaws.com/GitHubSetup.exe
* CMake for Windows
  * http://www.cmake.org/files/v3.1/cmake-3.1.1-win32-x86.exe
* Protobuf
  * https://code.google.com/p/protobuf/downloads/detail?name=protobuf-2.5.0.zip&can=2&q=
* HDF5 for Windows
  * http://www.hdfgroup.org/ftp/HDF5/current/bin/windows/extra/hdf5-1.8.14-win32-vs2013-shared.zip
  * Install it in a location without spaces in the path - this will make it easier to configure it in Visual Studio later
  * e.g. use C:/Local/, not C:/Program Files (x86)/
* Boost
  * http://sourceforge.net/projects/boost/files/boost/1.57.0/boost_1_57_0.zip/download
  * Install it in a location without spaces in the path - this will make it easier to configure it in Visual Studio later
  * e.g. use C:/Local/, not C:/Program Files (x86)/
* rabbitmq-c
  * https://github.com/alanxz/rabbitmq-c
  * Cloned from github, created VS project with CMake, then built with VS
  * Disabled SSL support because of linking errors (despite the SSL libraries definitely being included in the "Additional Dependencies" in the VS project configuration)
* GnuWin32
  * http://sourceforge.net/projects/getgnuwin32/files/getgnuwin32/0.6.30/GetGnuWin32-0.6.3.exe/download
  * This contains unistd.h
* Doxygen
  * ftp://ftp.stack.nl/pub/users/dimitri/doxygen-1.8.9.1-setup.exe


Useful documentation
====================

* http://zone.ni.com/reference/en-XX/help/370592W-01/digitizers/creating_an_application_with_visual_c/

Help file:
C:\Program Files (x86)\IVI Foundation\IVI\Drivers\niScope\Documentation\English

Examples:
C:\Users\Public\Documents\National Instruments\NI-SCOPE\examples\VisualBasic

https://cognitivewaves.wordpress.com/cmake-and-visual-studio/

Protobuf Installation
https://github.com/google/protobuf/tree/master/vsprojects
https://code.google.com/p/protobuf/issues/detail?id=531

Install Protobuf
================

* Download the latest release version of protobuf 2 from code.google.com
  * Do not download the latest version from GitHub, nor v3
* Apply the patch vs2013.patch
  * Alternatively, edit ./protobuf-2.5.0/src/google/protobuf/stubs/common.h, add the line `` #include <algorithm>``
* Open the file protobuf\vsprojects\protobuf.sln -> it will start VS2013
  * VS2013 will ask if you want to update the files in the protobuf project - say yes
* In Visual Studio 2013
  * Open BUILD -> Configuration Manager
  * Select “Release” in “Active Configuration Solution"
  * Now we need to add a compiler flag. For each of the projects (with the exception gtest and test_main): “libprotobuf”, “libprotobuf-lite”, etc...

      * Click on PROJECT -> Properties
      * Select "Configuration Properties" -> "C/C++" -> "Command Line"
      * In the field “Additional Options”, add /FS
      * Make sure you do that for all the projects

  * Click on BUILD -> Build Solution

* Test the build

  * Open PowerShell, go to the folder where you built protobuf:
  * .\protobuf-2.5.0\vsprojects\Release\
  * run tests.exe, run lite-test.exe

* Go to .\protobuf-2.5.0\vsprojects\ , run extract_includes.bat
* Create a folder to install protobuf  (i.e. ~/Documents/protobuf)
* Copy the headers and build files to the right place:

  * ``cp -r ~/Documents/protobuf-2.5.0/vsprojects/include ~/Documents/protobuf/``
  * ``cp ~/Documents/protobuf-2.5.0/vsprojects/Release/protoc.exe ~/Documents/protobuf/bin/``
  * ``cp ~/Documents/protobuf-2.5.0/vsprojects/Release/*.lib ~/Documents/protobuf/lib/``

Note that protobuf only had configurations for 32-bits (win32), not for 64-bits.  It’s possible that it would work with 64-bits too, but I decided not to try it.  Let’s configure everything as 32-bits for the rest of this.

Install other software:
======================:
* HDF5

  * Install it in a location without spaces in the path - this will make it easier to configure it in Visual Studio later
  * e.g. use C:/Local/, not C:/Program Files (x86)/

* Boost

  * Install it in a location without spaces in the path - this will make it easier to configure it in Visual Studio later
  * e.g. use C:/Local/, not C:/Program Files (x86)/

* DOXYGEN
* GnuWin32


Install Mantis
==============

The idea is to use cmake to generate the Visual Studio Project Files, then open the project files in VS, and compile Mantis in VS.

* First, use Git to download the Mantis source
  * Make sure mantis is on the monarch3 branch
  * Make sure the folder Monarch is on the hdf5 branch

* Open CMAKE-GUI
  * Select the source (the Mantis folder)
  * Select a build folder (I used ~/Documents/build/)
  * Click configure
  * Select Visual Studio 12.0 2013  (that’s the 32bit version)
  * You will need to manually configure the cmake options for:

    * HDF5

      * HDF5_CXX_LIBRARY = C:/Local/HDF_Group/HDF5/1.8.14/lib/
      * HDF5_CXX_INCLUDE_DIR = C:/Local/HDF_Group/HDF5/1.8.14/include/
      * HDF5_DIR = C:/Local/HDF_Group/HDF5/1.8.14/cmake/hdf5
      * Don’t bother with the other fields, they won’t be used in Visual Studio
        (We intend to change that, but haven’t had the time yet)
    * PROTOBUF
      * Fill up all fields
   
  * Turn off warnings in the Options menu -> select “Suppress Dev Warnings (-Wno-dev)"
  * Click configure
  * Click Generate

* Open the build folder, double-click the mantis.sln file
  * This will open the file in Visual Basic 2013
  * The .sln extension means “Solution”. A “solution” is made up of several “projects”, like MantisClient, Monarch3, lib thorax, etc… 
* We will compile Mantis in Debug mode, for Win32
  * Windows 64-bits can run executables compiled in Win32 or Win64, so compiling in Win32 will make it more compatible;  besides, it’s usually easier to find libraries for Win32.  For example, Protobuf is only available for Win32.
* Compile libthorax by parts
  * Select the thorax project on the left pane, right-click on it, click on Build
  * Do same for thorax_tests and thorax_cpp_tests
  * Open a PowerShell, go to the build folder:

    * cd c:\User\me\Documents\build\Monarch\libthorax\Debug
    * run the test executables: thorax_tests.exe and thorax_cpp_tests.exe

* Compile Monarch by parts

  * Select Monarch3 on the left pane, right click, click on Build
  * Debug\Monarch3.lib will be built

* Select MantisProto on the left pane, right click, click on Build
* Select atomicboost on the left pane, right click, click on Properties
  * Compiling atomicboost on Windows seems to need more than just the atomic boost included in the GitHub distribution - it seems to need the rest of boost.
  * Select "VC++ Directories"
  * In the “Include Directories” field, add the path to the boost main folder (e.g.: C:\Local\boost_1_57_0)
  * Again, select atomicboost on the left pane, right click, now click on Build
* Select MantisCommon on the left pane, right click, click on Properties
  * Select "VC++ Directories"
  * In the “Include Directories” field, add the path to the boost main folder, to the HDF5 include folder, to the pthread include folder, and to the gnuwin32 include folder, e.g.:

    * C:\Local\boost_1_57_0
    * C:/Local/HDF_Group/HDF5/1.8.14/include/
    * C:\Local\pthreads-w32-2-9-1\include
    * C:\Local\gnuwin32\include

* Select MantisCommon on the left pane, right click, click on Build

  * **You will see the compilation errors here - now you just need to fix all that!**
  * Compilation errors are related to the missing ``_netinet/in.h_`` in Windows

