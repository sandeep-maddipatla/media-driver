/*===================== begin_copyright_notice ==================================

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

======================= end_copyright_notice ==================================*/
//!
//! \file       media_render_copy_xe_hpm.cpp
//! \brief      implementation of xe hardware functions
//! \details    Render functions
//!

#include "media_render_copy_xe_hpm.h"
#include "hal_kerneldll.h"
#include "hal_kerneldll_next.h"
#include "media_common_defs.h"
#include "media_copy.h"
#include "mhw_render.h"
#include "mhw_state_heap.h"
#include "mos_defs_specific.h"
#include "mos_os.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"
#include "renderhal.h"
#include "umKmInc/UmKmDmaPerfTimer.h"
#include "vp_common.h"
#include "vphal.h"
#include "vphal_render_common.h"
#include "vpkrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_xe_hpg.h"
#endif

class MhwInterfaces;
RenderCopy_Xe_Hpm::RenderCopy_Xe_Hpm(PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces):
    RenderCopyState(osInterface, mhwInterfaces)
{
  
}

RenderCopy_Xe_Hpm:: ~RenderCopy_Xe_Hpm()
{
}

MOS_STATUS RenderCopy_Xe_Hpm::CopySurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    VPHAL_GET_SURFACE_INFO  Info;
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));
    m_Source.OsResource = *src;
    m_Source.Format     = Format_Invalid;
    MCPY_CHK_STATUS_RETURN(VpHal_GetSurfaceInfo(
       m_osInterface,
       &Info,
       &m_Source));
    m_Source.rcSrc.right     = m_Source.dwWidth;
    m_Source.rcSrc.bottom    = m_Source.dwHeight;
    m_Source.rcDst.right     = m_Source.dwWidth;
    m_Source.rcDst.bottom    = m_Source.dwHeight;
    m_Source.rcMaxSrc.right  = m_Source.dwWidth;
    m_Source.rcMaxSrc.bottom = m_Source.dwHeight;
    m_Source.ColorSpace      = CSpace_Any;

    m_Target.OsResource = *dst;
    m_Target.Format     = Format_Invalid;
    MCPY_CHK_STATUS_RETURN(VpHal_GetSurfaceInfo(
       m_osInterface,
       &Info,
       &m_Target));
    m_Target.rcSrc.right     = m_Target.dwWidth;
    m_Target.rcSrc.bottom    = m_Target.dwHeight;
    m_Target.rcDst.right     = m_Target.dwWidth;
    m_Target.rcDst.bottom    = m_Target.dwHeight;
    m_Target.rcMaxSrc.right  = m_Target.dwWidth;
    m_Target.rcMaxSrc.bottom = m_Target.dwHeight;
    m_Target.ColorSpace      = CSpace_Any;

    if ((m_Target.Format != Format_RGBP) && (m_Target.Format != Format_NV12) && (m_Target.Format != Format_RGB)
        && (m_Target.Format != Format_P010) && (m_Target.Format != Format_P016) && (m_Target.Format != Format_YUY2)
        && (m_Target.Format != Format_Y210)  && (m_Target.Format != Format_Y216)  && (m_Target.Format != Format_AYUV)
        && (m_Target.Format != Format_Y410)  && (m_Target.Format != Format_Y416)  && (m_Target.Format != Format_A8R8G8B8))
    {
        MCPY_ASSERTMESSAGE("Can't suppport format %d ", m_Target.Format);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MCPY_CHK_STATUS_RETURN(GetCurentKernelID());
    return SubmitCMD();
}

