/*
*      Copyright (C) 2005-2008 Team XBMC
*      http://www.xbmc.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with XBMC; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*  http://www.gnu.org/copyleft/gpl.html
*
*/

#ifndef WINDOWING_FACTORY_H
#define WINDOWING_FACTORY_H

#if defined(_WIN32) && defined(HAS_GL)
#include "WinSystemWin32GL.h"
#endif

#if defined(_WIN32) && defined(HAS_DX)
#include "WinSystemWin32DX.h"
#endif

#if defined(__APPLE__)
#include "WinSystemOSXGL.h"
extern CWinSystemOSXGL g_Windowing;

#elif defined(HAS_GLX)
#include "WinSystemX11GL.h"
extern CWinSystemX11GL g_Windowing;

#elif defined(HAS_GDL)
#include "WinSystemGDLGLES.h"
extern CWinSystemGDLGLES g_Windowing;

#elif defined(HAS_EGL) && defined(HAS_GLES)
#include "WinSystemEGLGLES.h"
extern CWinSystemEGLGLES g_Windowing;
#endif

#endif // WINDOWING_FACTORY_H

