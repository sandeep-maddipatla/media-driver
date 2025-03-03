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
//! \file     media_interfaces_mcpy_next.h
//! \brief    Gen-specific factory creation of the mcpy interfaces
//!

#ifndef __MEDIA_INTERFACES_MCPY_NEXT_H__
#define __MEDIA_INTERFACES_MCPY_NEXT_H__
#include "media_factory.h"

// forward declaration
class MediaCopyBaseState;
class MhwInterfacesNext;
//!
//! \class    McpyDevice
//! \brief    MCPY device
//!
class McpyDeviceNext
{
public:
    virtual ~McpyDeviceNext() {}

    MediaCopyBaseState *m_mcpyDevice = nullptr; //!< Media memory copy device

    //!
    //! \brief    Create Media memory copy instance
    //! \details  Entry point to create Gen specific media memory compression instance
    //! \param    [in] osDriverContext
    //!           OS context used by to initialize the MOS_INTERFACE, includes information necessary for resource management and interfacing with KMD in general
    //!
    //! \return   Pointer to Gen specific  mediacopy instance if
    //!           successful, otherwise return nullptr
    //!
    static void* CreateFactory(
        PMOS_CONTEXT osDriverContext);

    //!
    //! \brief    Initializes platform specific Media copy states
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] mhwInterfaces
    //!           HW interfaces to be used by media copy
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        PMOS_INTERFACE    osInterface,
        MhwInterfacesNext *mhwInterfaces) = 0;

    //!
    //! \brief    Creat platform specific media copy HW interface
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MhwInterfaces if succeeded.
    //!
    virtual MhwInterfacesNext* CreateMhwInterface(
        PMOS_INTERFACE osInterface);
    MEDIA_CLASS_DEFINE_END(McpyDeviceNext)
};

extern template class MediaFactory<uint32_t, McpyDeviceNext>;


#endif // __MEDIA_INTERFACES_MCPY_NEXT_H__