MOS_STATUS RenderCopy_Xe_Hpm::SubmitCMD( )
{
    PRENDERHAL_INTERFACE        pRenderHal;
    PMOS_INTERFACE              pOsInterface;
    MHW_KERNEL_PARAM            MhwKernelParam;
    int32_t                     iKrnAllocation;
    int32_t                     iCurbeOffset;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    PRenderCopy_Xe_Hpm         pRenderCopy = this;
    PMEDIACOPY_RENDER_DATA      pRenderData = &(pRenderCopy->m_RenderData);
    MHW_WALKER_PARAMS           WalkerParams = {0};
    PMHW_WALKER_PARAMS          pWalkerParams = nullptr;
    MHW_GPGPU_WALKER_PARAMS     ComputeWalkerParams = {0};
    PMHW_GPGPU_WALKER_PARAMS    pComputeWalkerParams = nullptr;
    MOS_GPUCTX_CREATOPTIONS     createOption;

    pRenderHal   = pRenderCopy->m_renderHal;
    pOsInterface = pRenderCopy->m_osInterface;
    // no gpucontext will be created if the gpu context has been created before.
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_COMPUTE,
        MOS_GPU_NODE_COMPUTE,
        &createOption));

    // Set GPU Context to Render Engine
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnSetGpuContext(pOsInterface, MOS_GPU_CONTEXT_COMPUTE));

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(pOsInterface);

    // Register the resource of GSH
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnReset(pRenderHal));

    // Register the input resource;
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(
        pOsInterface,
        (PMOS_RESOURCE)&pRenderCopy->m_Source.OsResource,
        true,
        true));

    // Ensure input can be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface,
        &pRenderCopy->m_Source.OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        false);

    // Ensure Output can be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface,
        &pRenderCopy->m_Target.OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        false);


    // Set copy kernel
    pRenderCopy->SetupKernel(m_currKernelId);

    //----------------------------------
    // Allocate and reset media state
    //----------------------------------
     pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_RENDER_COPY);
     MCPY_CHK_NULL_RETURN(pRenderData->pMediaState);

    // Allocate and reset SSH instance
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnAssignSshInstance(pRenderHal));

    // Assign and Reset Binding Table
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnAssignBindingTable(
            pRenderHal,
            &pRenderData->iBindingTable));

    // Setup surface states
    MCPY_CHK_STATUS_RETURN(SetupSurfaceStates())

    // load static data
    MCPY_CHK_STATUS_RETURN(LoadStaticData(
            &iCurbeOffset));

    //----------------------------------
    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    //----------------------------------
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        pRenderData->pKernelParam->Thread_Count,
        pRenderData->iCurbeLength,
        pRenderData->iInlineLength,
        nullptr));

    //----------------------------------
    // Load kernel to GSH
    //----------------------------------
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, &pRenderData->KernelEntry);
    iKrnAllocation = pRenderHal->pfnLoadKernel(
        pRenderHal,
        pRenderData->pKernelParam,
        &MhwKernelParam,
        nullptr);
    if (iKrnAllocation < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    //----------------------------------
    // Allocate Media ID, link to kernel
    //----------------------------------
    pRenderData->iMediaID = pRenderHal->pfnAllocateMediaID(
        pRenderHal,
        iKrnAllocation,
        pRenderData->iBindingTable,
        iCurbeOffset,
        pRenderData->pKernelParam->CURBE_Length << 5,
        0,
        nullptr);
    if (pRenderData->iMediaID < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Set Perf Tag
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(
        pOsInterface,
        pRenderData->PerfTag);

    // Setup Compute Walker
    pWalkerParams = nullptr;
    pComputeWalkerParams = &ComputeWalkerParams;

    RenderCopyComputerWalker(
       &ComputeWalkerParams);

    // Submit all states to render the kernel
    MCPY_CHK_STATUS_RETURN(VpHal_RndrCommonSubmitCommands(
        pRenderHal,
        nullptr,
        pRenderCopy->m_bNullHwRenderCopy,
        pWalkerParams,
        pComputeWalkerParams,
        (VpKernelID)kernelRenderCopy,
        true));

finish:
    return eStatus;
}


MOS_STATUS RenderCopy_Xe_Hpm::SetupKernel(
    int32_t iKDTIndex)
{
    Kdll_CacheEntry             *pCacheEntryTable;                              // Kernel Cache Entry table
    int32_t                     iKUID = 0 ;                                     // Kernel Unique ID
    int32_t                     iInlineLength;                                  // Inline data length
    int32_t                     iTotalRows;                                     // Total number of row in statistics surface
    int32_t                     iTotalColumns;                                  // Total number of columns in statistics surface
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;                   // Return code
    uint32_t                    dwKernelBinSize;
    PMEDIACOPY_RENDER_DATA      pRenderData = &m_RenderData;
    const void                  *pcKernelBin = nullptr;

    MCPY_CHK_NULL_RETURN(pRenderData);
    if (iKDTIndex == KERNEL_CopyKernel_1D_to_2D_NV12)
    {
        iKUID = IDR_VP_CopyKernel_1D_to_2D_NV12_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_2D_NV12)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_2D_NV12_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_1D_NV12)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_1D_NV12_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_1D_to_2D_Planar)
    {
        iKUID = IDR_VP_CopyKernel_1D_to_2D_RGBP_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_2D_Planar)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_2D_RGBP_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_1D_Planar)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_1D_RGBP_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_1D_to_2D_Packed)
    {
        iKUID = IDR_VP_CopyKernel_1D_to_2D_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_2D_Packed)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_2D_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_1D_Packed)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_1D_genx;
    }
    else
    {
        MCPY_ASSERTMESSAGE("Can't find the right kernel.");
        return MOS_STATUS_UNKNOWN;
    }

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    
    pcKernelBin = (const void*)IGVPKRN_XE_HPG;
    dwKernelBinSize = IGVPKRN_XE_HPG_SIZE;
