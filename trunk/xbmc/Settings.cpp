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
#include "Settings.h"
#include "AdvancedSettings.h"
#include "Application.h"
#include "KeyboardLayoutConfiguration.h"
#include "Util.h"
#include "URL.h"
#include "GUIWindowFileManager.h"
#include "GUIDialogButtonMenu.h"
#include "GUIFontManager.h"
#include "LangCodeExpander.h"
#include "ButtonTranslator.h"
#include "XMLUtils.h"
#include "GUIPassword.h"
#include "GUIAudioManager.h"
#include "AudioContext.h"
#include "utils/GUIInfoManager.h"
#include "utils/Network.h"
#include "FileSystem/MultiPathDirectory.h"
#include "FileSystem/SpecialProtocol.h"
#include "GUIBaseContainer.h" // for VIEW_TYPE enum
#include "GUIUserMessages.h"
#include "MediaManager.h"
#include "DNSNameCache.h"
#include "GUIWindowManager.h"
#include "GUIDialogYesNo.h"
#include "FileSystem/Directory.h"
#include "FileItem.h"
#include "utils/SystemInfo.h"
#include "lib/libBoxee/bxutils.h"
#include "LangInfo.h"
#include "LocalizeStrings.h"
#include "StringUtils.h"
#ifdef _WIN32
#include "win32/WIN32Util.h"
#endif
#if defined(_LINUX) && defined(HAS_FILESYSTEM_SMB)
#include "FileSystem/SMBDirectory.h"
#endif
#include "playercorefactory/PlayerCoreFactory.h"
#include "cores/VideoRenderers/RenderManager.h"
#include "BoxeeUtils.h"
#include "AppManager.h"
#include "GUIDialogYesNo2.h"
#include "BoxeeBrowseMenuManager.h"
#include "lib/libBoxee/boxee.h"
#include <algorithm>

#include "LicenseConfig.h"

using namespace std;
using namespace XFILE;
using namespace DIRECTORY;

struct CSettings::stSettings g_stSettings;
class CSettings g_settings;

extern CStdString g_LoadErrorStr;



CSettings::CSettings(void)
{
  m_settingsVersion = 1;
}

void CSettings::Initialize()
{
  RESOLUTION_INFO res;
  vector<RESOLUTION_INFO>::iterator it = m_ResInfo.begin();

  m_ResInfo.insert(it, RES_CUSTOM, res);

  for (int i = RES_HDTV_1080i; i <= RES_PAL60_16x9; i++)
  {
    g_graphicsContext.ResetScreenParameters((RESOLUTION)i);
    g_graphicsContext.ResetOverscan((RESOLUTION)i, m_ResInfo[i].Overscan);
  }

  g_stSettings.m_iMyVideoStack = STACK_NONE;

  g_stSettings.m_bMyMusicSongInfoInVis = true;    // UNUSED - depreciated.
  g_stSettings.m_bMyMusicSongThumbInVis = false;  // used for music info in vis screen

  g_stSettings.m_bMyMusicPlaylistRepeat = false;
  g_stSettings.m_bMyMusicPlaylistShuffle = false;

  g_stSettings.m_bMyVideoPlaylistRepeat = false;
  g_stSettings.m_bMyVideoPlaylistShuffle = false;
  g_stSettings.m_bMyVideoNavFlatten = false;
  g_stSettings.m_bStartVideoWindowed = false;

  g_stSettings.m_nVolumeLevel = 0;
  g_stSettings.m_dynamicRangeCompressionLevel = 0;
  g_stSettings.m_iPreMuteVolumeLevel = 0;
  g_stSettings.m_bMute = false;
  g_stSettings.m_fZoomAmount = 1.0f;
  g_stSettings.m_fPixelRatio = 1.0f;
  g_stSettings.m_bNonLinStretch = false;

  g_stSettings.m_pictureExtensions = ".png|.jpg|.jpeg|.bmp|.gif|.ico|.tif|.tiff|.tga|.pcx|.cbz|.cbr|.dng|.nef|.cr2|.crw|.orf|.arw|.erf|.3fr|.dcr|.x3f|.mef|.raf|.mrw|.pef|.sr2";
  g_stSettings.m_musicExtensions = ".nsv|.m4a|.flac|.aac|.strm|.pls|.rm|.rma|.mpa|.wav|.wma|.ogg|.mp3|.mp2|.m3u|.mod|.amf|.669|.dmf|.dsm|.far|.gdm|.imf|.it|.m15|.med|.okt|.s3m|.stm|.sfx|.ult|.uni|.xm|.sid|.ec3|.hbra|.aif|.aiff|.wpl|.ape|.mac|.mpc|.mp+|.mpp|.shn|.zip|.rar|.wv|.nsf|.spc|.gym|.adplug|.adx|.dsp|.adp|.ymf|.ast|.afc|.hps|.xsp|.xwav|.waa|.wvs|.wam|.gcm|.idsp|.mpdsp|.mss|.spt|.rsd|.mid|.kar|.sap|.cmc|.cmr|.dmc|.mpt|.mpd|.rmt|.tmc|.tm8|.tm2|.oga|.pxml|.m4b|.spx";
  g_stSettings.m_videoExtensions = ".m4v|.3g2|.3gp|.nsv|.tp|.ts|.ty|.strm|.pls|.rm|.rmvb|.m3u|.m3u8|.ifo|.mov|.qt|.divx|.xvid|.bivx|.vob|.nrg|.img|.iso|.pva|.wmv|.asf|.asx|.ogm|.m2v|.avi|.dat|.mpg|.mpeg|.mp4|.mkv|.avc|.vp3|.svq3|.nuv|.viv|.dv|.fli|.flv|.f4v|.rar|.001|.wpl|.zip|.vdr|.dvr-ms|.xsp|.mts|.m2t|.m2ts|.evo|.ogv|.sdp|.avs|.rec|.url|.pxml|.vc1|.h264|.rcv|.webm|.trp|.wtv|.ac3|.eac3|.dts";
  // internal music extensions
  g_stSettings.m_musicExtensions += "|.sidstream|.oggstream|.nsfstream|.asapstream|.cdda";

  #ifdef __APPLE__
    CStdString logDir = getenv("HOME");
    logDir += "/Library/Logs/";
    g_stSettings.m_logFolder = logDir;
  #else
    g_stSettings.m_logFolder = "special://home/";              // log file location
  #endif

  m_iLastLoadedProfileIndex = 0;

  // defaults for scanning
  g_stSettings.m_bMyMusicIsScanning = false;

  g_stSettings.iAdditionalSubtitleDirectoryChecked = 0;

  g_settings.bUseLoginScreen = false;

  g_stSettings.m_doneFTU = false;
  g_stSettings.m_doneFTU2 = false;
}

CSettings::~CSettings(void)
{
  m_ResInfo.clear();
}


void CSettings::Save() const
{
  if (g_application.m_bStop)
  {
    //don't save settings when we're busy stopping the application
    //a lot of screens try to save settings on deinit and deinit is called
    //for every screen when the application is stopping.
    return ;
  }
  if (!SaveSettings(GetSettingsFile()))
  {
    CLog::Log(LOGERROR, "Unable to save settings to %s", GetSettingsFile().c_str());
  }
}

bool CSettings::Reset()
{
  CLog::Log(LOGINFO, "Resetting settings");
  CFile::Delete(GetSettingsFile());
  Save();
  return LoadSettings(GetSettingsFile());
}

