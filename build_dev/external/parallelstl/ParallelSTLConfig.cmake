##===-- ParallelSTLConfig.cmake.in ----------------------------------------===##
#
# Copyright (C) 2018-2019 Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# This file incorporates work covered by the following copyright and permission
# notice:
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
#
##===----------------------------------------------------------------------===##

include(CMakeFindDependencyMacro)

set(PARALLELSTL_BACKEND "tbb")

if(PARALLELSTL_BACKEND STREQUAL "tbb")
    find_dependency(TBB 2018 REQUIRED tbb)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/ParallelSTLTargets.cmake")
