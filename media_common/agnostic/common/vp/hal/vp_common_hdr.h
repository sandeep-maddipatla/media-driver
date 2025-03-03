/*
* Copyright (c) 2022, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     vp_common_hdr.h
//! \brief    vphal HDR interface clarification
//! \details  vphal HDR interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VP_COMMON_HDR_H__
#define __VP_COMMON_HDR_H__

#define VPHAL_HDR_MODE_3DLUT_MASK                   0x10
#define VPHAL_HDR_MODE_VEBOX_3DLUT_MASK             0x20
#define VPHAL_HDR_MODE_VEBOX_3DLUT33_MASK           0x30
#define VPHAL_HDR_MODE_VEBOX_1DLUT_MASK             0x40
#define VPHAL_HDR_MODE_VEBOX_1DLUT_3DLUT_MASK       0x50

#define HDR_DEFAULT_MAXCLL    4000
#define HDR_DEFAULT_MAXFALL   400

//!
//! \brief HDR mode enum
//!
typedef enum _VPHAL_HDR_MODE
{
    VPHAL_HDR_MODE_NONE = 0,
    VPHAL_HDR_MODE_TONE_MAPPING,
    VPHAL_HDR_MODE_INVERSE_TONE_MAPPING,
    VPHAL_HDR_MODE_H2H,
    VPHAL_HDR_MODE_S2S,
    VPHAL_HDR_MODE_BT1886_DEGAMMA,
    VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE,
    VPHAL_HDR_MODE_H2H_AUTO_MODE,
    VPHAL_HDR_MODE_TONE_MAPPING_3DLUT               = VPHAL_HDR_MODE_TONE_MAPPING | VPHAL_HDR_MODE_3DLUT_MASK,
    VPHAL_HDR_MODE_INVERSE_TONE_MAPPING_3DLUT       = VPHAL_HDR_MODE_INVERSE_TONE_MAPPING | VPHAL_HDR_MODE_3DLUT_MASK,
    VPHAL_HDR_MODE_H2H_3DLUT                        = VPHAL_HDR_MODE_H2H | VPHAL_HDR_MODE_3DLUT_MASK,
    VPHAL_HDR_MODE_BT1886_DEGAMMA_3DLUT             = VPHAL_HDR_MODE_BT1886_DEGAMMA | VPHAL_HDR_MODE_3DLUT_MASK,
    VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE_3DLUT     = VPHAL_HDR_MODE_TONE_MAPPING_AUTO_MODE | VPHAL_HDR_MODE_3DLUT_MASK,
    VPHAL_HDR_MODE_H2H_VEBOX_3DLUT                  = VPHAL_HDR_MODE_H2H | VPHAL_HDR_MODE_VEBOX_3DLUT_MASK,
    VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_3DLUT         = VPHAL_HDR_MODE_TONE_MAPPING | VPHAL_HDR_MODE_VEBOX_3DLUT_MASK,
    VPHAL_HDR_MODE_H2H_VEBOX_3DLUT33                = VPHAL_HDR_MODE_H2H | VPHAL_HDR_MODE_VEBOX_3DLUT33_MASK,
    VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_3DLUT33       = VPHAL_HDR_MODE_TONE_MAPPING | VPHAL_HDR_MODE_VEBOX_3DLUT33_MASK,
    VPHAL_HDR_MODE_H2H_VEBOX_1DLUT                  = VPHAL_HDR_MODE_H2H | VPHAL_HDR_MODE_VEBOX_1DLUT_MASK,
    VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_1DLUT         = VPHAL_HDR_MODE_TONE_MAPPING | VPHAL_HDR_MODE_VEBOX_1DLUT_MASK,
    VPHAL_HDR_MODE_TONE_MAPPING_VEBOX_1DLUT_3DLUT   = VPHAL_HDR_MODE_VEBOX_1DLUT_3DLUT_MASK,
} VPHAL_HDR_MODE;

//!
//! \brief HDR LUT mode enum
//!
typedef enum _VPHAL_HDR_LUT_MODE
{
    VPHAL_HDR_LUT_MODE_NONE,
    VPHAL_HDR_LUT_MODE_2D,
    VPHAL_HDR_LUT_MODE_3D,
    VPHAL_HDR_LUT_MODE_NUM
} VPHAL_HDR_LUT_MODE;
C_ASSERT(VPHAL_HDR_LUT_MODE_NUM == 3);

//!
//! Structure VPHAL_EOTF_TYPE
//! \brief Electronic-Optimal Transfer Function type
//!
typedef enum _VPHAL_HDR_EOTF_TYPE
{
    VPHAL_HDR_EOTF_INVALID = -1,
    VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR = 0,
    VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_HDR,
    VPHAL_HDR_EOTF_SMPTE_ST2084,
    VPHAL_HDR_EOTF_BT1886,
    VPHAL_HDR_EOTF_FUTURE_EOTF
} VPHAL_HDR_EOTF_TYPE;


//!
//! Structure VPHAL_HDR_PARAMS
//! \brief High Dynamic Range parameters
//!
typedef struct _VPHAL_HDR_PARAMS
{
    VPHAL_HDR_EOTF_TYPE EOTF                 = VPHAL_HDR_EOTF_INVALID;    //!< Electronic-Optimal Transfer Function
    uint16_t display_primaries_x[3]          = {0};                       //!< Display Primaries X chromaticity coordinates
    uint16_t display_primaries_y[3]          = {0};                       //!< Display Primaries Y chromaticity coordinates
    uint16_t white_point_x                   = 0;                         //!< X Chromaticity coordinate of White Point
    uint16_t white_point_y                   = 0;                         //!< Y Chromaticity coordinate of White Point
    uint16_t max_display_mastering_luminance = 0;                         //!< The nominal maximum display luminance of the mastering display
    uint16_t min_display_mastering_luminance = 0;                         //!< The nominal minimum display luminance of the mastering display
    uint16_t MaxCLL                          = 0;                         //!< Max Content Light Level
    uint16_t MaxFALL                         = 0;                         //!< Max Frame Average Light Level
    bool     bAutoMode                       = false;                     //!< Hdr auto mode.
    bool     bPathKernel                     = false;                     //!< Hdr path config to use kernel
} VPHAL_HDR_PARAMS, *PVPHAL_HDR_PARAMS;

#endif  // __VP_COMMON_HDR_H__