bool CSettings::Load(bool& bXboxMediacenter, bool& bSettings)
{
  // load settings file...
  bXboxMediacenter = bSettings = false;

  CSpecialProtocol::SetProfilePath(GetProfileUserDataFolder());
  CLog::Log(LOGNOTICE, "loading %s", GetSettingsFile().c_str());
  if (!LoadSettings(GetSettingsFile()))
  {
    CLog::Log(LOGERROR, "Unable to load %s, creating new %s with default values", GetSettingsFile().c_str(), GetSettingsFile().c_str());
    Save();
    if (!(bSettings = Reset()))
      return false;
  }

  // clear sources, then load xml file...
  m_fileSources.clear();
  m_musicSources.clear();
  m_pictureSources.clear();
  m_programSources.clear();
  m_videoSources.clear();
  CStdString strXMLFile = GetSourcesFile();
  CLog::Log(LOGNOTICE, "%s",strXMLFile.c_str());
  TiXmlDocument xmlDoc;
  TiXmlElement *pRootElement = NULL;
  if ( xmlDoc.LoadFile( strXMLFile ) )
  {
    pRootElement = xmlDoc.RootElement();
    CStdString strValue;
    if (pRootElement)
      strValue = pRootElement->Value();
    if ( strValue != "sources")
      CLog::Log(LOGERROR, "%s sources.xml file does not contain <sources>", __FUNCTION__);
  }
  else if (CFile::Exists(strXMLFile))
    CLog::Log(LOGERROR, "%s Error loading %s: Line %d, %s", __FUNCTION__, strXMLFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());

  // look for external sources file
  TiXmlNode *pInclude = pRootElement ? pRootElement->FirstChild("remote") : NULL;
  if (pInclude)
  {
    CStdString strRemoteFile = pInclude->FirstChild()->Value();
    if (!strRemoteFile.IsEmpty())
    {
        CLog::Log(LOGDEBUG, "Found <remote> tag");
        CLog::Log(LOGDEBUG, "Attempting to retrieve remote file: %s", strRemoteFile.c_str());
        // sometimes we have to wait for the network
      if (!g_application.getNetwork().IsAvailable(true) && CFile::Exists(strRemoteFile))
        {
        if ( xmlDoc.LoadFile(strRemoteFile) )
          {
      pRootElement = xmlDoc.RootElement();
      CStdString strValue;
      if (pRootElement)
        strValue = pRootElement->Value();
      if ( strValue != "sources")
        CLog::Log(LOGERROR, "%s remote_sources.xml file does not contain <sources>", __FUNCTION__);
    }
    else
          CLog::Log(LOGERROR, "%s unable to load file: %s, Line %d, %s", __FUNCTION__, strRemoteFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
  }
      else
        CLog::Log(LOGNOTICE, "Could not retrieve remote file, defaulting to local sources");
    }
  }

  if (pRootElement)
  { 
    // parse sources...
    GetSources(pRootElement, "programs", m_programSources, m_defaultProgramSource);
    GetSources(pRootElement, "pictures", m_pictureSources, m_defaultPictureSource);
    GetSources(pRootElement, "files", m_fileSources, m_defaultFileSource);
    GetSources(pRootElement, "music", m_musicSources, m_defaultMusicSource);
    GetSources(pRootElement, "video", m_videoSources, m_defaultVideoSource);    
  }

  bXboxMediacenter = true;

  LoadRSSFeeds();
  LoadUserFolderLayout();

  if (m_iLastLoadedProfileIndex != 0)
  {
    // load shortcuts for user that isn't the MasterUser
    m_shortcuts.Load();
  }

  CStdString strKeyMap;
  for(size_t i = 0;i<m_videoSources.size();i++)
  {
    CURI url(m_videoSources[i].strPath);
    if(url.GetUserName() != "")
    {
      strKeyMap = url.GetHostName();
      if(url.GetShareName() != "")
      {
        strKeyMap += "/" + url.GetShareName();
      }
      g_passwordManager.m_mapCIFSPasswordCache[strKeyMap].first = url.GetUserName();
      g_passwordManager.m_mapCIFSPasswordCache[strKeyMap].second = url.GetPassWord();
    }
  }

  return true;
}

// The function returns true if the matching media source was found, and sets the source output parameter
// and false otherwise
bool CSettings::GetSourcesFromPath(const CStdString& _strPath, const CStdString& strType, std::vector<CMediaSource>& vecSources)
{
  size_t n=0;
  
  CStdString strPath = _P(_strPath);

  if (strPath.find("rar://") != std::string::npos || strPath.find("zip://") != std::string::npos)
    strPath = BOXEE::BXUtils::URLDecode(strPath).substr(6);

  strPath = BOXEE::BXUtils::RemoveSMBCredentials(strPath);
  CUtil::AddSlashAtEnd(strPath);
  transform (strPath.begin(), strPath.end(), strPath.begin(), BOXEE::to_lower());

  if (strType == "video" || strType == "")
  {
    for (n = 0; n < g_settings.m_videoSources.size(); n++)
    {
      CStdString strPath2 = _P(g_settings.m_videoSources[n].strPath);
      CUtil::AddSlashAtEnd(strPath2);

      //if we're on upnp we need to compare without the last slash because the slashes in upnp are '\' and encoded
      if ((strPath2.Find("upnp://") != -1) && CUtil::HasSlashAtEnd(strPath2))
      {
        CUtil::RemoveSlashAtEnd(strPath2);
      }

      transform(strPath2.begin(), strPath2.end(), strPath2.begin(), BOXEE::to_lower());

      strPath2 = BOXEE::BXUtils::RemoveSMBCredentials(strPath2);
      std::string::size_type startPos = strPath.find(strPath2, 0);
      if (startPos == 0)
      {
        vecSources.push_back(g_settings.m_videoSources[n]);
      }
    }
  }

  if (strType == "music" || strType == "")
  {
    for (n = 0; n < g_settings.m_musicSources.size(); n++)
    {
      CStdString strPath2 = _P(g_settings.m_musicSources[n].strPath);
      transform(strPath2.begin(), strPath2.end(), strPath2.begin(), BOXEE::to_lower());

      if ((strPath2.Find("upnp://") != -1) && BOXEE::BXUtils::HasSlashAtEnd(strPath2))
      {
        BOXEE::BXUtils::RemoveSlashAtEnd(strPath2);
      }

      strPath2 = BOXEE::BXUtils::RemoveSMBCredentials(strPath2);
      std::string::size_type startPos = strPath.find(strPath2, 0);
      if (startPos == 0)
      {
        vecSources.push_back(g_settings.m_musicSources[n]);
      }
    }
  }

  if (strType == "pictures" || strType == "")
  {
    for (n = 0; n < g_settings.m_pictureSources.size(); n++)
    {
      CStdString strPath2 = _P(g_settings.m_pictureSources[n].strPath);
      transform(strPath2.begin(), strPath2.end(), strPath2.begin(), BOXEE::to_lower());

      if ((strPath2.Find("upnp://") != -1) && BOXEE::BXUtils::HasSlashAtEnd(strPath2))
      {
        BOXEE::BXUtils::RemoveSlashAtEnd(strPath2);
      }

      strPath2 = BOXEE::BXUtils::RemoveSMBCredentials(strPath2);
      std::string::size_type startPos = strPath.find(strPath2, 0);
      if (startPos == 0)
      {
        vecSources.push_back(g_settings.m_pictureSources[n]);
      }
    }
  }
  
  return (vecSources.size() != 0);
}
  
bool CSettings::IsPathOnSource(const CStdString &strPath)
{
  CLog::Log(LOGDEBUG,"CSettings::IsPathOnSource, path = %s (checkpath)", strPath.c_str());
  std::vector<CMediaSource> vecSources;
  return GetSourcesFromPath(strPath, "", vecSources);

}

bool CSettings::GetSoureScanType(const CStdString &strPath, const CStdString &strType, std::map<std::pair<CStdString, CStdString>, int >)
{
  return true;
//  std::vector<CMediaSource>& vecSources;
//  if (GetSourcesFromPath(strPath, strType, vecSources))
//  {
//    return source.m_iScanType;
//  }
//  else
//  {
//    return -1;
//  }
}

VECSOURCES *CSettings::GetSourcesFromType(const CStdString &type)
{
  if (type == "programs" || type == "myprograms")
    return &g_settings.m_programSources;
  else if (type == "files")
  {
    // this nasty block of code is needed as we have to
    // call getlocaldrives after localize strings has been loaded
    bool bAdded=false;
    for (unsigned int i=0;i<g_settings.m_fileSources.size();++i)
    {
      if (g_settings.m_fileSources[i].m_ignore)
      {
        bAdded = true;
        break;
      }
    }
    if (!bAdded)
    {
      VECSOURCES shares;
      g_mediaManager.GetLocalDrives(shares, true);  // true to include Q
      m_fileSources.insert(m_fileSources.end(),shares.begin(),shares.end());
      
      CMediaSource source;
      source.strName = g_localizeStrings.Get(22013);
      source.m_ignore = true;
      source.strPath = "special://profile/";
      source.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
      m_fileSources.push_back(source);
    }

    return &g_settings.m_fileSources;
  }
  else if (type == "music")
    return &g_settings.m_musicSources;
  else if (type == "video")
    return &g_settings.m_videoSources;
  else if (type == "pictures")
    return &g_settings.m_pictureSources;
  else if (type == "upnpmusic")
    return &g_settings.m_UPnPMusicSources;
  else if (type == "upnpvideo")
    return &g_settings.m_UPnPVideoSources;
  else if (type == "upnppictures")
    return &g_settings.m_UPnPPictureSources;
  else if (type == "all")
    return GetAllMediaSources();
  
  return NULL;
}

VECSOURCES* CSettings::GetAllMediaSources(bool allowDuplicatePaths)
{
  /////////////////////////////////////////////////////////////////////////////////
  // Return VECSOURCES that include all of the Video, Music and Pictures sources //
  // (filter duplicate source paths)                                             //
  /////////////////////////////////////////////////////////////////////////////////
  
  m_allMediaSources.clear();

  std::set<CStdString> alreadyAddedSourcePath;
  alreadyAddedSourcePath.clear();
  std::set<CStdString>::iterator it;
 
  // Add video sources
  for(size_t i=0; i<m_videoSources.size(); i++)
  {
    if (!allowDuplicatePaths && (alreadyAddedSourcePath.find(m_videoSources[i].strPath) != alreadyAddedSourcePath.end()))
    {
      continue;
    }

    m_allMediaSources.push_back(m_videoSources[i]);
    alreadyAddedSourcePath.insert(m_videoSources[i].strPath);
  }
 
  // Add music sources
  for(size_t i=0; i<m_musicSources.size(); i++)
  {
    if (!allowDuplicatePaths && (alreadyAddedSourcePath.find(m_musicSources[i].strPath) != alreadyAddedSourcePath.end()))
    {
      continue;
    }

    m_allMediaSources.push_back(m_musicSources[i]);
    alreadyAddedSourcePath.insert(m_musicSources[i].strPath);
  }

  // Add pictures sources
  for(size_t i=0; i<m_pictureSources.size(); i++)
  {
    if (!allowDuplicatePaths && (alreadyAddedSourcePath.find(m_pictureSources[i].strPath) != alreadyAddedSourcePath.end()))
    {
      continue;
    }

    m_allMediaSources.push_back(m_pictureSources[i]);
    alreadyAddedSourcePath.insert(m_pictureSources[i].strPath);
  }

  return &m_allMediaSources;
}

CStdString CSettings::GetDefaultSourceFromType(const CStdString &type)
{
  CStdString defaultShare;
  if (type == "programs" || type == "myprograms")
    defaultShare = g_settings.m_defaultProgramSource;
  else if (type == "files")
    defaultShare = g_settings.m_defaultFileSource;
  else if (type == "music")
    defaultShare = g_settings.m_defaultMusicSource;
  else if (type == "video")
    defaultShare = g_settings.m_defaultVideoSource;
  else if (type == "pictures")
    defaultShare = g_settings.m_defaultPictureSource;
  return defaultShare;
}

void CSettings::GetSources(const TiXmlElement* pRootElement, const CStdString& strTagName, VECSOURCES& items, CStdString& strDefault)
{
  //CLog::Log(LOGDEBUG, "  Parsing <%s> tag", strTagName.c_str());
  strDefault = "";

  items.clear();
  const TiXmlNode *pChild = pRootElement->FirstChild(strTagName.c_str());
  if (pChild)
  {
    pChild = pChild->FirstChild();
    while (pChild > 0)
    {
      CStdString strValue = pChild->Value();
      if (strValue == "source" || strValue == "bookmark") // "bookmark" left in for backwards compatibility
      {
        CMediaSource share;
        if (GetSource(strTagName, pChild, share))
        {
          items.push_back(share);
        }
        else
        {
          CLog::Log(LOGERROR, "    Missing or invalid <name> and/or <path> in source");          
        }
      }

      if (strValue == "default")
      {
        const TiXmlNode *pValueNode = pChild->FirstChild();
        if (pValueNode)
        {
          const char* pszText = pChild->FirstChild()->Value();
          if (strlen(pszText) > 0)
            strDefault = pszText;
          CLog::Log(LOGDEBUG, "    Setting <default> source to : %s", strDefault.c_str());
        }
      }
      pChild = pChild->NextSibling();
    }
  }
  else
  {
    CLog::Log(LOGDEBUG, "  <%s> tag is missing or sources.xml is malformed", strTagName.c_str());    
  }
}

bool CSettings::GetSource(const CStdString &category, const TiXmlNode *source, CMediaSource &share)
{
  //CLog::Log(LOGDEBUG,"    ---- SOURCE START ----");
  const TiXmlNode *pNodeName = source->FirstChild("name");
  CStdString strName;
  if (pNodeName && pNodeName->FirstChild())
  {
    strName = pNodeName->FirstChild()->Value();
    //CLog::Log(LOGDEBUG,"    Found name: %s", strName.c_str());
  }
  // get multiple paths
  vector<CStdString> vecPaths;
  const TiXmlElement *pPathName = source->FirstChildElement("path");
  while (pPathName)
  {
    if (pPathName->FirstChild())
    {
      int pathVersion = 0;
      pPathName->Attribute("pathversion", &pathVersion);
      CStdString strPath = pPathName->FirstChild()->Value();
      strPath = CSpecialProtocol::ReplaceOldPath(strPath, pathVersion);
      // make sure there are no virtualpaths or stack paths defined in xboxmediacenter.xml
      //CLog::Log(LOGDEBUG,"    Found path: %s", strPath.c_str());
      if (!CUtil::IsVirtualPath(strPath) && !CUtil::IsStack(strPath))
      {
        // translate special tags
        if (!strPath.IsEmpty() && strPath.at(0) == '$')
        {
          CStdString strPathOld(strPath);
          strPath = CUtil::TranslateSpecialSource(strPath);
          if (!strPath.IsEmpty())
          {
            //CLog::Log(LOGDEBUG,"    -> Translated to path: %s", strPath.c_str());
          }
          else
          {
            //CLog::Log(LOGERROR,"    -> Skipping invalid token: %s", strPathOld.c_str());
            pPathName = pPathName->NextSiblingElement("path");
            continue;
          }
        }
        vecPaths.push_back(strPath);
      }
      else
        CLog::Log(LOGERROR,"    Invalid path type (%s) in source", strPath.c_str());
    }
    pPathName = pPathName->NextSiblingElement("path");
  }

  const TiXmlNode *pLockMode = source->FirstChild("lockmode");
  const TiXmlNode *pLockCode = source->FirstChild("lockcode");
  const TiXmlNode *pBadPwdCount = source->FirstChild("badpwdcount");
  const TiXmlNode *pThumbnailNode = source->FirstChild("thumbnail");
// BOXEE
  const TiXmlNode *pScanType = source->FirstChild("scantype");
  const TiXmlNode *pAdult = source->FirstChild("adult");
  const TiXmlNode *pCountry = source->FirstChild("country");
  const TiXmlNode *pCountryAllow = source->FirstChild("country-allow");
// END BOXEE
  if (!strName.IsEmpty() && vecPaths.size() > 0)
  {
    vector<CStdString> verifiedPaths;
    // disallowed for files, or theres only a single path in the vector
    if ((category.Equals("files")) || (vecPaths.size() == 1))
      verifiedPaths.push_back(vecPaths[0]);

    // multiple paths?
    else
    {
      // validate the paths
      for (int j = 0; j < (int)vecPaths.size(); ++j)
      {
        CURI url(vecPaths[j]);
        CStdString protocol = url.GetProtocol();
        bool bIsInvalid = false;

        // for my programs
        if (category.Equals("programs") || category.Equals("myprograms"))
        {
          // only allow HD and plugins
          if (url.IsLocal() || protocol.Equals("plugin"))
            verifiedPaths.push_back(vecPaths[j]);
          else
            bIsInvalid = true;
        }

        // for others allow everything (if the user does something silly, we can't stop them)
        else
          verifiedPaths.push_back(vecPaths[j]);

        // error message
        if (bIsInvalid)
          CLog::Log(LOGERROR,"    Invalid path type (%s) for multipath source", vecPaths[j].c_str());
      }

      // no valid paths? skip to next source
      if (verifiedPaths.size() == 0)
      {
        CLog::Log(LOGERROR,"    Missing or invalid <name> and/or <path> in source");
        return false;
      }
    }

    share.FromNameAndPaths(category, strName, verifiedPaths);

/*    CLog::Log(LOGDEBUG,"      Adding source:");
    CLog::Log(LOGDEBUG,"        Name: %s", share.strName.c_str());
    if (CUtil::IsVirtualPath(share.strPath) || CUtil::IsMultiPath(share.strPath))
    {
      for (int i = 0; i < (int)share.vecPaths.size(); ++i)
        CLog::Log(LOGDEBUG,"        Path (%02i): %s", i+1, share.vecPaths.at(i).c_str());
    }
    else
      CLog::Log(LOGDEBUG,"        Path: %s", share.strPath.c_str());
*/
    share.m_iBadPwdCount = 0;
    if (pLockMode)
    {
      share.m_iLockMode = LockType(atoi(pLockMode->FirstChild()->Value()));
      share.m_iHasLock = 2;
    }

    if (pLockCode)
    {
      if (pLockCode->FirstChild())
        share.m_strLockCode = pLockCode->FirstChild()->Value();
    }

    if (pBadPwdCount)
    {
      if (pBadPwdCount->FirstChild())
        share.m_iBadPwdCount = atoi( pBadPwdCount->FirstChild()->Value() );
    }

    if (pThumbnailNode)
    {
      if (pThumbnailNode->FirstChild())
        share.m_strThumbnailImage = pThumbnailNode->FirstChild()->Value();
    }

// BOXEE

    if (pScanType && pScanType->FirstChild())
    {
      share.m_iScanType = atoi( pScanType->FirstChild()->Value() );
    }
    
    share.m_adult = false;
    if (pAdult && pAdult->FirstChild()) 
    {
      share.m_adult = (strcmp(pAdult->FirstChild()->Value(), "true") == 0); 
    }

    share.m_countryAllow = true;
    if (pCountryAllow && pCountryAllow->FirstChild()) 
    {
      share.m_countryAllow = (strcmp(pCountryAllow->FirstChild()->Value(), "true") == 0); 
    }

    share.m_country = "all";
    if (pCountry && pCountry->FirstChild()) 
    {
      share.m_country = pCountry->FirstChild()->Value();
    }

    share.m_type = category;
// END BOXEE

    return true;
  }
  return false;
}

bool CSettings::GetPath(const TiXmlElement* pRootElement, const char *tagName, CStdString &strValue)
{
  CStdString strDefault = strValue;
  if (XMLUtils::GetPath(pRootElement, tagName, strValue))
  { // tag exists
    // check for "-" for backward compatibility
    if (!strValue.Equals("-"))
      return true;
  }
  // tag doesn't exist - set default
  strValue = strDefault;
  return false;
}

bool CSettings::GetString(const TiXmlElement* pRootElement, const char *tagName, CStdString &strValue, const CStdString& strDefaultValue)
{
  if (XMLUtils::GetString(pRootElement, tagName, strValue))
  { // tag exists
    // check for "-" for backward compatibility
    if (!strValue.Equals("-"))
      return true;
  }
  // tag doesn't exist - set default
  strValue = strDefaultValue;
  return false;
}

bool CSettings::GetString(const TiXmlElement* pRootElement, const char *tagName, char *szValue, const CStdString& strDefaultValue)
{
  CStdString strValue;
  bool ret = GetString(pRootElement, tagName, strValue, strDefaultValue);
  if (szValue)
    strcpy(szValue, strValue.c_str());
  return ret;
}

bool CSettings::GetInteger(const TiXmlElement* pRootElement, const char *tagName, int& iValue, const int iDefault, const int iMin, const int iMax)
{
  if (XMLUtils::GetInt(pRootElement, tagName, iValue, iMin, iMax))
    return true;
  // default
  iValue = iDefault;
  return false;
}

bool CSettings::GetUint(const TiXmlElement* pRootElement, const char *strTagName, uint32_t& uValue, const uint32_t uDefault, const uint32_t uMin, const uint32_t uMax)
{
  if (XMLUtils::GetUInt(pRootElement, strTagName, uValue))
    return true;
  // default
  uValue = uDefault;
  return false;

}

bool CSettings::GetFloat(const TiXmlElement* pRootElement, const char *tagName, float& fValue, const float fDefault, const float fMin, const float fMax)
{
  if (XMLUtils::GetFloat(pRootElement, tagName, fValue, fMin, fMax))
    return true;
  // default
  fValue = fDefault;
  return false;
}

void CSettings::GetViewState(const TiXmlElement *pRootElement, const CStdString &strTagName, CViewState &viewState, SORT_METHOD defaultSort, int defaultView)
{
  const TiXmlElement* pNode = pRootElement->FirstChildElement(strTagName);
  if (!pNode)
  {
    viewState.m_sortMethod = defaultSort;
    viewState.m_viewMode = defaultView;
    return;
  }
  GetInteger(pNode, "viewmode", viewState.m_viewMode, defaultView, DEFAULT_VIEW_LIST, DEFAULT_VIEW_MAX);

  int sortMethod;
  GetInteger(pNode, "sortmethod", sortMethod, defaultSort, SORT_METHOD_NONE, SORT_METHOD_MAX);
  viewState.m_sortMethod = (SORT_METHOD)sortMethod;

  int sortOrder;
  GetInteger(pNode, "sortorder", sortOrder, SORT_ORDER_ASC, SORT_ORDER_NONE, SORT_ORDER_DESC);
  viewState.m_sortOrder = (SORT_ORDER)sortOrder;
  }

void CSettings::SetViewState(TiXmlNode *pRootNode, const CStdString &strTagName, const CViewState &viewState) const
{
  TiXmlElement newElement(strTagName);
  TiXmlNode *pNewNode = pRootNode->InsertEndChild(newElement);
  if (pNewNode)
  {
    XMLUtils::SetInt(pNewNode, "viewmode", viewState.m_viewMode);
    XMLUtils::SetInt(pNewNode, "sortmethod", (int)viewState.m_sortMethod);
    XMLUtils::SetInt(pNewNode, "sortorder", (int)viewState.m_sortOrder);
  }
}

bool CSettings::LoadCalibration(const TiXmlElement* pRoot, const CStdString& strSettingsFile)
{
  const TiXmlElement *pElement = pRoot->FirstChildElement("resolutions");
  if (!pElement)
  {
    g_LoadErrorStr.Format("%s Doesn't contain <resolutions>", strSettingsFile.c_str());
    return false;
  }
  const TiXmlElement *pResolution = pElement->FirstChildElement("resolution");
  while (pResolution)
  {
    // get the data for this resolution
    CStdString mode = "";
    XMLUtils::GetString(pResolution, "description", mode);
    // find this resolution in our resolution vector
    for (unsigned int res = 0; res < g_settings.m_ResInfo.size(); res++)
    {
      if (res == RES_WINDOW)
      continue;

      if (g_settings.m_ResInfo[res].strMode == mode)
      { // found, read in the rest of the information for this item
        const TiXmlElement *pOverscan = pResolution->FirstChildElement("overscan");
        if (pOverscan)
        {
          GetInteger(pOverscan, "left", m_ResInfo[res].Overscan.left, 0, -m_ResInfo[res].iWidth / 4, m_ResInfo[res].iWidth / 4);
          GetInteger(pOverscan, "top", m_ResInfo[res].Overscan.top, 0, -m_ResInfo[res].iHeight / 4, m_ResInfo[res].iHeight / 4);
          GetInteger(pOverscan, "right", m_ResInfo[res].Overscan.right, m_ResInfo[res].iWidth, m_ResInfo[res].iWidth / 2, m_ResInfo[res].iWidth*3 / 2);
          GetInteger(pOverscan, "bottom", m_ResInfo[res].Overscan.bottom, m_ResInfo[res].iHeight, m_ResInfo[res].iHeight / 2, m_ResInfo[res].iHeight*3 / 2);
        }

        m_ResInfo[res].strMode =  mode.c_str();

        // get the appropriate "safe graphics area" = 10% for 4x3, 3.5% for 16x9
        float fSafe;
        if (res == RES_PAL_4x3 || res == RES_NTSC_4x3 || res == RES_PAL60_4x3 || res == RES_HDTV_480p_4x3)
          fSafe = 0.1f;
        else
          fSafe = 0.035f;

        GetInteger(pResolution, "subtitles", m_ResInfo[res].iSubtitles, (int)((1 - fSafe)*m_ResInfo[res].iHeight), m_ResInfo[res].iHeight / 2, m_ResInfo[res].iHeight*5 / 4);
        GetFloat(pResolution, "pixelratio", m_ResInfo[res].fPixelRatio, 128.0f / 117.0f, 0.5f, 2.0f);
    /*    CLog::Log(LOGDEBUG, "  calibration for %s %ix%i", m_ResInfo[res].strMode, m_ResInfo[res].iWidth, m_ResInfo[res].iHeight);
        CLog::Log(LOGDEBUG, "    subtitle yposition:%i pixelratio:%03.3f offsets:(%i,%i)->(%i,%i)",
                  m_ResInfo[res].iSubtitles, m_ResInfo[res].fPixelRatio,
                  m_ResInfo[res].Overscan.left, m_ResInfo[res].Overscan.top,
                  m_ResInfo[res].Overscan.right, m_ResInfo[res].Overscan.bottom);*/
      }
    }
    // iterate around
    pResolution = pResolution->NextSiblingElement("resolution");


/* Hmm, these stuff shouldn't be releaded, they should be used instead of our internal
   id counter to select what resolution is affected by this settings
#ifdef HAS_XRANDR
    const CStdString def("");
    CStdString val;
    GetString(pResolution, "xrandrid", val, def);
    strncpy(m_ResInfo[iRes].strId, val.c_str(), sizeof(m_ResInfo[iRes].strId));
    GetString(pResolution, "output", val, def);
    strncpy(m_ResInfo[iRes].strOutput, val.c_str(), sizeof(m_ResInfo[iRes].strOutput));
    GetFloat(pResolution, "refreshrate", m_ResInfo[iRes].fRefreshRate, 0, 0, 200);
#endif
*/
    }
  return true;
}

bool CSettings::SaveCalibration(TiXmlNode* pRootNode) const
{
  TiXmlElement xmlRootElement("resolutions");
  TiXmlNode *pRoot = pRootNode->InsertEndChild(xmlRootElement);

  // save WINDOW, DESKTOP and CUSTOM resolution
  for (size_t i = RES_WINDOW ; i < m_ResInfo.size() ; i++)
  {
    // Write the resolution tag
    TiXmlElement resElement("resolution");
    TiXmlNode *pNode = pRoot->InsertEndChild(resElement);
    // Now write each of the pieces of information we need...
    XMLUtils::SetString(pNode, "description", m_ResInfo[i].strMode);
    XMLUtils::SetInt(pNode, "subtitles", m_ResInfo[i].iSubtitles);
    XMLUtils::SetFloat(pNode, "pixelratio", m_ResInfo[i].fPixelRatio);
#ifdef HAS_XRANDR
    XMLUtils::SetFloat(pNode, "refreshrate", m_ResInfo[i].fRefreshRate);
    XMLUtils::SetString(pNode, "output", m_ResInfo[i].strOutput);
    XMLUtils::SetString(pNode, "xrandrid", m_ResInfo[i].strId);
#endif
    // create the overscan child
    TiXmlElement overscanElement("overscan");
    TiXmlNode *pOverscanNode = pNode->InsertEndChild(overscanElement);
    XMLUtils::SetInt(pOverscanNode, "left", m_ResInfo[i].Overscan.left);
    XMLUtils::SetInt(pOverscanNode, "top", m_ResInfo[i].Overscan.top);
    XMLUtils::SetInt(pOverscanNode, "right", m_ResInfo[i].Overscan.right);
    XMLUtils::SetInt(pOverscanNode, "bottom", m_ResInfo[i].Overscan.bottom);
  }
  return true;
}

bool CSettings::LoadSettings(const CStdString& strSettingsFile)
{
  // load the xml file
  TiXmlDocument xmlDoc;

  if (!xmlDoc.LoadFile(strSettingsFile))
  {
    g_LoadErrorStr.Format("%s, Line %d\n%s", strSettingsFile.c_str(), xmlDoc.ErrorRow(), xmlDoc.ErrorDesc());
    return false;
  }

  TiXmlElement *pRootElement = xmlDoc.RootElement();
  if (strcmpi(pRootElement->Value(), "settings") != 0)
  {
    g_LoadErrorStr.Format("%s\nDoesn't contain <settings>", strSettingsFile.c_str());
    return false;
  }

  if (pRootElement->Attribute("version"))
  {
    m_settingsVersion = atoi(pRootElement->Attribute("version"));
  }

  // mymusic settings
  TiXmlElement *pElement = pRootElement->FirstChildElement("mymusic");
  if (pElement)
  {
    TiXmlElement *pChild = pElement->FirstChildElement("playlist");
    if (pChild)
    {
      XMLUtils::GetBoolean(pChild, "repeat", g_stSettings.m_bMyMusicPlaylistRepeat);
      XMLUtils::GetBoolean(pChild, "shuffle", g_stSettings.m_bMyMusicPlaylistShuffle);
    }
    // if the user happened to reboot in the middle of the scan we save this state
    pChild = pElement->FirstChildElement("scanning");
    if (pChild)
    {
      XMLUtils::GetBoolean(pChild, "isscanning", g_stSettings.m_bMyMusicIsScanning);
    }
    GetInteger(pElement, "startwindow", g_stSettings.m_iMyMusicStartWindow, WINDOW_MUSIC_FILES, WINDOW_MUSIC_FILES, WINDOW_MUSIC_NAV); //501; view songs
    XMLUtils::GetBoolean(pElement, "songinfoinvis", g_stSettings.m_bMyMusicSongInfoInVis);
    XMLUtils::GetBoolean(pElement, "songthumbinvis", g_stSettings.m_bMyMusicSongThumbInVis);
    GetPath(pElement, "defaultlibview", g_settings.m_defaultMusicLibSource);
  }
  // myvideos settings
  pElement = pRootElement->FirstChildElement("myvideos");
  if (pElement)
  {
    GetInteger(pElement, "startwindow", g_stSettings.m_iVideoStartWindow, WINDOW_VIDEO_FILES, WINDOW_VIDEO_FILES, WINDOW_VIDEO_NAV);
    GetInteger(pElement, "stackvideomode", g_stSettings.m_iMyVideoStack, STACK_NONE, STACK_NONE, STACK_SIMPLE);

    GetPath(pElement, "defaultlibview", g_settings.m_defaultVideoLibSource);
    GetInteger(pElement, "watchmode", g_stSettings.m_iMyVideoWatchMode, VIDEO_SHOW_ALL, VIDEO_SHOW_ALL, VIDEO_SHOW_WATCHED);
    XMLUtils::GetBoolean(pElement, "flatten", g_stSettings.m_bMyVideoNavFlatten);

    TiXmlElement *pChild = pElement->FirstChildElement("playlist");
    if (pChild)
    { // playlist
      XMLUtils::GetBoolean(pChild, "repeat", g_stSettings.m_bMyVideoPlaylistRepeat);
      XMLUtils::GetBoolean(pChild, "shuffle", g_stSettings.m_bMyVideoPlaylistShuffle);
    }
  }

  pElement = pRootElement->FirstChildElement("viewstates");
  if (pElement)
  {
    GetViewState(pElement, "musicnavartists", g_stSettings.m_viewStateMusicNavArtists);
    GetViewState(pElement, "musicnavalbums", g_stSettings.m_viewStateMusicNavAlbums);
    GetViewState(pElement, "musicnavsongs", g_stSettings.m_viewStateMusicNavSongs);
    GetViewState(pElement, "musicshoutcast", g_stSettings.m_viewStateMusicShoutcast);
    GetViewState(pElement, "musiclastfm", g_stSettings.m_viewStateMusicLastFM);
    GetViewState(pElement, "videonavactors", g_stSettings.m_viewStateVideoNavActors);
    GetViewState(pElement, "videonavyears", g_stSettings.m_viewStateVideoNavYears);
    GetViewState(pElement, "videonavgenres", g_stSettings.m_viewStateVideoNavGenres);
    GetViewState(pElement, "videonavtitles", g_stSettings.m_viewStateVideoNavTitles);
    GetViewState(pElement, "videonavepisodes", g_stSettings.m_viewStateVideoNavEpisodes, SORT_METHOD_EPISODE);
    GetViewState(pElement, "videonavtvshows", g_stSettings.m_viewStateVideoNavTvShows);
    GetViewState(pElement, "videonavseasons", g_stSettings.m_viewStateVideoNavSeasons);
    GetViewState(pElement, "videonavmusicvideos", g_stSettings.m_viewStateVideoNavMusicVideos);

    GetViewState(pElement, "programs", g_stSettings.m_viewStatePrograms, SORT_METHOD_LABEL, DEFAULT_VIEW_AUTO);
    GetViewState(pElement, "pictures", g_stSettings.m_viewStatePictures, SORT_METHOD_LABEL, DEFAULT_VIEW_AUTO);
    GetViewState(pElement, "videofiles", g_stSettings.m_viewStateVideoFiles, SORT_METHOD_LABEL, DEFAULT_VIEW_AUTO);
    GetViewState(pElement, "musicfiles", g_stSettings.m_viewStateMusicFiles, SORT_METHOD_LABEL, DEFAULT_VIEW_AUTO);
  }

  // general settings
  pElement = pRootElement->FirstChildElement("general");
  if (pElement)
  {
    GetInteger(pElement, "systemtotaluptime", g_stSettings.m_iSystemTimeTotalUp, 0, 0, INT_MAX);
    GetInteger(pElement, "httpapibroadcastlevel", g_stSettings.m_HttpApiBroadcastLevel, 0, 0,5);
    GetInteger(pElement, "httpapibroadcastport", g_stSettings.m_HttpApiBroadcastPort, 8278, 1, 65535);
    int nTime=0;
    GetInteger(pElement, "lasttimecheckforthumbremoval", nTime, 0, 0, INT_MAX);
    g_stSettings.m_lastTimeCheckForThumbRemoval = nTime;
  }

  pElement = pRootElement->FirstChildElement("defaultvideosettings");
  if (pElement)
  {
    int interlaceMethod;
    GetInteger(pElement, "interlacemethod", interlaceMethod, VS_INTERLACEMETHOD_NONE, VS_INTERLACEMETHOD_NONE, VS_INTERLACEMETHOD_INVERSE_TELECINE);
    g_stSettings.m_defaultVideoSettings.m_InterlaceMethod = (EINTERLACEMETHOD)interlaceMethod;
    int scalingMethod;
    GetInteger(pElement, "scalingmethod", scalingMethod, VS_SCALINGMETHOD_LINEAR, VS_SCALINGMETHOD_NEAREST, VS_SCALINGMETHOD_CUBIC);
    g_stSettings.m_defaultVideoSettings.m_ScalingMethod = (ESCALINGMETHOD)scalingMethod;

    GetInteger(pElement, "viewmode", g_stSettings.m_defaultVideoSettings.m_ViewMode, VIEW_MODE_NORMAL, VIEW_MODE_NORMAL, VIEW_MODE_CUSTOM);
    GetFloat(pElement, "zoomamount", g_stSettings.m_defaultVideoSettings.m_CustomZoomAmount, 1.0f, 0.5f, 2.0f);
    GetFloat(pElement, "pixelratio", g_stSettings.m_defaultVideoSettings.m_CustomPixelRatio, 1.0f, 0.5f, 2.0f);
    GetFloat(pElement, "volumeamplification", g_stSettings.m_defaultVideoSettings.m_VolumeAmplification, VOLUME_DRC_MINIMUM * 0.01f, VOLUME_DRC_MINIMUM * 0.01f, VOLUME_DRC_MAXIMUM * 0.01f);
    GetFloat(pElement, "noisereduction", g_stSettings.m_defaultVideoSettings.m_NoiseReduction, 0.0f, 0.0f, 1.0f);
    GetFloat(pElement, "sharpness", g_stSettings.m_defaultVideoSettings.m_Sharpness, 0.0f, -1.0f, 1.0f);
    XMLUtils::GetBoolean(pElement, "outputtoallspeakers", g_stSettings.m_defaultVideoSettings.m_OutputToAllSpeakers);
    XMLUtils::GetBoolean(pElement, "showsubtitles", g_stSettings.m_defaultVideoSettings.m_SubtitleOn);
    GetFloat(pElement, "brightness", g_stSettings.m_defaultVideoSettings.m_Brightness, 50, 0, 100);
    GetFloat(pElement, "contrast", g_stSettings.m_defaultVideoSettings.m_Contrast, 50, 0, 100);
    GetFloat(pElement, "gamma", g_stSettings.m_defaultVideoSettings.m_Gamma, 20, 0, 100);
    GetFloat(pElement, "audiodelay", g_stSettings.m_defaultVideoSettings.m_AudioDelay, 0.0f, -10.0f, 10.0f);
    GetFloat(pElement, "subtitledelay", g_stSettings.m_defaultVideoSettings.m_SubtitleDelay, 0.0f, -50.0f, 50.0f);

    g_stSettings.m_defaultVideoSettings.m_SubtitleCached = false;
  }
  // audio settings
  pElement = pRootElement->FirstChildElement("audio");
  if (pElement)
  {
    GetInteger(pElement, "volumelevel", g_stSettings.m_nVolumeLevel, VOLUME_MAXIMUM, VOLUME_MINIMUM, VOLUME_MAXIMUM);

    // volume is not only availible with dedicated buttons on the remote
    // We set the volume to max for user without it
    g_stSettings.m_nVolumeLevel = VOLUME_MAXIMUM;

    GetInteger(pElement, "dynamicrangecompression", g_stSettings.m_dynamicRangeCompressionLevel, VOLUME_DRC_MINIMUM, VOLUME_DRC_MINIMUM, VOLUME_DRC_MAXIMUM);
    for (int i = 0; i < 4; i++)
    {
      CStdString setting;
      setting.Format("karaoke%i", i);
#define XVOICE_MASK_PARAM_DISABLED (-1.0f)
      GetFloat(pElement, setting + "energy", g_stSettings.m_karaokeVoiceMask[i].energy, XVOICE_MASK_PARAM_DISABLED, XVOICE_MASK_PARAM_DISABLED, 1.0f);
      GetFloat(pElement, setting + "pitch", g_stSettings.m_karaokeVoiceMask[i].pitch, XVOICE_MASK_PARAM_DISABLED, XVOICE_MASK_PARAM_DISABLED, 1.0f);
      GetFloat(pElement, setting + "whisper", g_stSettings.m_karaokeVoiceMask[i].whisper, XVOICE_MASK_PARAM_DISABLED, XVOICE_MASK_PARAM_DISABLED, 1.0f);
      GetFloat(pElement, setting + "robotic", g_stSettings.m_karaokeVoiceMask[i].robotic, XVOICE_MASK_PARAM_DISABLED, XVOICE_MASK_PARAM_DISABLED, 1.0f);
    }
  }

  // FTU
  pElement = pRootElement->FirstChildElement("ftu");
  if (pElement)
  {
    int isDoneFTU;
    pElement->Attribute("done",&isDoneFTU);
    g_stSettings.m_doneFTU = isDoneFTU;

    int isDoneFTU2;
    pElement->Attribute("done2",&isDoneFTU2);
    g_stSettings.m_doneFTU2 = isDoneFTU2;
  }

  LoadCalibration(pRootElement, strSettingsFile);
  g_guiSettings.LoadXML(pRootElement);
  LoadSkinSettings(pRootElement);

  // Configure the PlayerCoreFactory
  LoadPlayerCoreFactorySettings("special://xbmc/system/playercorefactory.xml", true);
  LoadPlayerCoreFactorySettings(g_settings.GetUserDataItem("playercorefactory.xml"), false);

  // Advanced settings
  g_advancedSettings.Load();

  // Default players?
  CLog::Log(LOGNOTICE, "Default DVD Player: %s", g_advancedSettings.m_videoDefaultDVDPlayer.c_str());
  CLog::Log(LOGNOTICE, "Default Video Player: %s", g_advancedSettings.m_videoDefaultPlayer.c_str());
  CLog::Log(LOGNOTICE, "Default Audio Player: %s", g_advancedSettings.m_audioDefaultPlayer.c_str());

  // Upgrade procedure
  UpgradeSettings();

  return true;
}

void CSettings::UpgradeSettings()
{
  int originVersion = m_settingsVersion;

  if (m_settingsVersion == 1)
  {
#ifdef HAS_EMBEDDED
    // Upgrade to projectm
    g_guiSettings.SetString("mymusic.visualisation", "ProjectM.vis");
#endif
    ++m_settingsVersion;
  }

  // ...
  if (m_settingsVersion != originVersion)
    Save();
}

bool CSettings::LoadPlayerCoreFactorySettings(const CStdString& fileStr, bool clear)
{
  CLog::Log(LOGNOTICE, "Loading player core factory settings from %s.", fileStr.c_str());
  if (!CFile::Exists(fileStr))
  { // tell the user it doesn't exist
    CLog::Log(LOGNOTICE, "%s does not exist. Skipping.", fileStr.c_str());
    return false;
  }

  TiXmlDocument playerCoreFactoryXML;
  if (!playerCoreFactoryXML.LoadFile(fileStr))
  {
    CLog::Log(LOGERROR, "Error loading %s, Line %d (%s)", fileStr.c_str(), playerCoreFactoryXML.ErrorRow(), playerCoreFactoryXML.ErrorDesc());
    return false;
  }

  return CPlayerCoreFactory::LoadConfiguration(playerCoreFactoryXML.RootElement(), clear);
}

bool CSettings::SaveSettings(const CStdString& strSettingsFile, CGUISettings *localSettings /* = NULL */) const
{
  TiXmlDocument xmlDoc;
  TiXmlElement xmlRootElement("settings");

  char versionStr[16];
  sprintf(versionStr, "%d", m_settingsVersion);
  xmlRootElement.SetAttribute("version", versionStr);

  TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
  if (!pRoot) return false;
  // write our tags one by one - just a big list for now (can be flashed up later)

  // mymusic settings
  TiXmlElement musicNode("mymusic");
  TiXmlNode *pNode = pRoot->InsertEndChild(musicNode);
  if (!pNode) return false;
  {
    TiXmlElement childNode("playlist");
    TiXmlNode *pChild = pNode->InsertEndChild(childNode);
    if (!pChild) return false;
    XMLUtils::SetBoolean(pChild, "repeat", g_stSettings.m_bMyMusicPlaylistRepeat);
    XMLUtils::SetBoolean(pChild, "shuffle", g_stSettings.m_bMyMusicPlaylistShuffle);
  }
  {
    TiXmlElement childNode("scanning");
    TiXmlNode *pChild = pNode->InsertEndChild(childNode);
    if (!pChild) return false;
    XMLUtils::SetBoolean(pChild, "isscanning", g_stSettings.m_bMyMusicIsScanning);
  }

  XMLUtils::SetInt(pNode, "startwindow", g_stSettings.m_iMyMusicStartWindow);
  XMLUtils::SetBoolean(pNode, "songinfoinvis", g_stSettings.m_bMyMusicSongInfoInVis);
  XMLUtils::SetBoolean(pNode, "songthumbinvis", g_stSettings.m_bMyMusicSongThumbInVis);
  XMLUtils::SetPath(pNode, "defaultlibview", g_settings.m_defaultMusicLibSource);

  // myvideos settings
  TiXmlElement videosNode("myvideos");
  pNode = pRoot->InsertEndChild(videosNode);
  if (!pNode) return false;

  XMLUtils::SetInt(pNode, "startwindow", g_stSettings.m_iVideoStartWindow);

  XMLUtils::SetInt(pNode, "stackvideomode", g_stSettings.m_iMyVideoStack);

  XMLUtils::SetPath(pNode, "defaultlibview", g_settings.m_defaultVideoLibSource);

  XMLUtils::SetInt(pNode, "watchmode", g_stSettings.m_iMyVideoWatchMode);
  XMLUtils::SetBoolean(pNode, "flatten", g_stSettings.m_bMyVideoNavFlatten);

  { // playlist window
    TiXmlElement childNode("playlist");
    TiXmlNode *pChild = pNode->InsertEndChild(childNode);
    if (!pChild) return false;
    XMLUtils::SetBoolean(pChild, "repeat", g_stSettings.m_bMyVideoPlaylistRepeat);
    XMLUtils::SetBoolean(pChild, "shuffle", g_stSettings.m_bMyVideoPlaylistShuffle);
  }

  // view states
  TiXmlElement viewStateNode("viewstates");
  pNode = pRoot->InsertEndChild(viewStateNode);
  if (pNode)
  {
    SetViewState(pNode, "musicnavartists", g_stSettings.m_viewStateMusicNavArtists);
    SetViewState(pNode, "musicnavalbums", g_stSettings.m_viewStateMusicNavAlbums);
    SetViewState(pNode, "musicnavsongs", g_stSettings.m_viewStateMusicNavSongs);
    SetViewState(pNode, "musicshoutcast", g_stSettings.m_viewStateMusicShoutcast);
    SetViewState(pNode, "musiclastfm", g_stSettings.m_viewStateMusicLastFM);
    SetViewState(pNode, "videonavactors", g_stSettings.m_viewStateVideoNavActors);
    SetViewState(pNode, "videonavyears", g_stSettings.m_viewStateVideoNavYears);
    SetViewState(pNode, "videonavgenres", g_stSettings.m_viewStateVideoNavGenres);
    SetViewState(pNode, "videonavtitles", g_stSettings.m_viewStateVideoNavTitles);
    SetViewState(pNode, "videonavepisodes", g_stSettings.m_viewStateVideoNavEpisodes);
    SetViewState(pNode, "videonavseasons", g_stSettings.m_viewStateVideoNavSeasons);
    SetViewState(pNode, "videonavtvshows", g_stSettings.m_viewStateVideoNavTvShows);
    SetViewState(pNode, "videonavmusicvideos", g_stSettings.m_viewStateVideoNavMusicVideos);

    SetViewState(pNode, "programs", g_stSettings.m_viewStatePrograms);
    SetViewState(pNode, "pictures", g_stSettings.m_viewStatePictures);
    SetViewState(pNode, "videofiles", g_stSettings.m_viewStateVideoFiles);
    SetViewState(pNode, "musicfiles", g_stSettings.m_viewStateMusicFiles);
  }

  // general settings
  TiXmlElement generalNode("general");
  pNode = pRoot->InsertEndChild(generalNode);
  if (!pNode) return false;
  XMLUtils::SetInt(pNode, "systemtotaluptime", g_stSettings.m_iSystemTimeTotalUp);
  XMLUtils::SetInt(pNode, "httpapibroadcastport", g_stSettings.m_HttpApiBroadcastPort);
  XMLUtils::SetInt(pNode, "httpapibroadcastlevel", g_stSettings.m_HttpApiBroadcastLevel);
  XMLUtils::SetLong(pNode, "lasttimecheckforthumbremoval", g_stSettings.m_lastTimeCheckForThumbRemoval);
  
  // default video settings
  TiXmlElement videoSettingsNode("defaultvideosettings");
  pNode = pRoot->InsertEndChild(videoSettingsNode);
  if (!pNode) return false;
  XMLUtils::SetInt(pNode, "interlacemethod", g_stSettings.m_defaultVideoSettings.m_InterlaceMethod);
  XMLUtils::SetInt(pNode, "scalingmethod", g_stSettings.m_defaultVideoSettings.m_ScalingMethod);
  XMLUtils::SetFloat(pNode, "noisereduction", g_stSettings.m_defaultVideoSettings.m_NoiseReduction);
  XMLUtils::SetFloat(pNode, "sharpness", g_stSettings.m_defaultVideoSettings.m_Sharpness);
  XMLUtils::SetInt(pNode, "viewmode", g_stSettings.m_defaultVideoSettings.m_ViewMode);
  XMLUtils::SetFloat(pNode, "zoomamount", g_stSettings.m_defaultVideoSettings.m_CustomZoomAmount);
  XMLUtils::SetFloat(pNode, "pixelratio", g_stSettings.m_defaultVideoSettings.m_CustomPixelRatio);
  XMLUtils::SetFloat(pNode, "volumeamplification", g_stSettings.m_defaultVideoSettings.m_VolumeAmplification);
  XMLUtils::SetBoolean(pNode, "outputtoallspeakers", g_stSettings.m_defaultVideoSettings.m_OutputToAllSpeakers);
  XMLUtils::SetBoolean(pNode, "showsubtitles", g_stSettings.m_defaultVideoSettings.m_SubtitleOn);
  XMLUtils::SetFloat(pNode, "brightness", g_stSettings.m_defaultVideoSettings.m_Brightness);
  XMLUtils::SetFloat(pNode, "contrast", g_stSettings.m_defaultVideoSettings.m_Contrast);
  XMLUtils::SetFloat(pNode, "gamma", g_stSettings.m_defaultVideoSettings.m_Gamma);
  XMLUtils::SetFloat(pNode, "audiodelay", g_stSettings.m_defaultVideoSettings.m_AudioDelay);
  XMLUtils::SetFloat(pNode, "subtitledelay", g_stSettings.m_defaultVideoSettings.m_SubtitleDelay);


  // audio settings
  TiXmlElement volumeNode("audio");
  pNode = pRoot->InsertEndChild(volumeNode);
  if (!pNode) return false;
  XMLUtils::SetInt(pNode, "volumelevel", g_stSettings.m_nVolumeLevel);
  XMLUtils::SetInt(pNode, "dynamicrangecompression", g_stSettings.m_dynamicRangeCompressionLevel);
  for (int i = 0; i < 4; i++)
  {
    CStdString setting;
    setting.Format("karaoke%i", i);
    XMLUtils::SetFloat(pNode, setting + "energy", g_stSettings.m_karaokeVoiceMask[i].energy);
    XMLUtils::SetFloat(pNode, setting + "pitch", g_stSettings.m_karaokeVoiceMask[i].pitch);
    XMLUtils::SetFloat(pNode, setting + "whisper", g_stSettings.m_karaokeVoiceMask[i].whisper);
    XMLUtils::SetFloat(pNode, setting + "robotic", g_stSettings.m_karaokeVoiceMask[i].robotic);
  }

  // FTU
  TiXmlElement ftuNode("ftu");
  ftuNode.SetAttribute("done",g_stSettings.m_doneFTU);
  ftuNode.SetAttribute("done2",g_stSettings.m_doneFTU2);
  pRoot->InsertEndChild(ftuNode);

  SaveCalibration(pRoot);

  if (localSettings) // local settings to save
    localSettings->SaveXML(pRoot);
  else // save the global settings
    g_guiSettings.SaveXML(pRoot);

  SaveSkinSettings(pRoot);

  // For mastercode
  SaveProfiles( PROFILES_FILE );

  // save the file
  return xmlDoc.SaveFile(strSettingsFile);
}

CStdString CSettings::GetLicenseFile() const
{
  CStdString lic_path;
// Boxee
//  if (g_settings.m_iLastLoadedProfileIndex == 0)
    lic_path = "special://xbmc/system/license.xml";
//    settings = "special://profile/guisettings.xml";
// Boxee end

  return lic_path;
}

bool CSettings::LoadProfile(int index)
{
  int iOldIndex = m_iLastLoadedProfileIndex;
  m_iLastLoadedProfileIndex = index;
  bool bSourcesXML=true;
  CStdString strOldSkin = g_guiSettings.GetString("lookandfeel.skin");
  CStdString strOldFont = g_guiSettings.GetString("lookandfeel.font");
  CStdString strOldTheme = g_guiSettings.GetString("lookandfeel.skintheme");
  CStdString strOldColors = g_guiSettings.GetString("lookandfeel.skincolors");
  CStdString strOldLanguage = g_guiSettings.GetString("locale.language");
  //int iOldRes = g_guiSettings.GetInt("videoscreen.resolution");

  ////////////////////////////////////////////////////////
  // if needed -> send user Profile Files  to the server //
  ////////////////////////////////////////////////////////

  bool succeeded = UpdateServerOfProfileFiles();

  if (!succeeded)
  {
    CLog::Log(LOGERROR,"CSettings::LoadProfile - FAILED to report to server of profile files (rtspf)");
  }

  if (Load(bSourcesXML,bSourcesXML))
  {
    g_settings.CreateProfileFolders();

    // initialize our charset converter
    g_charsetConverter.reset();

    // Load the langinfo to have user charset <-> utf-8 conversion
    CStdString strLanguage = g_guiSettings.GetString("locale.language");
    
    if (!strLanguage.Equals(strOldLanguage))
    {
    strLanguage[0] = toupper(strLanguage[0]);

    CStdString strLangInfoPath;
    strLangInfoPath.Format("special://xbmc/language/%s/langinfo.xml", strLanguage.c_str());
    CLog::Log(LOGINFO, "load language info file:%s", strLangInfoPath.c_str());
    g_langInfo.Load(strLangInfoPath);

#ifdef _XBOX
    CStdString strKeyboardLayoutConfigurationPath;
    strKeyboardLayoutConfigurationPath.Format("special://xbmc/language/%s/keyboardmap.xml", strLanguage.c_str());
    CLog::Log(LOGINFO, "load keyboard layout configuration info file: %s", strKeyboardLayoutConfigurationPath.c_str());
    g_keyboardLayoutConfiguration.Load(strKeyboardLayoutConfigurationPath);
#endif

    CStdString strLanguagePath;
    strLanguagePath.Format("special://xbmc/language/%s/strings.xml", strLanguage.c_str());

    CButtonTranslator::GetInstance().Load();
    g_localizeStrings.Load(strLanguagePath);

    g_infoManager.ResetCache();
    g_infoManager.ResetLibraryBools();

    // always reload the skin - we need it for the new language strings
    g_application.LoadSkin(g_guiSettings.GetString("lookandfeel.skin"));
    }

    if (m_iLastLoadedProfileIndex != 0)
    {
      TiXmlDocument doc;
      if (doc.LoadFile(CUtil::AddFileToFolder(GetUserDataFolder(),"guisettings.xml")))
        g_guiSettings.LoadMasterLock(doc.RootElement());
    }

    // to set labels - shares are reloaded
#if !defined(_WIN32) && defined(HAS_DVD_DRIVE)
    MEDIA_DETECT::CDetectDVDMedia::UpdateState();
#endif
    // init windows
    CGUIMessage msg(GUI_MSG_NOTIFY_ALL,0,0,GUI_MSG_WINDOW_RESET);
    g_windowManager.SendMessage(msg);

    CUtil::DeleteMusicDatabaseDirectoryCache();
    CUtil::DeleteVideoDatabaseDirectoryCache();

#ifdef HAS_EMBEDDED
    g_lic_settings.Load();
#endif
    
    return true;
  }

  m_iLastLoadedProfileIndex = iOldIndex;

  return false;
}

bool CSettings::DeleteProfile(int index)
{
  if (index < 0 && index >= (int)g_settings.m_vecProfiles.size())
    return false;

  CStdString message;
  CStdString str = g_localizeStrings.Get(55691);
  message.Format(str.c_str(), g_settings.m_vecProfiles.at(index).getName());

  if (CGUIDialogYesNo2::ShowAndGetInput(g_localizeStrings.Get(55690),message,0))
  {
    //delete profile
    CStdString strDirectory = g_settings.m_vecProfiles[index].getDirectory();
    m_vecProfiles.erase(g_settings.m_vecProfiles.begin()+index);
    if (index == g_settings.m_iLastLoadedProfileIndex)
    {
      g_settings.LoadProfile(0);
      g_settings.Save();
    }

    CFileItem item(CUtil::AddFileToFolder(GetUserDataFolder(), strDirectory));
    item.m_strPath = CUtil::AddFileToFolder(GetUserDataFolder(), strDirectory + "\\");
    item.m_bIsFolder = true;
    item.Select(true);
    CGUIWindowFileManager::DeleteItem(&item,false);
  }
  else
  {
    return false;
  }

  SaveProfiles( PROFILES_FILE );

  return true;
}

bool CSettings::SaveSettingsToProfile(int index)
{
  /*CProfile& profile = m_vecProfiles.at(index);
  return SaveSettings(profile.getFileName(), false);*/
  return true;
}


bool CSettings::LoadProfiles(const CStdString& strSettingsFile)
{
  TiXmlDocument profilesDoc;
  CStdString backup = strSettingsFile + ".bak";
  
  if (!CFile::Exists(strSettingsFile) && !CFile::Exists(backup))
  { // set defaults, or assume no rss feeds??
	  CLog::Log(LOGWARNING, "Settings file does not exist, defaults assumed, path = %s", strSettingsFile.c_str());
    return false;
  }
  if (!profilesDoc.LoadFile(strSettingsFile))
  {
    int nRow = profilesDoc.ErrorRow();
    CStdString strErrorDesc = profilesDoc.ErrorDesc();
    if (!profilesDoc.LoadFile(backup.c_str()))
    {
      CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", strSettingsFile.c_str(), nRow, strErrorDesc.c_str());
      return false;
    }
    else
      ::CopyFile(backup, strSettingsFile, false);
  }

  TiXmlElement *pRootElement = profilesDoc.RootElement();
  if (!pRootElement || strcmpi(pRootElement->Value(),"profiles") != 0)
  {
    CLog::Log(LOGERROR, "Error loading %s, no <profiles> node", strSettingsFile.c_str());
    return false;
  }
  GetInteger(pRootElement,"lastloaded",m_iLastLoadedProfileIndex,0,0,1000);
  if (m_iLastLoadedProfileIndex < 0)
    m_iLastLoadedProfileIndex = 0;

  XMLUtils::GetBoolean(pRootElement,"useloginscreen",bUseLoginScreen);

  TiXmlElement* pProfile = pRootElement->FirstChildElement("profile");
  CProfile profile;

  while (pProfile)
  {
    profile.setName("Master user");
    if (CDirectory::Exists("special://home/userdata"))
      profile.setDirectory("special://home/userdata");
    else
      profile.setDirectory("special://xbmc/userdata");

    CStdString strid;
    XMLUtils::GetString(pProfile,"id",strid);
    profile.setID(strid);

    CStdString strName;
    XMLUtils::GetString(pProfile,"name",strName);
    profile.setName(strName);

    CStdString strDirectory;
    XMLUtils::GetPath(pProfile,"directory",strDirectory);
    profile.setDirectory(strDirectory);

    CStdString strThumb;
    XMLUtils::GetPath(pProfile,"thumbnail",strThumb);
    profile.setThumb(strThumb);

    bool bHas=true;
    XMLUtils::GetBoolean(pProfile, "hasdatabases", bHas);
    profile.setDatabases(bHas);

    bHas = true;
    XMLUtils::GetBoolean(pProfile, "canwritedatabases", bHas);
    profile.setWriteDatabases(bHas);

    bHas = true;
    XMLUtils::GetBoolean(pProfile, "hassources", bHas);
    profile.setSources(bHas);

    bHas = true;
    XMLUtils::GetBoolean(pProfile, "canwritesources", bHas);
    profile.setWriteSources(bHas);

    bHas = false;
    XMLUtils::GetBoolean(pProfile, "locksettings", bHas);
    profile.setSettingsLocked(bHas);

    bHas = false;
    XMLUtils::GetBoolean(pProfile, "lockfiles", bHas);
    profile.setFilesLocked(bHas);

    bHas = false;
    XMLUtils::GetBoolean(pProfile, "lockmusic", bHas);
    profile.setMusicLocked(bHas);

    bHas = false;
    XMLUtils::GetBoolean(pProfile, "lockvideo", bHas);
    profile.setVideoLocked(bHas);

    bHas = false;
    XMLUtils::GetBoolean(pProfile, "lockpictures", bHas);
    profile.setPicturesLocked(bHas);

    bHas = false;
    XMLUtils::GetBoolean(pProfile, "lockprograms", bHas);
    profile.setProgramsLocked(bHas);

    LockType iLockMode;
    int lockMode = (int)LOCK_MODE_EVERYONE;
    XMLUtils::GetInt(pProfile,"lockmode",lockMode);
    iLockMode = (LockType)lockMode;

    if (iLockMode > LOCK_MODE_QWERTY || iLockMode < LOCK_MODE_EVERYONE)
      iLockMode = LOCK_MODE_EVERYONE;
    profile.setLockMode(iLockMode);

    CStdString strLockCode;
    XMLUtils::GetString(pProfile,"lockcode",strLockCode);
    profile.setLockCode(strLockCode);

    CStdString strLastLockCode;
    XMLUtils::GetString(pProfile,"lastlockcode",strLastLockCode);
    profile.setLastLockCode(strLastLockCode);

    CStdString strDate;
    XMLUtils::GetString(pProfile,"lastdate",strDate);
    profile.setDate(strDate);
    
    bHas = true;
    XMLUtils::GetBoolean(pProfile, "lockadult", bHas);
    profile.setAdultLocked(bHas);
    
    CStdString strAdultLockCode;
    XMLUtils::GetString(pProfile,"adultlockcode",strAdultLockCode);
    profile.setAdultLockCode(strAdultLockCode);    

    m_vecProfiles.push_back(profile);
    pProfile = pProfile->NextSiblingElement("profile");
  }

  if (m_iLastLoadedProfileIndex >= (int)m_vecProfiles.size() || m_iLastLoadedProfileIndex < 0)
    m_iLastLoadedProfileIndex = 0;

  return true;
}

bool CSettings::SaveProfiles(const CStdString& strSettingsFile) const
{
  TiXmlDocument xmlDoc;
  TiXmlElement xmlRootElement("profiles");
  TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
  if (!pRoot) return false;
  XMLUtils::SetInt(pRoot,"lastloaded",m_iLastLoadedProfileIndex);
  XMLUtils::SetBoolean(pRoot,"useloginscreen",bUseLoginScreen);
  for (unsigned int iProfile=0;iProfile<g_settings.m_vecProfiles.size();++iProfile)
  {
    TiXmlElement profileNode("profile");
    TiXmlNode *pNode = pRoot->InsertEndChild(profileNode);
    XMLUtils::SetString(pNode,"id",g_settings.m_vecProfiles[iProfile].getID());
    XMLUtils::SetString(pNode,"name",g_settings.m_vecProfiles[iProfile].getName());
    XMLUtils::SetPath(pNode,"directory",g_settings.m_vecProfiles[iProfile].getDirectory());
    XMLUtils::SetPath(pNode,"thumbnail",g_settings.m_vecProfiles[iProfile].getThumb());
    XMLUtils::SetString(pNode,"lastdate",g_settings.m_vecProfiles[iProfile].getDate());

//Boxee
    //if (g_settings.m_vecProfiles[0].getLockMode() != LOCK_MODE_EVERYONE)
//end Boxee
    {
      XMLUtils::SetInt(pNode,"lockmode",g_settings.m_vecProfiles[iProfile].getLockMode());
      //CLog::Log(LOGDEBUG, "CSettings::SaveProfiles, LOGIN, save lock code: %s", g_settings.m_vecProfiles[iProfile].getLockCode().c_str());
      XMLUtils::SetString(pNode,"lockcode",g_settings.m_vecProfiles[iProfile].getLockCode());
      XMLUtils::SetString(pNode,"lastlockcode",g_settings.m_vecProfiles[iProfile].getLastLockCode());
      XMLUtils::SetString(pNode,"adultlockcode",g_settings.m_vecProfiles[iProfile].getAdultLockCode());
      XMLUtils::SetBoolean(pNode,"lockmusic",g_settings.m_vecProfiles[iProfile].musicLocked());
      XMLUtils::SetBoolean(pNode,"lockvideo",g_settings.m_vecProfiles[iProfile].videoLocked());
      XMLUtils::SetBoolean(pNode,"lockpictures",g_settings.m_vecProfiles[iProfile].picturesLocked());
      XMLUtils::SetBoolean(pNode,"lockprograms",g_settings.m_vecProfiles[iProfile].programsLocked());
      XMLUtils::SetBoolean(pNode,"locksettings",g_settings.m_vecProfiles[iProfile].settingsLocked());
      XMLUtils::SetBoolean(pNode,"lockfiles",g_settings.m_vecProfiles[iProfile].filesLocked());
      XMLUtils::SetBoolean(pNode,"rememberpassword",g_settings.m_vecProfiles[iProfile].rememberPassword());
      XMLUtils::SetBoolean(pNode,"lockadult",g_settings.m_vecProfiles[iProfile].adultLocked());      
    }

    if (iProfile > 0)
    {
      XMLUtils::SetBoolean(pNode,"hasdatabases",g_settings.m_vecProfiles[iProfile].hasDatabases());
      XMLUtils::SetBoolean(pNode,"canwritedatabases",g_settings.m_vecProfiles[iProfile].canWriteDatabases());
      XMLUtils::SetBoolean(pNode,"hassources",g_settings.m_vecProfiles[iProfile].hasSources());
      XMLUtils::SetBoolean(pNode,"canwritesources",g_settings.m_vecProfiles[iProfile].canWriteSources());
    }
  }
  // save the file
  CStdString backup = strSettingsFile + ".bak";
  CFile::Cache(strSettingsFile,backup);  
  return xmlDoc.SaveFile(strSettingsFile);
}

bool CSettings::LoadUPnPXml(const CStdString& strSettingsFile)
{
  TiXmlDocument UPnPDoc;

  if (!CFile::Exists(strSettingsFile))
  { // set defaults, or assume no rss feeds??
    return false;
  }
  if (!UPnPDoc.LoadFile(strSettingsFile))
  {
    CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", strSettingsFile.c_str(), UPnPDoc.ErrorRow(), UPnPDoc.ErrorDesc());
    return false;
  }

  TiXmlElement *pRootElement = UPnPDoc.RootElement();
  if (!pRootElement || strcmpi(pRootElement->Value(),"upnpserver") != 0)
  {
    CLog::Log(LOGERROR, "Error loading %s, no <upnpserver> node", strSettingsFile.c_str());
    return false;
  }
  // load settings

  // default values for ports
  g_settings.m_UPnPPortServer = 0;
  g_settings.m_UPnPPortRenderer = 0;
  g_settings.m_UPnPMaxReturnedItems = 0;

  XMLUtils::GetString(pRootElement, "UUID", g_settings.m_UPnPUUIDServer);
  XMLUtils::GetInt(pRootElement, "Port", g_settings.m_UPnPPortServer);
  XMLUtils::GetInt(pRootElement, "MaxReturnedItems", g_settings.m_UPnPMaxReturnedItems);
  XMLUtils::GetString(pRootElement, "UUIDRenderer", g_settings.m_UPnPUUIDRenderer);
  XMLUtils::GetInt(pRootElement, "PortRenderer", g_settings.m_UPnPPortRenderer);

  CStdString strDefault;
  GetSources(pRootElement,"music",g_settings.m_UPnPMusicSources,strDefault);
  GetSources(pRootElement,"video",g_settings.m_UPnPVideoSources,strDefault);
  GetSources(pRootElement,"pictures",g_settings.m_UPnPPictureSources,strDefault);

  return true;
}

bool CSettings::SaveUPnPXml(const CStdString& strSettingsFile) const
{
  TiXmlDocument xmlDoc;
  TiXmlElement xmlRootElement("upnpserver");
  TiXmlNode *pRoot = xmlDoc.InsertEndChild(xmlRootElement);
  if (!pRoot) return false;

  // create a new Element for UUID
  XMLUtils::SetString(pRoot, "UUID", g_settings.m_UPnPUUIDServer);
  XMLUtils::SetInt(pRoot, "Port", g_settings.m_UPnPPortServer);
  XMLUtils::SetInt(pRoot, "MaxReturnedItems", g_settings.m_UPnPMaxReturnedItems);
  XMLUtils::SetString(pRoot, "UUIDRenderer", g_settings.m_UPnPUUIDRenderer);
  XMLUtils::SetInt(pRoot, "PortRenderer", g_settings.m_UPnPPortRenderer);

  VECSOURCES* pShares[3];
  pShares[0] = &g_settings.m_UPnPMusicSources;
  pShares[1] = &g_settings.m_UPnPVideoSources;
  pShares[2] = &g_settings.m_UPnPPictureSources;
  for (int k=0;k<3;++k)
  {
    if ((*pShares)[k].size()==0)
      continue;

    TiXmlElement xmlType("");
    if (k==0)
      xmlType = TiXmlElement("music");
    if (k==1)
      xmlType = TiXmlElement("video");
    if (k==2)
      xmlType = TiXmlElement("pictures");

    TiXmlNode* pNode = pRoot->InsertEndChild(xmlType);

    for (unsigned int j=0;j<(*pShares)[k].size();++j)
    {
      // create a new Element
      TiXmlText xmlName((*pShares)[k][j].strName);
      TiXmlElement eName("name");
      eName.InsertEndChild(xmlName);

      TiXmlElement source("source");
      source.InsertEndChild(eName);

      for (unsigned int i = 0; i < (*pShares)[k][j].vecPaths.size(); i++)
      {
        TiXmlText xmlPath((*pShares)[k][j].vecPaths[i]);
        TiXmlElement ePath("path");
        ePath.InsertEndChild(xmlPath);
        source.InsertEndChild(ePath);
      }

      if (pNode)
        pNode->ToElement()->InsertEndChild(source);
    }
  }
  // save the file
  return xmlDoc.SaveFile(strSettingsFile);
}

bool CSettings::UpdateShare(const CStdString &type, const CStdString oldName, const CMediaSource &share)
{
  VECSOURCES *pShares = GetSourcesFromType(type);

  if (!pShares) return false;

  // update our current share list
  CMediaSource* pShare=NULL;
  for (IVECSOURCES it = pShares->begin(); it != pShares->end(); it++)
  {
    if (((*it).strName).CompareNoCase(oldName) == 0)
    {
      (*it).strName = share.strName;
      (*it).strPath = share.strPath;
      (*it).vecPaths = share.vecPaths;
      (*it).m_strThumbnailImage = share.m_strThumbnailImage;
      (*it).m_iScanType = share.m_iScanType;
      (*it).m_adult = share.m_adult;
      (*it).m_country = share.m_country;
      (*it).m_countryAllow = share.m_countryAllow;
      (*it).m_type = share.m_type;
      
      pShare = &(*it);
      
      break;
    }
  }

  if (!pShare)
    return false;
  
  if (!BOXEE::Boxee::GetInstance().GetMetadataEngine().UpdateMediaShare(oldName, type, pShare->strName, pShare->strPath, pShare->m_type, pShare->m_iScanType))
  {
    CLog::Log(LOGERROR,"CSettings::UpdateShare - FAILED to UpdateMediaShare in Database. [sourceName=%s][type=%s][NewName=%s][NewPath=%s][NewType=%s][NewScanType=%d] (msdb)",oldName.c_str(),type.c_str(),pShare->strName.c_str(),pShare->strPath.c_str(),pShare->m_type.c_str(),pShare->m_iScanType);
  }

  // Update our XML file as well
  return SaveSources();
}

// NOTE: This function does NOT save the sources.xml file - you need to call SaveSources() separately.
bool CSettings::UpdateSource(const CStdString &strType, const CStdString strOldName, const CStdString &strUpdateElement, const CStdString &strUpdateText)
{
  VECSOURCES *pShares = GetSourcesFromType(strType);

  if (!pShares) return false;

  // disallow virtual paths
  if (strUpdateElement.Equals("path") && CUtil::IsVirtualPath(strUpdateText))
    return false;

  for (IVECSOURCES it = pShares->begin(); it != pShares->end(); it++)
  {
    if ((*it).strName == strOldName)
    {
      if ("name" == strUpdateElement)
        (*it).strName = strUpdateText;
      else if ("lockmode" == strUpdateElement)
        (*it).m_iLockMode = LockType(atoi(strUpdateText));
      else if ("lockcode" == strUpdateElement)
        (*it).m_strLockCode = strUpdateText;
      else if ("badpwdcount" == strUpdateElement)
        (*it).m_iBadPwdCount = atoi(strUpdateText);
      else if ("thumbnail" == strUpdateElement)
        (*it).m_strThumbnailImage = strUpdateText;
      else if ("scanType" == strUpdateElement)
        (*it).m_iScanType = atoi(strUpdateText);
      else if ("path" == strUpdateElement)
      {
        (*it).vecPaths.clear();
        (*it).strPath = strUpdateText;
        (*it).vecPaths.push_back(strUpdateText);
      }
      else
        return false;
      
      if (!BOXEE::Boxee::GetInstance().GetMetadataEngine().UpdateMediaShare(strOldName, strType, (*it).strName, (*it).strPath, (*it).m_type, (*it).m_iScanType))
      {
        CLog::Log(LOGERROR,"CSettings::UpdateSource - FAILED to UpdateMediaShare in Database. [sourceName=%s][type=%s][FieldToUpdate=%s][NewValue=%s] (msdb)",strOldName.c_str(),strType.c_str(),strUpdateElement.c_str(),strUpdateText.c_str());
      }

      return true;
    }
  }
  return false;
}


bool CSettings::DeleteSourceBg(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource)
{
  // create a clean process
  bool result(false);
  SettingDeleteShare* SettingDeleteShareTask = new SettingDeleteShare(this,strType,strName,strPath, virtualSource, 0,false);
  if(SettingDeleteShareTask)
  {
    BOXEE::Boxee::GetInstance().GetBoxeeScheduleTaskManager().AddScheduleTask(SettingDeleteShareTask);
    result = true; // half truth - we don't know if the task completes successfully
  }

  return result;
}

bool CSettings::DeleteSource(const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource)
{
  bool found(false);

  {
	// This mutex locks all the sources vector from all the types - it should improve and represent a specific source type
    CSingleLock lock(m_sourcesVecLock);

  VECSOURCES *pShares = GetSourcesFromType(strType);
  if (!pShares) return false;

  for (IVECSOURCES it = pShares->begin(); it != pShares->end(); it++)
  {
    if ((*it).strName == strName && (*it).strPath == strPath)
    {
      CLog::Log(LOGDEBUG,"found share, removing!");
      pShares->erase(it);
      found = true;
      break;
    }
  }
  }

  if (virtualSource)
    return found;
  
  if (!BOXEE::Boxee::GetInstance().GetMetadataEngine().DeleteMediaShare(strName, strPath, strType))
  {
    CLog::Log(LOGERROR,"CSettings::DeleteSource - FAILED to DeleteMediaShare from Database. [name=%s][path=%s][type=%s] (msdb)",strName.c_str(),strPath.c_str(),strType.c_str());
  }

  CLog::Log(LOGINFO,"CSettings::DeleteSource - ERASE, all the files that are under the share [path=%s] (msdb)", strPath.c_str());

  // this is the only table that doesnt have trigger - will be fixed later ?
  CStdString strFolderShareDeletePath = _P(strPath);

  if (strFolderShareDeletePath.Left(7) == "upnp://" && CUtil::HasSlashAtEnd(strFolderShareDeletePath))
  {
    CUtil::RemoveSlashAtEnd(strFolderShareDeletePath);
  }

  if (!BOXEE::Boxee::GetInstance().GetMetadataEngine().RemoveAudioByFolder(strFolderShareDeletePath))
  {
    CLog::Log(LOGERROR,"CSettings::DeleteSource - FAILED to RemoveAudioByFolder [path=%s] from Database (msdb)",(_P(strPath)).c_str());
  }

  if (!BOXEE::Boxee::GetInstance().GetMetadataEngine().RemoveFolderByPath(strFolderShareDeletePath))
  {
    CLog::Log(LOGERROR,"CSettings::DeleteSource - FAILED to RemoveFolderByPath [path=%s] from Database (msdb)",(_P(strPath)).c_str());
  }


  return SaveSources();
}

bool CSettings::AddShare(const CStdString &type, const CMediaSource &share)
{
  VECSOURCES *pShares = GetSourcesFromType(type);
  if (!pShares) return false;

  // translate dir and add to our current shares
  CStdString strPath1 = share.strPath;
  strPath1.ToUpper();
  if(strPath1.IsEmpty())
  {
    CLog::Log(LOGERROR, "unable to add empty path");
    return false;
  }

  CMediaSource shareToAdd = share;
  shareToAdd.m_type = type;

  if (strPath1.at(0) == '$')
  {
    shareToAdd.strPath = CUtil::TranslateSpecialSource(strPath1);
    if (!share.strPath.IsEmpty())
      CLog::Log(LOGDEBUG, "%s Translated (%s) to Path (%s)",__FUNCTION__ ,strPath1.c_str(),shareToAdd.strPath.c_str());
    else
    {
      CLog::Log(LOGDEBUG, "%s Skipping invalid special directory token: %s",__FUNCTION__,strPath1.c_str());
      return false;
    }
  }
  pShares->push_back(shareToAdd);
  
  // Add the share to the watchdog
  g_application.AddPathToWatch(shareToAdd.strPath);

  if (!BOXEE::Boxee::GetInstance().GetMetadataEngine().AddMediaShare(shareToAdd.strName, shareToAdd.strPath, shareToAdd.m_type, shareToAdd.m_iScanType))
  {
    CLog::Log(LOGERROR,"CSettings::AddShare - FAILED to AddMediaShare to Database. [name=%s][path=%s][type=%s][ScanType=%d] (msdb)",shareToAdd.strName.c_str(),shareToAdd.strPath.c_str(),shareToAdd.m_type.c_str(),shareToAdd.m_iScanType);
  }

  if (!share.m_ignore || type.Find("upnp") < 0)
  {
    return SaveSources();
  }
  
  return true;
}

bool CSettings::SaveSources()
{
  // TODO: Should we be specifying utf8 here??
  TiXmlDocument doc;
  TiXmlElement xmlRootElement("sources");
  TiXmlNode *pRoot = doc.InsertEndChild(xmlRootElement);
  if (!pRoot) return false;

  // ok, now run through and save each sources section
  SetSources(pRoot, "programs", g_settings.m_programSources, g_settings.m_defaultProgramSource);
  SetSources(pRoot, "video", g_settings.m_videoSources, g_settings.m_defaultVideoSource);
  SetSources(pRoot, "music", g_settings.m_musicSources, g_settings.m_defaultMusicSource);
  SetSources(pRoot, "pictures", g_settings.m_pictureSources, g_settings.m_defaultPictureSource);
  SetSources(pRoot, "files", g_settings.m_fileSources, g_settings.m_defaultFileSource);

  CBoxeeBrowseMenuManager::GetInstance().ClearDynamicMenuButtons("mn_local_movies_sources");

  return doc.SaveFile(g_settings.GetSourcesFile());
}
  
bool CSettings::SetSources(TiXmlNode *root, const char *section, const VECSOURCES &shares, const char *defaultPath)
{
  TiXmlElement sectionElement(section);
  TiXmlNode *sectionNode = root->InsertEndChild(sectionElement);
  if (sectionNode)
  {
    XMLUtils::SetPath(sectionNode, "default", defaultPath);
    for (unsigned int i = 0; i < shares.size(); i++)
    {
      const CMediaSource &share = shares[i];
      if (share.m_ignore)
        continue;
      TiXmlElement source("source");

      XMLUtils::SetString(&source, "name", share.strName);

      for (unsigned int i = 0; i < share.vecPaths.size(); i++)
        XMLUtils::SetPath(&source, "path", share.vecPaths[i]);

      if (share.m_iHasLock)
      {
        XMLUtils::SetInt(&source, "lockmode", share.m_iLockMode);
        XMLUtils::SetString(&source, "lockcode", share.m_strLockCode);
        XMLUtils::SetInt(&source, "badpwdcount", share.m_iBadPwdCount);
      }
      if (!share.m_strThumbnailImage.IsEmpty())
        XMLUtils::SetPath(&source, "thumbnail", share.m_strThumbnailImage);
// BOXEE
      XMLUtils::SetInt(&source, "scantype", share.m_iScanType);
      if (share.vecPaths.size() == 1 && CUtil::IsApp(share.vecPaths[0]))
      {
        XMLUtils::SetBoolean(&source, "adult", share.m_adult);
        XMLUtils::SetBoolean(&source, "country-allow", share.m_countryAllow);
        XMLUtils::SetString(&source, "country", share.m_country);
      }
// END BOXEE
      sectionNode->InsertEndChild(source);
    }
  }
  return true;
}

void CSettings::LoadSkinSettings(const TiXmlElement* pRootElement)
{
  int number = 0;
  const TiXmlElement *pElement = pRootElement->FirstChildElement("skinsettings");
  if (pElement)
  {
    m_skinStrings.clear();
    m_skinBools.clear();
    const TiXmlElement *pChild = pElement->FirstChildElement("setting");
    while (pChild)
    {
      CStdString settingName = pChild->Attribute("name");
      if (pChild->Attribute("type") && strcmpi(pChild->Attribute("type"),"string") == 0)
      { // string setting
        CSkinString string;
        string.name = settingName;
        string.value = pChild->FirstChild() ? pChild->FirstChild()->Value() : "";
        m_skinStrings.insert(pair<int, CSkinString>(number++, string));
      }
      else
      { // bool setting
        CSkinBool setting;
        setting.name = settingName;
        setting.value = pChild->FirstChild() ? strcmpi(pChild->FirstChild()->Value(), "true") == 0 : false;
        m_skinBools.insert(pair<int, CSkinBool>(number++, setting));
      }
      pChild = pChild->NextSiblingElement("setting");
    }
  }
}

void CSettings::SaveSkinSettings(TiXmlNode *pRootElement) const
{
  // add the <skinsettings> tag
  TiXmlElement xmlSettingsElement("skinsettings");
  TiXmlNode *pSettingsNode = pRootElement->InsertEndChild(xmlSettingsElement);
  if (!pSettingsNode) return;
  for (map<int, CSkinBool>::const_iterator it = m_skinBools.begin(); it != m_skinBools.end(); ++it)
  {
    // Add a <setting type="bool" name="name">true/false</setting>
    TiXmlElement xmlSetting("setting");
    xmlSetting.SetAttribute("type", "bool");
    xmlSetting.SetAttribute("name", (*it).second.name.c_str());
    TiXmlText xmlBool((*it).second.value ? "true" : "false");
    xmlSetting.InsertEndChild(xmlBool);
    pSettingsNode->InsertEndChild(xmlSetting);
  }
  for (map<int, CSkinString>::const_iterator it = m_skinStrings.begin(); it != m_skinStrings.end(); ++it)
  {
    // Add a <setting type="string" name="name">string</setting>
    TiXmlElement xmlSetting("setting");
    xmlSetting.SetAttribute("type", "string");
    xmlSetting.SetAttribute("name", (*it).second.name.c_str());
    TiXmlText xmlLabel((*it).second.value);
    xmlSetting.InsertEndChild(xmlLabel);
    pSettingsNode->InsertEndChild(xmlSetting);
  }
}

void CSettings::Clear()
{
  m_programSources.clear();
  m_pictureSources.clear();
  m_fileSources.clear();
  m_musicSources.clear();
  m_videoSources.clear();
//  m_vecIcons.clear();
  m_vecProfiles.clear();
  m_mapRssUrls.clear();
  m_skinBools.clear();
  m_skinStrings.clear();
}

int CSettings::TranslateSkinString(const CStdString &setting)
{
  CStdString settingName;
  settingName.Format("%s.%s", g_guiSettings.GetString("lookandfeel.skin").c_str(), setting);
  // run through and see if we have this setting
  for (std::map<int, CSkinString>::const_iterator it = m_skinStrings.begin(); it != m_skinStrings.end(); it++)
  {
    if (settingName.Equals((*it).second.name))
      return (*it).first;
  }
  // didn't find it - insert it
  CSkinString skinString;
  skinString.name = settingName;
  m_skinStrings.insert(pair<int, CSkinString>(m_skinStrings.size() + m_skinBools.size(), skinString));
  return m_skinStrings.size() + m_skinBools.size() - 1;
}

const CStdString &CSettings::GetSkinString(int setting) const
{
  std::map<int, CSkinString>::const_iterator it = m_skinStrings.find(setting);
  if (it != m_skinStrings.end())
  {
    return (*it).second.value;
  }
  return StringUtils::EmptyString;
}

void CSettings::SetSkinString(int setting, const CStdString &label)
{
  std::map<int, CSkinString>::iterator it = m_skinStrings.find(setting);
  if (it != m_skinStrings.end())
  {
    (*it).second.value = label;
    return;
  }
  assert(false);
  CLog::Log(LOGFATAL, "%s : Unknown setting requested", __FUNCTION__);
}

void CSettings::ResetSkinSetting(const CStdString &setting)
{
  CStdString settingName;
  settingName.Format("%s.%s", g_guiSettings.GetString("lookandfeel.skin").c_str(), setting);
  // run through and see if we have this setting as a string
  for (std::map<int, CSkinString>::iterator it = m_skinStrings.begin(); it != m_skinStrings.end(); it++)
  {
    if (settingName.Equals((*it).second.name))
    {
      (*it).second.value = "";
      return;
    }
  }
  // and now check for the skin bool
  for (std::map<int, CSkinBool>::iterator it = m_skinBools.begin(); it != m_skinBools.end(); it++)
  {
    if (settingName.Equals((*it).second.name))
    {
      (*it).second.value = false;
      return;
    }
  }
}

int CSettings::TranslateSkinBool(const CStdString &setting)
{
  CStdString settingName;
  settingName.Format("%s.%s", g_guiSettings.GetString("lookandfeel.skin").c_str(), setting);
  // run through and see if we have this setting
  for (std::map<int, CSkinBool>::const_iterator it = m_skinBools.begin(); it != m_skinBools.end(); it++)
  {
    if (settingName.Equals((*it).second.name))
      return (*it).first;
  }
  // didn't find it - insert it
  CSkinBool skinBool;
  skinBool.name = settingName;
  skinBool.value = false;
  m_skinBools.insert(pair<int, CSkinBool>(m_skinBools.size() + m_skinStrings.size(), skinBool));
  return m_skinBools.size() + m_skinStrings.size() - 1;
}

bool CSettings::GetSkinBool(int setting) const
{
  std::map<int, CSkinBool>::const_iterator it = m_skinBools.find(setting);
  if (it != m_skinBools.end())
  {
    return (*it).second.value;
  }
  // default is to return false
  return false;
}

void CSettings::SetSkinBool(int setting, bool set)
{
  std::map<int, CSkinBool>::iterator it = m_skinBools.find(setting);
  if (it != m_skinBools.end())
  {
    (*it).second.value = set;
    return;
  }
  assert(false);
  CLog::Log(LOGFATAL,"%s : Unknown setting requested", __FUNCTION__);
}

void CSettings::ResetSkinSettings()
{
  CStdString currentSkin = g_guiSettings.GetString("lookandfeel.skin") + ".";
  // clear all the settings and strings from this skin.
  std::map<int, CSkinBool>::iterator it = m_skinBools.begin();
  while (it != m_skinBools.end())
  {
    CStdString skinName = (*it).second.name;
    if (skinName.Left(currentSkin.size()) == currentSkin)
      (*it).second.value = false;

    it++;
  }
  std::map<int, CSkinString>::iterator it2 = m_skinStrings.begin();
  while (it2 != m_skinStrings.end())
  {
    CStdString skinName = (*it2).second.name;
    if (skinName.Left(currentSkin.size()) == currentSkin)
      (*it2).second.value = "";

    it2++;
  }
  g_infoManager.ResetCache();
}

void CSettings::LoadUserFolderLayout()
{
  // check them all
  CStdString strDir = g_guiSettings.GetString("system.playlistspath");
  if (strDir == "set default")
  {
    strDir = "special://profile/playlists/";
    g_guiSettings.SetString("system.playlistspath",strDir.c_str());
  }
  CDirectory::Create(strDir);
  CDirectory::Create(CUtil::AddFileToFolder(strDir,"music"));
  CDirectory::Create(CUtil::AddFileToFolder(strDir,"video"));
  CDirectory::Create(CUtil::AddFileToFolder(strDir,"mixed"));
}

CStdString CSettings::GetProfileUserDataFolder() const
{
  CStdString folder;
  if (m_iLastLoadedProfileIndex == 0)
    return GetUserDataFolder();

  CUtil::AddFileToFolder(GetUserDataFolder(),m_vecProfiles[m_iLastLoadedProfileIndex].getDirectory(),folder);

  return folder;
}

CStdString CSettings::GetUserDataItem(const CStdString& strFile) const
{
  CStdString folder;
  folder = "special://profile/"+strFile;
  if (!CFile::Exists(folder))
    folder = "special://masterprofile/"+strFile;
  return folder;
}

CStdString CSettings::GetUserDataFolder() const
{
  CStdString dir = m_vecProfiles[0].getDirectory();
#ifdef _LINUX
  dir.Replace('\\', '/');
#endif
  return dir;
}

CStdString CSettings::GetDatabaseFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Database", folder);
  else
    CUtil::AddFileToFolder(GetUserDataFolder(), "Database", folder);

  return folder;
}

