Release Notes {#mainpage}
============

[TOC]

# Introduction {#Introduction}

Welcome to Equalizer, the standard middleware to create and deploy
parallel, scalable OpenGL applications. This release introduces major
new features, most notably integrated head tracking using VRPN or OpenCV.

Equalizer 1.8 is a feature release extending the 1.0 API, distilling
over several years of development and decades of experience into a
feature-rich, high-performance and mature parallel rendering
framework. It is intended for all application developers creating
parallel, interactive OpenGL applications. Equalizer 1.8 can be
retrieved by downloading the [source code](http://www.equalizergraphics.com/downloads/Equalizer-1.8.0.tar.gz") or one of the [precompiled packages](http://www.equalizergraphics.com/downloads/major.html).

## Features {#Features}

Equalizer provides the following major features to facilitate the development and deployment of scalable OpenGL applications. A [detailed feature list](http://www.equalizergraphics.com/features.html) can be found on the [Equalizer website](http://www.equalizergraphics.com).

* Runtime Configurability: An Equalizer application is configured
  automatically or manually at runtime and can be deployed on laptops,
  multi-GPU workstations and large-scale visualization clusters without
  recompilation.
* Runtime Scalability: An Equalizer application can benefit from
  multiple graphics cards, processors and computers to scale rendering
  performance, visual quality and display size.
* Distributed Execution: Equalizer applications can be written to
  support cluster-based execution. Equalizer uses the
  [Collage network library](http://www.libcollage.net), a cross-platform
  C++ library for building heterogenous, distributed applications.
* Support for Stereo and Immersive Environments: Equalizer supports
  stereo rendering head tracking, head-mounted displays and other
  advanced features for immersive Virtual Reality installations.

# New in this release {#New}

Equalizer 1.8 contains the following features, enhancements, bug fixes
and documentation changes over the Equalizer 1.7 release:

## New Features {#NewFeatures}

* Implemented Qt window system for onscreen windows. Issue
  [21](https://github.com/Eyescale/Equalizer/issues/21) partially done.
* DisplayCluster streaming can be enabled with automatic configuration using new
  global view attributes: EQ_VIEW_SATTR_DISPLAYCLUSTER and
  EQ_VIEW_SATTR_PIXELSTREAM_NAME.

## Enhancements {#Enhancements}

* New EqGLLibraries.cmake script for simpler OpenGL configuration in
  downstream projects
* Compression is enabled for DisplayCluster streaming
* DisplayCluster streaming is now asynchronous
* The application now doesn't abort if the init fails
* Enable FBO rendering using QGLWidget

## Optimizations {#Optimizations}

* DisplayCluster streaming is optimized: no alpha, and reuse FBO texture
  if available

## Examples {#Examples}

* A DisplayCluster example config was added

## Documentation {#Documentation}

The following documentation has been added or substantially improved
since the last release:

* Improved init documentation

## Bug Fixes {#Fixes}

Equalizer 1.8 includes various bugfixes over the 1.7 release, including
the following:

* [281](https://github.com/Eyescale/Equalizer/pull/281): Pan in Sequel with
  the right button as in eqPly
* [304](https://github.com/Eyescale/Equalizer/pull/304): Configs with no
  running channels should exit or fail
* [368](https://github.com/Eyescale/Equalizer/pull/368): Fix mixup of
  horizontal and vertical wheel events
* [370](https://github.com/Eyescale/Equalizer/pull/370): VRPN head tracking
  changes not visible when idle
* Universal AGL build is now fixed
* Fix crash while streaming to non-running DisplayCluster application
* Fix crash in eqPly render clients with Qt

## Known Bugs {#Bugs}

The following bugs were known at release time. Please file a
[Bug Report](https://github.com/Eyescale/Equalizer/issues) if you find
any other issue with this release.

* [17](https://github.com/Eyescale/Equalizer/issues/17): AGL: Window
  close does not work
* [18](https://github.com/Eyescale/Equalizer/issues/18): zoom: depth
  readback does not work
* [19](https://github.com/Eyescale/Equalizer/issues/19): zoom readback with FBO
* [77](https://github.com/Eyescale/Equalizer/issues/77):
  7-window.DB.PIXEL.eqc broken
* [78](https://github.com/Eyescale/Equalizer/issues/78): AGL: assertion
  on interaction with multiple GPUs
* [167](https://github.com/Eyescale/Equalizer/issues/167): HWLOC:
  Segmentation Fault with empty auto thread affinity mask
* [226](https://github.com/Eyescale/Equalizer/issues/226): Crash with
  layout switch on multi GPU system
* [232](https://github.com/Eyescale/Equalizer/issues/232): ROI with
  monitor equalizer results in wrong compositing
* [286](https://github.com/Eyescale/Equalizer/issues/286): Certain combinations
  of 2D and subpixels compounds don't work in eqPly
* [298](https://github.com/Eyescale/Equalizer/issues/298): RDMA connection to
  server from rendering client crashes

# About {#About}

Equalizer is a cross-platform framework, designed to run on any modern
operating system, including all Unix variants and the Windows operating
system. Equalizer requires at least [OpenGL 1.1](http://www.opengl.org),
but uses newer OpenGL features when available. Equalizer uses CMake and
[Buildyard](https://github.com/Eyescale/Buildyard) to create a
platform-specific build environment. The following platforms and build
environments are tested for version 1.8:

* Linux: Ubuntu 14.04, RHEL 6.5 (Makefile, i386, x64)
* Windows: 7 (Visual Studio 2008, i386, x64)
* Mac OS X: 10.8 (Makefile, XCode, i386, x64)

The Equalizer Programming and User Guide covers the basics of Collage
programming. The API documentation can be found on
[github](http://eyescale.github.com/).

As with any open source project, the available source code, in
particular the shipped examples provide a reference for developing or
porting applications.

Technical questions can be posted to the eq-dev Mailing List, or
directly to info@equalizergraphics.com.

Commercial support, custom software development and porting services are
available from [Eyescale](http://www.eyescale.ch). Please contact
[info@eyescale.ch](mailto:info@eyescale.ch?subject=Collage%20support)
for further information.

# Errata