#else
    pcKernelBin = nullptr;
    dwKernelBinSize = 0;
#endif

    if (nullptr == m_pKernelBin)
    {
        m_pKernelBin = MOS_AllocMemory(dwKernelBinSize);
    }

    MCPY_CHK_NULL_RETURN(m_pKernelBin);
    MOS_SecureMemcpy(m_pKernelBin,
                     dwKernelBinSize,
                     pcKernelBin,
                     dwKernelBinSize);

    // Allocate KDLL state (Kernel Dynamic Linking)
    if (nullptr == m_pKernelDllState)
    {
        m_pKernelDllState =  KernelDll_AllocateStates(
                                            m_pKernelBin,
                                            dwKernelBinSize,
                                            nullptr,
                                            0,
                                            nullptr,
                                            nullptr);
    }
    if ( nullptr == m_pKernelDllState)
    {
        MCPY_ASSERTMESSAGE("Failed to allocate KDLL state.");
        if (m_pKernelBin)
        {
            MOS_SafeFreeMemory(m_pKernelBin);
            m_pKernelBin = nullptr;
        }
        return MOS_STATUS_NULL_POINTER;
    }

    pCacheEntryTable =
          m_pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Set the Kernel Parameters
    pRenderData->pKernelParam = (PRENDERHAL_KERNEL_PARAM)&g_rendercopy_KernelParam[iKDTIndex];
    pRenderData->PerfTag = VPHAL_NONE;

    // Set Kernel entry
    pRenderData->KernelEntry.iKUID = iKUID;
    pRenderData->KernelEntry.iKCID = -1;
    pRenderData->KernelEntry.iSize = pCacheEntryTable[iKUID].iSize;
    pRenderData->KernelEntry.pBinary = pCacheEntryTable[iKUID].pBinary;

    return eStatus;
}