CStdString CSettings::GetCDDBFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Database/CDDB", folder);
  else
    CUtil::AddFileToFolder(GetUserDataFolder(), "Database/CDDB", folder);

  return folder;
}

CStdString CSettings::GetThumbnailsFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails", folder);

  return folder;
}

CStdString CSettings::GetMusicThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Music", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Music", folder);

  return folder;
}

CStdString CSettings::GetLastFMThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Music/LastFM", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Music/LastFM", folder);

  return folder;
}

CStdString CSettings::GetMusicArtistThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Music/Artists", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Music/Artists", folder);

  return folder;
}

CStdString CSettings::GetVideoThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Video", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Video", folder);

  return folder;
}

CStdString CSettings::GetVideoFanartFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Video/Fanart", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Video/Fanart", folder);

  return folder;
}

CStdString CSettings::GetMusicFanartFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Music/Fanart", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Music/Fanart", folder);

  return folder;
}

CStdString CSettings::GetBookmarksThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Video/Bookmarks", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Video/Bookmarks", folder);

  return folder;
}

CStdString CSettings::GetPicturesThumbFolder() const
{
  CStdString folder;
  if ((int) m_vecProfiles.size() > m_iLastLoadedProfileIndex && m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Pictures", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Pictures", folder);

  return folder;
}

