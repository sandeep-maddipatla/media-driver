# Copyright (c) 2020, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

if(${AV1_Decode_Supported} STREQUAL "yes")
set(TMP_SOURCES_
    ${TMP_SOURCES_}
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_single_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_picture_packet_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_slice_packet_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_front_end_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_back_end_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_tile_packet_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_huc_prob_update_packet_m12.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_downsampling_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_back_end_xe_m_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_front_end_xe_m_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_single_xe_m_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_xe_m_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_picture_packet_xe_m_base.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_slice_packet_xe_m_base.cpp
)

set(TMP_HEADERS_
    ${TMP_HEADERS_}
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_single_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_picture_packet_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_slice_packet_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_front_end_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_back_end_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_tile_packet_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_huc_prob_update_packet_m12.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_downsampling_packet.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_back_end_xe_m_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_front_end_xe_m_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_single_xe_m_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_packet_xe_m_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_picture_packet_xe_m_base.h
    ${CMAKE_CURRENT_LIST_DIR}/decode_vp9_slice_packet_xe_m_base.h
)
endif()

media_add_curr_to_include_path()
