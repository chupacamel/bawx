/*
 *      Copyright (C) 2005-2009 Team XBMC
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

#include "system.h"
#pragma once

#ifdef HAS_FILESYSTEM_HTSP

#include "IDirectory.h"
#include "Thread.h"
#include "utils/CriticalSection.h"
#include "utils/Event.h"
#include "URL.h"
#include "HTSPSession.h"
#include "boost/shared_ptr.hpp"

class CFileItem; typedef boost::shared_ptr<CFileItem> CFileItemPtr;

namespace HTSP
{
  class CHTSPDirectorySession 
      : public CThread
  {
    public: 
      bool                    GetEvent(SEvent& event, uint32_t id);
      SChannels               GetChannels();
      SChannels               GetChannels(int tag);
      SChannels               GetChannels(STag &tag);
      STags                   GetTags();
      htsmsg_t*               ReadResult(htsmsg_t* m);


      static CHTSPDirectorySession* Acquire(const CURI& url);
      static void                   Release(CHTSPDirectorySession* &session);
      static void                   CheckIdle(DWORD idle = 60000);

    protected:
       CHTSPDirectorySession();
      ~CHTSPDirectorySession();

      bool   Open(const CURI& url);
      void   Close();

    private:
      virtual void Process();
      CHTSPSession            m_session;
      SChannels               m_channels;
      STags                   m_tags;
      SEvents                 m_events;
      CCriticalSection        m_section;
      CEvent                  m_started;

      struct SMessage
      {
        CEvent   * event;
        htsmsg_t * msg;
      };
      typedef std::map<int, SMessage> SMessages;

      SMessages m_queue;
  };
}

namespace DIRECTORY
{

  class CHTSPDirectory : public IDirectory
  {
    public:
      CHTSPDirectory(void);
      virtual ~CHTSPDirectory(void);
      virtual bool GetDirectory(const CStdString& strPath, CFileItemList &items);
      virtual DIR_CACHE_TYPE GetCacheType(const CStdString& strPath) const { return DIR_CACHE_ONCE; };
    private:
      bool GetChannels(const CURI& base, CFileItemList &items);
      bool GetChannels(const CURI& base, CFileItemList &items, HTSP::SChannels channels, int tag);
      bool GetTag     (const CURI& base, CFileItemList &items);

      HTSP::CHTSPDirectorySession* m_session;
  };
}

#endif