CStdString CSettings::GetProgramsThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/Programs", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Programs", folder);

  return folder;
}

CStdString CSettings::GetGameSaveThumbFolder() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasDatabases())
    CUtil::AddFileToFolder(g_settings.GetProfileUserDataFolder(), "Thumbnails/GameSaves", folder);
  else
    CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/GameSaves", folder);

  return folder;
}

CStdString CSettings::GetProfilesThumbFolder() const
{
  CStdString folder;
  CUtil::AddFileToFolder(g_settings.GetUserDataFolder(), "Thumbnails/Profiles", folder);

  return folder;
}

CStdString CSettings::GetSourcesFile() const
{
  CStdString folder;
  if (m_vecProfiles[m_iLastLoadedProfileIndex].hasSources())
    CUtil::AddFileToFolder(GetProfileUserDataFolder(),"sources.xml",folder);
  else
    CUtil::AddFileToFolder(GetUserDataFolder(),"sources.xml",folder);

  return folder;
}

CStdString CSettings::GetSkinFolder() const
{
  CStdString folder;

  // Get the Current Skin Path
  return GetSkinFolder(g_guiSettings.GetString("lookandfeel.skin"));
}

CStdString CSettings::GetScriptsFolder() const
{
  CStdString folder = "special://home/scripts";

  if ( CDirectory::Exists(folder) )
    return folder;

  folder = "special://xbmc/scripts";
  return folder;
}