//!
 //! \brief    Render copy omputer walker setup
 //! \details  Computer walker setup for render copy
 //! \param    PMHW_WALKER_PARAMS pWalkerParams
 //!           [in/out] Pointer to Walker params
 //! \return   MOS_STATUS
 //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
 //!
 MOS_STATUS RenderCopy_Xe_Hpm::RenderCopyComputerWalker(
 PMHW_GPGPU_WALKER_PARAMS    pWalkerParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    PMEDIACOPY_RENDER_DATA                  pRenderData = &m_RenderData;
    RECT                                    AlignedRect;
    int32_t                                 iBytePerPixelPerPlane = GetBytesPerPixelPerPlane(m_Target.Format);

    MCPY_CHK_NULL_RETURN(pRenderData);

    if ((iBytePerPixelPerPlane < 1) || (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("RenderCopyComputerWalker wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((m_Target.Format == Format_YUY2) || (m_Target.Format == Format_Y210) || (m_Target.Format == Format_Y216)
        || (m_Target.Format == Format_AYUV) || (m_Target.Format == Format_Y410) || (m_Target.Format == Format_Y416)
        || (m_Target.Format == Format_A8R8G8B8))
    {
        if ((m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed) || (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed))
        {
            m_WalkerHeightBlockSize = 32;
        }
        else if (m_currKernelId == KERNEL_CopyKernel_2D_to_2D_Packed)
        {
            m_WalkerHeightBlockSize = 8;
        }
        else
        {
            MCPY_ASSERTMESSAGE("RenderCopyComputerWalker wrong kernel file.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        m_WalkerHeightBlockSize = 8;
    }

    if ((m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed) ||
        (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_NV12) ||
        (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Planar))
    {
        m_WalkerWidthBlockSize = 16;
    }
    else
    {
        m_WalkerWidthBlockSize = 128;
    }
    // Set walker cmd params - Rasterscan
    MOS_ZeroMemory(pWalkerParams, sizeof(*pWalkerParams));


    AlignedRect.left   = 0;
    AlignedRect.top    = 0;
    AlignedRect.right  = (m_Source.dwPitch < m_Target.dwPitch) ? m_Source.dwPitch : m_Target.dwPitch;
    AlignedRect.bottom = (m_Source.dwHeight < m_Target.dwHeight) ? m_Source.dwHeight : m_Target.dwHeight;
    // Calculate aligned output area in order to determine the total # blocks
   // to process in case of non-16x16 aligned target.
    AlignedRect.right += m_WalkerWidthBlockSize - 1;
    AlignedRect.bottom += m_WalkerHeightBlockSize - 1;
    AlignedRect.left -= AlignedRect.left % m_WalkerWidthBlockSize;
    AlignedRect.top -= AlignedRect.top % m_WalkerHeightBlockSize;
    AlignedRect.right -= AlignedRect.right % m_WalkerWidthBlockSize;
    AlignedRect.bottom -= AlignedRect.bottom % m_WalkerHeightBlockSize;

    pWalkerParams->InterfaceDescriptorOffset = pRenderData->iMediaID;

    pWalkerParams->GroupStartingX = (AlignedRect.left / m_WalkerWidthBlockSize);
    pWalkerParams->GroupStartingY = (AlignedRect.top / m_WalkerHeightBlockSize);

    // Set number of blocks
    pRenderData->iBlocksX =
        ((AlignedRect.right - AlignedRect.left) + m_WalkerWidthBlockSize - 1) / m_WalkerWidthBlockSize;
    pRenderData->iBlocksY =
        ((AlignedRect.bottom - AlignedRect.top) + m_WalkerHeightBlockSize -1)/ m_WalkerHeightBlockSize;

    // Set number of blocks, block size is m_WalkerWidthBlockSize x m_WalkerHeightBlockSize.
    pWalkerParams->GroupWidth = pRenderData->iBlocksX;
    pWalkerParams->GroupHeight = pRenderData->iBlocksY; // hight/m_WalkerWidthBlockSize

    pWalkerParams->ThreadWidth = 1;
    pWalkerParams->ThreadHeight = 1;
    pWalkerParams->ThreadDepth = 1;
    pWalkerParams->IndirectDataStartAddress = pRenderData->iCurbeOffset;
    // Indirect Data Length is a multiple of 64 bytes (size of L3 cacheline). Bits [5:0] are zero.
    pWalkerParams->IndirectDataLength = MOS_ALIGN_CEIL(pRenderData->iCurbeLength, 1 << MHW_COMPUTE_INDIRECT_SHIFT);
    pWalkerParams->BindingTableID = pRenderData->iBindingTable;
    MCPY_NORMALMESSAGE("WidthBlockSize %d, HeightBlockSize %d, Widththreads %d, Heightthreads%d",
        m_WalkerWidthBlockSize, m_WalkerHeightBlockSize, pWalkerParams->GroupWidth, pWalkerParams->GroupHeight);

    return eStatus;
}