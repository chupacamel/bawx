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

// Thread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_) && !defined(AFX_THREAD_H__67621B15_8724_4B5D_9343_7667075C89F2__INCLUDED_)
#define AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "system.h" // for HANDLE
#ifdef _LINUX
#include "PlatformInclude.h"
#endif
#include "Event.h"
#include "CriticalSection.h"

class CThread;
class IRunnable
{
public:
  IRunnable(): m_bJobResult(true), m_IsCanceled(false), pParent(NULL) , m_IsSilent(false) , m_allowAbort(true){}
  virtual void Run()=0;
  virtual ~IRunnable() {}
  bool m_bJobResult;
  bool m_IsCanceled;
  CThread *pParent;
  bool m_IsSilent;
  bool m_allowAbort;
};

#ifdef CTHREAD
#undef CTHREAD
#endif

// minimum as mandated by XTL
// Zvisha updated from 0x10000 because of Linux crash.
#define THREAD_MINSTACKSIZE     0x40000
#define THREAD_NORMALSTACKSIZE 0    // system default


class CThread
{
public:
  CThread();
  CThread(IRunnable* pRunnable);
  virtual ~CThread();
  void Create(bool bAutoDelete = false, unsigned stacksize = THREAD_NORMALSTACKSIZE);
  bool WaitForThreadExit(unsigned int milliseconds);
  DWORD WaitForSingleObject(HANDLE hHandle, unsigned int milliseconds) const;
  DWORD WaitForMultipleObjects(DWORD nCount, HANDLE *lpHandles, BOOL bWaitAll, unsigned int milliseconds) const;
  void Sleep(unsigned int milliseconds);
  bool SetPriority(const int iPriority);
  void SetName( LPCTSTR szThreadName );
  HANDLE ThreadHandle();
  operator HANDLE();
  operator HANDLE() const;
  bool IsAutoDelete() const;
  void SetAutoDelete(bool bAutoDelete);
  virtual void StopThread(bool bWait = true);
  float GetRelativeUsage();  // returns the relative cpu usage of this thread since last call
  bool IsCurrentThread() const;

  static bool Equals(const ThreadIdentifier tid1, const ThreadIdentifier tid2);
  static bool IsCurrentThread(const ThreadIdentifier tid);
  static ThreadIdentifier GetCurrentThreadId();
  inline bool IsRunning() { return !m_bStop; }
  
  void Detach(bool bAutoDelete = true);
  
protected:
  virtual void OnStartup(){};
  virtual void OnExit(){};
  virtual void OnException(){} // signal termination handler
  virtual void Process(); 

#ifdef _LINUX
  static void term_handler (int signum);
#endif

  volatile bool m_bStop;
  volatile bool m_bDone;
  HANDLE m_ThreadHandle;

private:
  ThreadIdentifier ThreadId() const;
  bool m_bAutoDelete;
  HANDLE m_StopEvent;
  unsigned m_ThreadId; // This value is unreliable on platforms using pthreads
                       // Use m_ThreadHandle->m_hThread instead
  IRunnable* m_pRunnable;

  unsigned __int64 m_iLastUsage;
  unsigned __int64 m_iLastTime;
  float m_fLastUsage;

  CCriticalSection m_lock; 
private:
#ifndef _WIN32
  static int staticThread(void* data);
#else
  static DWORD WINAPI staticThread(LPVOID* data);
#endif
};

#endif // !defined(AFX_THREAD_H__ACFB7357_B961_4AC1_9FB2_779526219817__INCLUDED_)