CStdString CSettings::GetSkinFolder(const CStdString &skinName) const
{
  CStdString folder;

  // Get the Current Skin Path
  CUtil::AddFileToFolder("special://home/skin/", skinName, folder);
  if ( ! CDirectory::Exists(folder) )
    CUtil::AddFileToFolder("special://xbmc/skin/", skinName, folder);

  return folder;
}

void CSettings::LoadRSSFeeds()
{
  CStdString rssXML;
  rssXML = GetUserDataItem("RssFeeds.xml");
  TiXmlDocument rssDoc;
  if (!CFile::Exists(rssXML))
  { // set defaults, or assume no rss feeds??
    return;
  }
  if (!rssDoc.LoadFile(rssXML))
  {
    CLog::Log(LOGERROR, "Error loading %s, Line %d\n%s", rssXML.c_str(), rssDoc.ErrorRow(), rssDoc.ErrorDesc());
    return;
  }

  TiXmlElement *pRootElement = rssDoc.RootElement();
  if (!pRootElement || strcmpi(pRootElement->Value(),"rssfeeds") != 0)
  {
    CLog::Log(LOGERROR, "Error loading %s, no <rssfeeds> node", rssXML.c_str());
    return;
  }

  g_settings.m_mapRssUrls.clear();
  TiXmlElement* pSet = pRootElement->FirstChildElement("set");
  while (pSet)
  {
    int iId;
    if (pSet->QueryIntAttribute("id", &iId) == TIXML_SUCCESS)
    {
      RssSet set;
      set.rtl = pSet->Attribute("rtl") && strcasecmp(pSet->Attribute("rtl"),"true")==0;
      TiXmlElement* pFeed = pSet->FirstChildElement("feed");
      while (pFeed)
      {
        int iInterval;
        if ( pFeed->QueryIntAttribute("updateinterval",&iInterval) != TIXML_SUCCESS)
        {
          iInterval=30; // default to 30 min
          CLog::Log(LOGDEBUG,"no interval set, default to 30!");
        }
        if (pFeed->FirstChild())
        {
          // TODO: UTF-8: Do these URLs need to be converted to UTF-8?
          //              What about the xml encoding?
          CStdString strUrl = pFeed->FirstChild()->Value();
          set.url.push_back(strUrl);
          set.interval.push_back(iInterval);
        }
        pFeed = pFeed->NextSiblingElement("feed");
      }
      g_settings.m_mapRssUrls.insert(make_pair(iId,set));
    }
    else
      CLog::Log(LOGERROR,"found rss url set with no id in RssFeeds.xml, ignored");

    pSet = pSet->NextSiblingElement("set");
  }
}

