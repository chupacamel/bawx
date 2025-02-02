#pragma once
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

#ifdef HAS_LASTFM

#include "utils/CriticalSection.h"
#include "utils/Thread.h"
#include "URL.h"
#include "PlayListPlayer.h"

namespace PLAYLIST
{
  class CPlayList;
}

namespace MUSIC_INFO
{
  class CMusicInfoTag;
}

class CGUIDialogProgress;

class CLastFmManager : CThread
{
public:
  virtual ~CLastFmManager();
  static void RemoveInstance();
  static CLastFmManager* GetInstance();

  //radio
  bool ChangeStation(const CURI& stationUrl);
  void StopRadio(bool bKillSession = true);
  void OnSongChange(CFileItem& newSong);
  bool IsRadioEnabled() {return !m_RadioSession.IsEmpty();}
  bool IsLastFmEnabled();
  bool Love(bool askConfirmation = true);
  bool Ban(bool askConfirmation = true);
  bool CanLove();
  bool CanBan();
  bool Unlove(const MUSIC_INFO::CMusicInfoTag& musicinfotag, bool askConfirmation = true);
  bool Unban(const MUSIC_INFO::CMusicInfoTag& musicinfotag, bool askConfirmation = true);

  CStdString GetStation();
  CURI       GetStationURL();

  bool LastFmHandShake();
  
  static void ShowLastFMSettings(void);

  //scrobbler
  bool CanScrobble(const CFileItem &fileitem);
protected:
  virtual void Process();
  virtual void OnStartup();

private:
  typedef struct
  {
    CFileItem* CurrentSong;
    bool IsLoved;
    bool IsBanned;
  } LastFmManagerSong;

  CLastFmManager();
  static CLastFmManager* m_pInstance;

  bool RadioHandShake();
  bool RequestRadioTracks();
  void CacheTrackThumb(const int nrInitialTracksToAdd);
  void Parameter(const CStdString& key, const CStdString& data, CStdString& value);
  bool ReapSongs();
  bool MovePlaying();
  void SendUpdateMessage();
  void Update();
  void AddToPlaylist(const int nrTracks);
  bool CallXmlRpc(const CStdString& action, const CStdString& artist, const CStdString& title);
  void CreateMD5Hash(const CStdString& bufferToHash, CStdString& hash);
  bool Love(const MUSIC_INFO::CMusicInfoTag& musicinfotag);
  bool Ban(const MUSIC_INFO::CMusicInfoTag& musicinfotag);

  CStdString m_RadioUrl;
  CStdString m_RadioSession;
  CStdString m_RadioBaseUrl;
  CStdString m_RadioBasePath;
  CStdString m_RadioSubscriber;
  CStdString m_RadioBanned;

  CURI m_stationUrl;

  LastFmManagerSong m_CurrentSong;

  PLAYLIST::CPlayList* m_RadioTrackQueue;
  HANDLE m_hWorkerEvent;
  CCriticalSection m_lockCache;
  CCriticalSection m_lockPlaylist;

  CGUIDialogProgress* m_dlgProgress;
  void InitProgressDialog(const CStdString& strUrl);
  void UpdateProgressDialog(const int iStringID);
  void CloseProgressDialog();
  
  CCriticalSection m_LastFmHandShakelock;
};

#endif
