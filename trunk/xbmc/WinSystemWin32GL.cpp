/*!
\file Surface.h
\brief
*/



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
#include "WinSystemWin32GL.h"
#include "WIN32Util.h"

#if defined(HAS_GL)

#ifdef HAS_GLEW
#include <GL/glew.h>
#endif

#pragma comment (lib,"opengl32.lib")
#pragma comment (lib,"glu32.lib")
#ifdef HAS_GLEW
#pragma comment (lib,"../../xbmc/lib/libglew/glew32.lib") 
#endif

CWinSystemWin32GL g_Windowing;


CWinSystemWin32GL::CWinSystemWin32GL()
{
  m_wglSwapIntervalEXT = NULL;
}

CWinSystemWin32GL::~CWinSystemWin32GL()
{

}

bool CWinSystemWin32GL::InitRenderSystem()
{
  if(m_hWnd == NULL || m_hDC == NULL)
    return false;

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.cStencilBits = 8;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;

  // if these fail, wglCreateContext will also quietly fail
  int format;
  if ((format = ChoosePixelFormat(m_hDC, &pfd)) != 0)
    SetPixelFormat(m_hDC, format, &pfd);
  else
    return false;

  m_hglrc = wglCreateContext(m_hDC);
  if (!m_hglrc)
    return false;

  wglMakeCurrent(m_hDC, m_hglrc);

  if(strstr((const char*)glGetString(GL_EXTENSIONS), "WGL_EXT_swap_control"))
    m_wglSwapIntervalEXT = (BOOL (APIENTRY *)(int))wglGetProcAddress( "wglSwapIntervalEXT" );

  if(!CRenderSystemGL::InitRenderSystem())
    return false;

  CWIN32Util::CheckGLVersion();

  return true;
}

void CWinSystemWin32GL::SetVSyncImpl(bool enable)
{
  if(m_wglSwapIntervalEXT)
    m_wglSwapIntervalEXT(enable ? 1 : 0);

  // elis test
  //m_wglSwapIntervalEXT(0);
}

bool CWinSystemWin32GL::PresentRenderImpl()
{
  if(!m_bWindowCreated || !m_bRenderCreated)
    return false;

  SwapBuffers(m_hDC);

  return true;
}

bool CWinSystemWin32GL::ResizeWindow(int newWidth, int newHeight, int newLeft, int newTop)
{
  CWinSystemWin32::ResizeWindow(newWidth, newHeight, newLeft, newTop);
  CRenderSystemGL::ResetRenderSystem(newWidth, newHeight, false, 0);  

  return true;
}

bool CWinSystemWin32GL::SetFullScreen(bool fullScreen, RESOLUTION_INFO& res, bool blankOtherDisplays)
{
  CWinSystemWin32::SetFullScreen(fullScreen, res, blankOtherDisplays);
  CRenderSystemGL::ResetRenderSystem(res.iWidth, res.iHeight, false, 0);  

  return true;
}

#endif // HAS_GL