CStdString CSettings::GetSettingsFile() const
{
  CStdString settings;
// Boxee
//  if (g_settings.m_iLastLoadedProfileIndex == 0)
    settings = "special://masterprofile/guisettings.xml";
//  else
//    settings = "special://profile/guisettings.xml";
// Boxee end

  return settings;
}

void CSettings::CreateProfileFolders()
{
#ifdef CANMORE
  if ((size_t) m_iLastLoadedProfileIndex <  m_vecProfiles.size())
  {
    CDirectory::Create("/tmp/profile/"+m_vecProfiles[m_iLastLoadedProfileIndex].getID());
  }
  CUtil::CreateTempDirectory("special://profile/cache");

#endif

  CDirectory::Create(GetDatabaseFolder());
  CDirectory::Create(GetCDDBFolder());

  CreateThumbnailsFolders();

  CDirectory::Create("special://profile/visualisations");
}

void CSettings::CreateThumbnailsFolders()
{
  // Thumbnails/ 
#ifdef CANMORE
  CStdString thumbFolder = GetThumbnailsFolder();
  struct stat fileStat;

  int rc = lstat(_P(thumbFolder).c_str(), &fileStat);
  if(rc == 0 && S_ISLNK(fileStat.st_mode))
  {
    CFile::Delete(thumbFolder);
    CLog::Log(LOGINFO, "%s is a soft link and needs to be deleted", _P(thumbFolder).c_str());
  }
#endif
  CDirectory::Create(GetThumbnailsFolder());
  CDirectory::Create(GetMusicThumbFolder());
  CDirectory::Create(GetMusicArtistThumbFolder());
  CDirectory::Create(GetLastFMThumbFolder());
  CDirectory::Create(GetVideoThumbFolder());
  CDirectory::Create(GetVideoFanartFolder());
  CDirectory::Create(GetMusicFanartFolder());
  CDirectory::Create(GetBookmarksThumbFolder());
  CDirectory::Create(GetProgramsThumbFolder());
  CDirectory::Create(GetPicturesThumbFolder());
  CLog::Log(LOGINFO, "  thumbnails folder:%s", GetThumbnailsFolder().c_str());
  for (unsigned int hex=0; hex < 16; hex++)
  {
    CStdString strHex;
    strHex.Format("%x",hex);
    CDirectory::Create(CUtil::AddFileToFolder(GetPicturesThumbFolder(), strHex));
    CDirectory::Create(CUtil::AddFileToFolder(GetMusicThumbFolder(), strHex));
    CDirectory::Create(CUtil::AddFileToFolder(GetVideoThumbFolder(), strHex));
  }
}

