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
 
#include "system.h"
#include "DVDFactoryInputStream.h"
#include "DVDInputStream.h"
#include "DVDInputStreamFile.h"
#include "DVDInputStreamPlaylist.h"
#include "DVDInputStreamNavigator.h"
#include "DVDInputStreamHttp.h"
#include "DVDInputStreamFFmpeg.h"
#include "../../../FileSystem/cdioSupport.h"
#include "DVDInputStreamTV.h"
#include "DVDInputStreamRTMP.h"
#ifdef HAVE_LIBBLURAY
#include "DVDInputStreamBluray.h"
#endif
#ifdef HAS_FILESYSTEM_HTSP
#include "DVDInputStreamHTSP.h"
#endif
#ifdef ENABLE_DVDINPUTSTREAM_STACK
#include "DVDInputStreamStack.h"
#endif
#ifdef HAS_FILESYSTEM_MMS
#include "DVDInputStreamMMS.h"
#endif
#include "FileItem.h"

CDVDInputStream* CDVDFactoryInputStream::CreateInputStream(IDVDPlayer* pPlayer, const std::string& file, const std::string& content)
{
  CFileItem item(file.c_str(), false);
  if (content != "bluray/iso" && (item.IsDVDFile(false, true) || item.IsDVDImage() ||
#ifdef HAS_DVD_DRIVE
#ifdef _WIN32
    file.compare(MEDIA_DETECT::CLibcdio::GetInstance()->GetDeviceFileName()+4) == 0 ))
#else
    file.compare(MEDIA_DETECT::CLibcdio::GetInstance()->GetDeviceFileName()) == 0 ))
#endif
#else
  0 ))
#endif
  {
    return (new CDVDInputStreamNavigator(pPlayer));
  }
#ifdef HAVE_LIBBLURAY
  else if (item.IsType(".bdmv") || item.IsType(".mpls") || content == "bluray/iso")
  { 
    return new CDVDInputStreamBluray(pPlayer);  
  } 
#endif
  else if(file.substr(0, 6) == "rtp://"
       || file.substr(0, 7) == "rtsp://"
       || file.substr(0, 6) == "sdp://"
       || file.substr(0, 6) == "udp://"
       || file.substr(0, 6) == "tcp://")
    return new CDVDInputStreamFFmpeg();
  else if(file.substr(0, 7) == "myth://"
       || file.substr(0, 8) == "cmyth://"
       || file.substr(0, 8) == "gmyth://"
       || file.substr(0, 6) == "vtp://"
       || file.substr(0, 6) == "dvb://")
    return new CDVDInputStreamTV();
#ifdef ENABLE_DVDINPUTSTREAM_STACK
  else if(file.substr(0, 8) == "stack://")
    return new CDVDInputStreamStack();
#endif
  else if(file.substr(0, 7) == "rtmp://")
    return new CDVDInputStreamRTMP();
#ifdef HAS_FILESYSTEM_HTSP
  else if(file.substr(0, 7) == "htsp://")
    return new CDVDInputStreamHTSP();
#endif
#ifdef HAS_FILESYSTEM_MMS
  else if(file.substr(0,6) == "mms://" || file.substr(0,7) == "mmsh://")
    return new CDVDInputStreamMMS();
#endif
  else if(file.substr(0,11) == "playlist://")
    return new CDVDInputStreamPlaylist();

  //else if (item.IsShoutCast())
  //  /* this should be replaced with standard file as soon as ffmpeg can handle raw aac */
  //  /* currently ffmpeg isn't able to detect that */
  //  return (new CDVDInputStreamHttp());
  //else if (item.IsInternetStream() )  
  //  return (new CDVDInputStreamHttp());
  
  // our file interface handles all these types of streams
  return (new CDVDInputStreamFile());
}