CBoxeeShortcutList& CSettings::GetShortcuts()
{
  return m_shortcuts;
}

BlackLevelType CSettings::GetBlackLevelAsEnum(const std::string& blackLevel)
{
  for (int i=BLACK_LEVEL_PC; i<NUM_OF_BLACK_LEVEL; i++)
  {
    if (blackLevelValues[i] == g_guiSettings.GetString("videoscreen.blacklevel"))
    {
      return (BlackLevelType)i;
    }
  }

  return BLACK_LEVEL_ERROR;
}

OverscanType CSettings::GetCurrentOverscan()
{
  RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
  const RESOLUTION_INFO& resolution = m_ResInfo[iRes];
  
  // Calculate average overscan
  float overscans[4];
  overscans[0] = ((float) resolution.Overscan.left) / ((float) resolution.iWidth);
  overscans[1] = ((float) resolution.Overscan.top) / ((float) resolution.iHeight);
  overscans[2] = ((float) (resolution.iWidth - resolution.Overscan.right)) / ((float) resolution.iWidth);
  overscans[3] = ((float) (resolution.iHeight - resolution.Overscan.bottom)) / ((float) resolution.iHeight);
  
  for (OverscanType i = OVERSCAN_NONE; i <= OVERSCAN_6_0; i = OverscanType(i+1))
  {
    int match = 0;
    
    for (int o = 0; o < 4; o++)
    {
      if (overscans[o] > OverscanValues[i] - 0.005f && overscans[o] < OverscanValues[i] + 0.005f)
      {
        match++;
      }
    }
    
    if (match == 4)
    {
      return i;
    }
  }
  
  return OVERSCAN_CUSTOM;
}

void CSettings::SetCurrentOverscan(OVERSCAN overscan)
{
  RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
  RESOLUTION_INFO& resolution = m_ResInfo[iRes];
  resolution.Overscan = overscan;

  // We need to copy the calibration to all matching resolutions
  for (int i = 0; i < (int)m_ResInfo.size(); i++)
  {
    RESOLUTION_INFO info = m_ResInfo[i];

    //printf("i = %d m_iCurRes = %d\n", i, iRes);

    if (i == iRes)
      continue;

    if (info.iWidth == resolution.iWidth && info.iHeight == resolution.iHeight
        && info.dwFlags == resolution.dwFlags)
    {
      //printf("For res %d %s Found matching %d %s\n", iRes,
      //    resolution.strMode.c_str(), i, info.strMode.c_str());
      m_ResInfo[i].Overscan = resolution.Overscan;
      m_ResInfo[i].iSubtitles = resolution.iSubtitles;
      m_ResInfo[i].fPixelRatio = resolution.fPixelRatio;
    }
  }

  g_settings.Save();
  // reset our screen resolution to what it was initially
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, false);
  // Inform the player so we can update the resolution
#ifdef HAS_VIDEO_PLAYBACK
  g_renderManager.Update(false);
#endif
  // and reload our fonts
  g_windowManager.SendMessage(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_WINDOW_RESIZE);   
}

void CSettings::SetCurrentOverscan(OverscanType overscan)
{
  RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
  RESOLUTION_INFO& resolution = m_ResInfo[iRes];
  float overscanPercent = OverscanValues[overscan];
  
  OVERSCAN overscanValues;
  overscanValues.left = (int) (overscanPercent * (float) resolution.iWidth);
  overscanValues.right = resolution.iWidth - (int) (overscanPercent *  (float) resolution.iWidth);
  overscanValues.top = (int) (overscanPercent * (float) resolution.iHeight);
  overscanValues.bottom = resolution.iHeight - (int) (overscanPercent * (float) resolution.iHeight);
  
  SetCurrentOverscan(overscanValues);
}

ScreenFormatType CSettings::GetCurrentScreenFormat()
{
  RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
  const RESOLUTION_INFO& resolution = m_ResInfo[iRes];
  
  for (ScreenFormatType i = SCREEN_FORMAT_4_3; i <= SCREEN_FORMAT_21_9; i = ScreenFormatType(i+1))
  {
    if (resolution.fPixelRatio > ScreenFormatValues[i] - 0.02f && resolution.fPixelRatio < ScreenFormatValues[i] + 0.02f)
    {
      return i;
    }
  }
  
  return SCREEN_FORMAT_CUSTOM;
}

void CSettings::SetCurrentScreenFormat(float pixelRatio)
{
  RESOLUTION iRes = g_graphicsContext.GetVideoResolution();
  RESOLUTION_INFO& resolution = m_ResInfo[iRes];  
  resolution.fPixelRatio = pixelRatio;
  g_settings.Save();
  // reset our screen resolution to what it was initially
  g_graphicsContext.SetVideoResolution(g_guiSettings.m_LookAndFeelResolution, false);
  // Inform the player so we can update the resolution
#ifdef HAS_VIDEO_PLAYBACK
  g_renderManager.Update(false);
#endif
  // and reload our fonts
  g_windowManager.SendMessage(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_WINDOW_RESIZE);  
}

void CSettings::SetCurrentScreenFormat(ScreenFormatType screenFormat)
{
  SetCurrentScreenFormat(ScreenFormatValues[screenFormat]);  
}

bool CSettings::UpdateServerOfProfileFiles()
{
  bool succeeded = true;
  bool retVal = true;

  succeeded = UpdateServerOfSourcesFile();

  if (!succeeded)
  {
    CLog::Log(LOGERROR,"CSettings::UpdateServerOfProfileFiles - FAILED to update server of SourcesFile (rtspf)");
    retVal &= succeeded;
  }

  succeeded = CAppManager::GetInstance().GetRepositories().UpdateServerOfRepositoriesFile();

  if (!succeeded)
  {
    CLog::Log(LOGERROR,"CSettings::UpdateServerOfProfileFiles - FAILED to update server of RepositoriesFile (rtspf)");
    retVal &= succeeded;
  }

  return retVal;
}

bool CSettings::UpdateServerOfSourcesFile()
{
  CStdString sourcesFilePath = GetSourcesFile();
  TiXmlDocument xmlDoc;
  if (!xmlDoc.LoadFile(sourcesFilePath))
  {
    CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to load [sourcesFilePath=%s]. exit and return FALSE (rtspf)",sourcesFilePath.c_str());
    return false;
  }

  TiXmlElement* pRootElement = xmlDoc.RootElement();

  if (!pRootElement)
  {
    CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to get RootElement from [sourcesFilePath=%s]. [pRootElement=%p] (rtspf)",sourcesFilePath.c_str(),pRootElement);
    return false;
  }

  if (pRootElement->Attribute("version"))
  {
    CLog::Log(LOGDEBUG,"CSettings::UpdateServerOfSourcesFile - RootElement has attribute [version] -> it is a new file [%s] -> no need to report to server (rtspf)",sourcesFilePath.c_str());
    return true;
  }

  m_fileSources.clear();
  m_musicSources.clear();
  m_pictureSources.clear();
  m_programSources.clear();
  m_videoSources.clear();

  VECSOURCES allSourcesApps;
  VECSOURCES allRssSources;

  bool succeeded = CollectSourcesForReportToServer(pRootElement, allSourcesApps, allRssSources);

  if (!succeeded)
  {
    CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to collect sources for reporting to server. exit and return FALSE (rtspf)");
    return succeeded;
  }

  CLog::Log(LOGDEBUG,"CSettings::UpdateServerOfSourcesFile - Call to CollectSourcesForReportToServer returned [allSourcesApps=%d][allRssSources=%d] (rtspf)",(int)allSourcesApps.size(),(int)allRssSources.size());

  /////////////////////////////////////
  // report to server about the apps //
  /////////////////////////////////////

  if ((int)allSourcesApps.size() > 0)
  {
    succeeded &= BoxeeUtils::ReportInstalledApps(allSourcesApps);

    if (succeeded)
    {
      CLog::Log(LOGDEBUG,"CSettings::UpdateServerOfSourcesFile - Succeeded to report sources apps to server (rtspf)");

      // after report to server -> remove apps from sources.xml file

      for(size_t i=0; i<allSourcesApps.size(); i++)
      {
        bool retVal = DeleteSource(allSourcesApps[i].m_type, allSourcesApps[i].strName, allSourcesApps[i].strPath);

        if (!retVal)
        {
          CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to delete app [name=%s][path=%s] from sources.xml (rtspf)", allSourcesApps[i].strName.c_str(), allSourcesApps[i].strPath.c_str());
        }
      }
    }
    else
    {
      CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to report sources apps to server (rtspf)");
    }
  }
  else
  {
    CLog::Log(LOGDEBUG,"CSettings::UpdateServerOfSourcesFile - [allSourcesApps=%d] -> No apps to report (rtspf)",(int)allSourcesApps.size());
  }

  ////////////////////////////////////
  // report to server about the rss //
  ////////////////////////////////////

  if (allRssSources.size() > 0)
  {
    succeeded &= BoxeeUtils::ReportInstallRss(allRssSources, true);

    if (succeeded)
    {
      CLog::Log(LOGDEBUG,"CSettings::UpdateServerOfSourcesFile - Succeeded to report sources rss to server (rtspf)");

      // after report to server -> remove rss from sources.xml file
      for(size_t i=0; i<allRssSources.size(); i++)
      {
        bool retVal = DeleteSource(allRssSources[i].m_type, allRssSources[i].strName, allRssSources[i].strPath);

        if (!retVal)
        {
          CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to delete rss [name=%s][path=%s] from sources.xml (rtspf)", allRssSources[i].strName.c_str(), allRssSources[i].strPath.c_str());
        }
      }
    }
    else
    {
      CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to report sources rss to server (rtspf)");
    }
  }
  else
  {
    CLog::Log(LOGDEBUG,"CSettings::UpdateServerOfSourcesFile - [allRssSources=%d] -> No rss to report (rtspf)", (int)allRssSources.size());
  }

  if (succeeded)
  {
    // reload the sources.xml file for edit
    if (!xmlDoc.LoadFile(sourcesFilePath))
    {
      CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to reload [sourcesFilePath=%s] (rtspf)",sourcesFilePath.c_str());
    }
    else
    {
      TiXmlElement* pRootElement = xmlDoc.RootElement();

      if (pRootElement)
      {
        pRootElement->SetAttribute("version",1);
        xmlDoc.SaveFile(sourcesFilePath);
      }
      else
      {
        CLog::Log(LOGERROR,"CSettings::UpdateServerOfSourcesFile - FAILED to get RootElement from [sourcesFilePath=%s] in reload. [pRootElement=%p] (rtspf)",sourcesFilePath.c_str(),pRootElement);
      }
    }
  }

  return succeeded;
}

bool CSettings::CollectSourcesForReportToServer(TiXmlElement* pRootElement, VECSOURCES& allSourcesApps, VECSOURCES& allRssSources)
{
  if (!pRootElement)
  {
    CLog::Log(LOGERROR,"CSettings::CollectSourcesForReportToServer - Enter function with [pRootElement=NULL]. exit and return FALSE (rtspf)");
    return false;
  }

  GetSources(pRootElement, "pictures", m_pictureSources, m_defaultPictureSource);
  GetSources(pRootElement, "music", m_musicSources, m_defaultMusicSource);
  GetSources(pRootElement, "video", m_videoSources, m_defaultVideoSource);

  // collect video apps and rss
  CollectSourcesForReportFromVec(m_videoSources, allSourcesApps, allRssSources);

  // collect music apps and rss
  CollectSourcesForReportFromVec(m_musicSources, allSourcesApps, allRssSources);

  // collect pictures apps and rss
  CollectSourcesForReportFromVec(m_pictureSources, allSourcesApps, allRssSources);

  return true;
}

void CSettings::CollectSourcesForReportFromVec(VECSOURCES sourcesFromTypeVec, VECSOURCES& allSourcesApps, VECSOURCES& allRssSources)
{
  for(size_t i=0; i<sourcesFromTypeVec.size(); i++)
  {
    if (CUtil::IsApp(sourcesFromTypeVec[i].strPath))
    {
      allSourcesApps.push_back(sourcesFromTypeVec[i]);
    }
    else if (CUtil::IsLastFM(sourcesFromTypeVec[i].strPath))
    {
      allSourcesApps.push_back(sourcesFromTypeVec[i]);
    }
    else if (CUtil::IsShoutCast(sourcesFromTypeVec[i].strPath))
    {
      allSourcesApps.push_back(sourcesFromTypeVec[i]);
    }
    else if ((sourcesFromTypeVec[i].strPath).Left(6) == "rss://")
    {
      allRssSources.push_back(sourcesFromTypeVec[i]);
    }
    else
    {
      CLog::Log(LOGDEBUG,"CSettings::CollectSourcesForReportFromVec - [path=%s] of [type=%s] is UNKNOWN. Path and won't be collect (rtspf)",sourcesFromTypeVec[i].strPath.c_str(),sourcesFromTypeVec[i].m_type.c_str());
    }
  }
}

bool CSettings::LoadAdditionalSettings()
{
  /////////////////////////////
  // Load Subtitles Language //
  /////////////////////////////

  for (int i=57202;i<=BOXEE::BXUtils::StringToInt(g_localizeStrings.Get(57201));i++)
  {
    CStdString lang = g_localizeStrings.Get(i);
    vector<CStdString> tokens;
    CUtil::Tokenize(lang,tokens,",");
    if ((int)tokens.size()<2)
    {
      CLog::Log(LOGERROR,"CSettings::LoadAdditionalSettings - LoadSubtitlesLanguage - FAILED to find comma in [%d=%s] (sl)",i,lang.c_str());
      continue;
    }
    m_subtitleLangsVec.push_back(tokens[0]);
    m_subtitleLangToCodeMap[tokens[0]] = tokens[1];
  }
  sort(m_subtitleLangsVec.begin(), m_subtitleLangsVec.end(), sortstringbyname());

  m_subtitleLangsVec.insert(m_subtitleLangsVec.begin(),g_localizeStrings.Get(57200));
  m_subtitleLangToCodeMap[g_localizeStrings.Get(57200)] = "def";

  return true;
}

//
// internal class - remove the share in background process
//

CSettings::SettingDeleteShare::SettingDeleteShare(CSettings* settingHandler,const CStdString &strType, const CStdString strName, const CStdString strPath, bool virtualSource,  unsigned long executionDelayInMS, bool repeat)
  :BoxeeScheduleTask("HttpCacheManagerCleanup",executionDelayInMS,repeat)
{
  m_settingHandler = settingHandler;
  m_strType         = strType;
  m_strName         = strName;
  m_strPath         = strPath;
  m_virtualSource   = virtualSource;
}

CSettings::SettingDeleteShare::~SettingDeleteShare()
{

}

void CSettings::SettingDeleteShare::DoWork()
{
  CLog::Log(LOGDEBUG,"CSettings::SettingDeleteShare::DoWork");
  m_settingHandler->DeleteSource(m_strType, m_strName, m_strPath, m_virtualSource);
}

