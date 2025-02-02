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
#include "GUIWindowFullScreen.h"
#include "Application.h"
#include "Util.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#include "utils/GUIInfoManager.h"
#include "GUIProgressControl.h"
#include "GUIAudioManager.h"
#include "GUILabelControl.h"
#include "GUIWindowOSD.h"
#include "GUIFontManager.h"
#include "GUITextLayout.h"
#include "GUIWindowManager.h"
#include "GUIDialogFullScreenInfo.h"
#include "GUIDialogAudioSubtitleSettings.h"
#include "GUIDialogNumeric.h"
#include "GUISliderControl.h"
#include "Settings.h"
#include "FileItem.h"
#include "VideoReferenceClock.h"
#include "AdvancedSettings.h"
#include "CPUInfo.h"
#include "GUISettings.h"
#include "MouseStat.h"
#include "LocalizeStrings.h"
#include "utils/SingleLock.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"
#include "GUIDialogBoxeeExitVideo.h"
#include "Builtins.h"

#include <stdio.h>

#include "GUIDialogBoxeeSeekBar.h"

#ifdef HAS_INTEL_SMD
#include "IntelSMDGlobals.h"
#endif

#ifdef HAS_EMBEDDED
#include "ItemLoader.h"
#endif

#ifndef DVD_NOPTS_VALUE
#define DVD_NOPTS_VALUE    (-1LL<<52) // should be possible to represent in both double and __int64
#endif

#define BLUE_BAR                          0
#define LABEL_ROW1                       10
#define LABEL_ROW2                       11
#define LABEL_ROW3                       12

#define BTN_OSD_VIDEO                    13
#define BTN_OSD_AUDIO                    14
#define BTN_OSD_SUBTITLE                 15

#define MENU_ACTION_AVDELAY               1
#define MENU_ACTION_SEEK                  2
#define MENU_ACTION_SUBTITLEDELAY         3
#define MENU_ACTION_SUBTITLEONOFF         4
#define MENU_ACTION_SUBTITLELANGUAGE      5
#define MENU_ACTION_INTERLEAVED           6
#define MENU_ACTION_FRAMERATECONVERSIONS  7
#define MENU_ACTION_AUDIO_STREAM          8

#define MENU_ACTION_NEW_BOOKMARK          9
#define MENU_ACTION_NEXT_BOOKMARK        10
#define MENU_ACTION_CLEAR_BOOKMARK       11

#define MENU_ACTION_NOCACHE              12

#define IMG_PAUSE                        16
#define IMG_2X                           17
#define IMG_4X                           18
#define IMG_8X                           19
#define IMG_16X                          20
#define IMG_32X                          21

#define IMG_2Xr                         117
#define IMG_4Xr                         118
#define IMG_8Xr                         119
#define IMG_16Xr                        120
#define IMG_32Xr                        121

//Displays current position, visible after seek or when forced
//Alt, use conditional visibility Player.DisplayAfterSeek
#define LABEL_CURRENT_TIME               22

//Displays when video is rebuffering
//Alt, use conditional visibility Player.IsCaching
#define LABEL_BUFFERING                  24

//Progressbar used for buffering status and after seeking
#define CONTROL_PROGRESS                 23

#define CONTROL_PAUSE_INDICATOR          9000

#define SHOW_SUDIO_CODEC_LOGO_IN_MS      5000

#ifdef __APPLE__
static CLinuxResourceCounter m_resourceCounter;
#endif

static color_t color[6] = { 0xFFFFFF00, 0xFFFFFFFF, 0xFF0099FF, 0xFF00FF00, 0xFFCCFF00, 0xFF00FFFF };

static CRect flashSrc;
static CRect flashDst;

CGUIWindowFullScreen::CGUIWindowFullScreen(void)
    : CGUIWindow(WINDOW_FULLSCREEN_VIDEO, "VideoFullScreen.xml")
{
  m_timeCodeStamp[0] = 0;
  m_timeCodePosition = 0;
  m_timeCodeShow = false;
  m_timeCodeTimeout = 0;
  m_bShowViewModeInfo = false;
  m_dwShowViewModeTimeout = 0;
  m_bShowCurrentTime = false;
  m_subsLayout = NULL;
  // audio
  //  - language
  //  - volume
  //  - stream

  // video
  //  - Create Bookmark (294)
  //  - Cycle bookmarks (295)
  //  - Clear bookmarks (296)
  //  - jump to specific time
  //  - slider
  //  - av delay

  // subtitles
  //  - delay
  //  - language

}

CGUIWindowFullScreen::~CGUIWindowFullScreen(void)
{}

void CGUIWindowFullScreen::PreloadDialog(unsigned int windowID)
{
  CGUIWindow *pWindow = g_windowManager.GetWindow(windowID);
  if (pWindow)
  {
    pWindow->Initialize();
    pWindow->DynamicResourceAlloc(false);
    pWindow->AllocResources(false);
  }
}

void CGUIWindowFullScreen::OnDeinitWindow(int nextWindow)
{
  g_windowManager.CloseDialogs(true);
}

void CGUIWindowFullScreen::UnloadDialog(unsigned int windowID)
{
  CGUIWindow *pWindow = g_windowManager.GetWindow(windowID);
  if (pWindow) {
    pWindow->FreeResources(pWindow->GetLoadOnDemand());
  }
}

void CGUIWindowFullScreen::ShowOSD(CSeekDirection::SeekDirectionEnums seekDirection)
{
  CGUIDialogBoxeeVideoCtx* pDlgInfo = (CGUIDialogBoxeeVideoCtx*)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_VIDEO_CTX);
  const CFileItem &item = g_application.CurrentFileItem();
  if (pDlgInfo)
  {
    pDlgInfo->SetItem(item);
    pDlgInfo->SetSeekDirectionOnOpen(seekDirection);
    pDlgInfo->DoModal();
  }
}

void CGUIWindowFullScreen::AllocResources(bool forceLoad)
{
  CGUIWindow::AllocResources(forceLoad);
  DynamicResourceAlloc(false);
  PreloadDialog(WINDOW_OSD);
  PreloadDialog(WINDOW_DIALOG_VIDEO_OSD_SETTINGS);
  PreloadDialog(WINDOW_DIALOG_AUDIO_OSD_SETTINGS);
  PreloadDialog(WINDOW_DIALOG_SUBTITLE_OSD_SETTINGS);
  PreloadDialog(WINDOW_DIALOG_BOXEE_VIDEO_CTX);

  // No need to preload these here, as they're preloaded by our app
//  PreloadDialog(WINDOW_DIALOG_SEEK_BAR);
//  PreloadDialog(WINDOW_DIALOG_VOLUME_BAR);
//  PreloadDialog(WINDOW_DIALOG_MUTE_BUG);
}

void CGUIWindowFullScreen::FreeResources(bool forceUnload)
{
  g_settings.Save();
  DynamicResourceAlloc(true);
  UnloadDialog(WINDOW_OSD);
  UnloadDialog(WINDOW_DIALOG_VIDEO_OSD_SETTINGS);
  UnloadDialog(WINDOW_DIALOG_AUDIO_OSD_SETTINGS);
  UnloadDialog(WINDOW_DIALOG_BOXEE_VIDEO_CTX);

  // No need to unload these here, as they're preloaded by our app
//  UnloadDialog(WINDOW_DIALOG_SEEK_BAR);
//  UnloadDialog(WINDOW_DIALOG_VOLUME_BAR);
//  UnloadDialog(WINDOW_DIALOG_MUTE_BUG);
  CGUIWindow::FreeResources(forceUnload);
}

bool CGUIWindowFullScreen::OnAction(const CAction &action)
{
  if (g_application.m_pPlayer != NULL && g_application.m_pPlayer->OnAction(action))
    return true;
  
  switch (action.id)
  {
  case ACTION_MOVE_UP:
  case ACTION_MOVE_DOWN:
  case ACTION_SHOW_INFO:
  {
    //as requested in http://jira.boxee.tv/browse/BOXEE-9125 -- begin
    ShowOSD();
    return true;
    //as requested in http://jira.boxee.tv/browse/BOXEE-9125 -- end

//Boxee
    /*bool showOSD = g_application.CurrentFileItem().GetPropertyBOOL("browsercansetfullscreen");
    if (showOSD)
    { //if we're in the browser, show the OSD
      ShowOSD();
      return true;
    }
    else*/

    /* //un comment to revert http://jira.boxee.tv/browse/BOXEE-9125 -- begin
    { //if we're in playback, act as play/pause
      CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - TogglePlayPause (ev)");
      g_application.TogglePlayPause();

      //if (g_application.IsPaused())
        //ShowOSD();

      return true;
    }*/ //un comment to revert http://jira.boxee.tv/browse/BOXEE-9125 -- end
//end Boxee
  }
  break;
  case ACTION_PARENT_DIR:
  case ACTION_SHOW_GUI:
  {
    //show exit dialog if browser is active
    /*bool showExitDialog = g_application.CurrentFileItem().GetPropertyBOOL("browsercansetfullscreen");
    CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - [browsercansetfullscreen=%d] (ev)",g_application.CurrentFileItem().GetPropertyBOOL("browsercansetfullscreen"));
    if (showExitDialog)
    {
      if (!CGUIDialogBoxeeExitVideo::ShowAndGetInput())
      {
        CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Call to CGUIDialogBoxeeExitVideo return FALSE. return to video. (ev)");
        return true;
      }

      CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Call to CGUIDialogBoxeeExitVideo return TRUE. Going to stop the video (ev)");
      g_application.StopPlaying();
      // switch back to the menu
      g_windowManager.PreviousWindow();
      return true;
    }
    else*//*  //uncomment the code below to revert http://jira.boxee.tv/browse/BOXEE-9125 -- begin
    {
      CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Showing OSD. (ev)");
      ShowOSD();
      return true;
              //uncomment the code below to revert http://jira.boxee.tv/browse/BOXEE-9125 -- begin
    }*/

    //as requested in http://jira.boxee.tv/browse/BOXEE-9125 -- begin
    bool showExitDialog = (g_application.CurrentFileItem().GetPropertyBOOL("browsercansetfullscreen") || g_guiSettings.GetBool("myvideos.showmessagewhenexit"));
    CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - [showExitDialog=%d]. [browsercansetfullscreen=%d][showmessagewhenexit=%d] (ev)",showExitDialog,g_application.CurrentFileItem().GetPropertyBOOL("browsercansetfullscreen"),g_guiSettings.GetBool("myvideos.showmessagewhenexit"));

    CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - [showExitDialog=%d]. [browsercansetfullscreen=%d] (ev)",showExitDialog,g_application.CurrentFileItem().GetPropertyBOOL("browsercansetfullscreen"));

    if (showExitDialog)
    {
      if (!CGUIDialogBoxeeExitVideo::ShowAndGetInput())
      {
        CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Call to CGUIDialogBoxeeExitVideo return FALSE. return to video. [ShowMessageWhenExit=%d] (ev)",g_guiSettings.GetBool("myvideos.showmessagewhenexit"));
        return true;
      }

      CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Call to CGUIDialogBoxeeExitVideo return TRUE. [ShowMessageWhenExit=%d] (ev)",g_guiSettings.GetBool("myvideos.showmessagewhenexit"));
    }
    else
    {
      CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Not showing exit dialog. [ShowMessageWhenExit=%d] (ev)",g_guiSettings.GetBool("myvideos.showmessagewhenexit"));
    }

    CLog::Log(LOGDEBUG,"CGUIWindowFullScreen::OnAction - Going to stop the video (ev)");
    g_application.StopPlaying();

    // switch back to the menu
    g_windowManager.PreviousWindow();
    return true;
    //as requested in http://jira.boxee.tv/browse/BOXEE-9125 -- end
  }
  break;
  case ACTION_STEP_BACK:
    Seek(false, false);
    return true;
    break;

  case ACTION_STEP_FORWARD:
    Seek(true, false);
    return true;
    break;

  case ACTION_BIG_STEP_BACK:
    Seek(false, true);
    return true;
    break;

  case ACTION_BIG_STEP_FORWARD:
    Seek(true, true);
    return true;
    break;

  case ACTION_NEXT_SCENE:
    if (g_application.m_pPlayer->SeekScene(true))
      g_infoManager.SetDisplayAfterSeek();
    return true;
    break;

  case ACTION_PREV_SCENE:
    if (g_application.m_pPlayer->SeekScene(false))
      g_infoManager.SetDisplayAfterSeek();
    return true;
    break;

  case ACTION_SHOW_OSD_TIME:
    m_bShowCurrentTime = !m_bShowCurrentTime;
    if(!m_bShowCurrentTime)
      g_infoManager.SetDisplayAfterSeek(0); //Force display off
    g_infoManager.SetShowTime(m_bShowCurrentTime);
    return true;
    break;

  case ACTION_SHOW_SUBTITLES:
    {
      g_stSettings.m_currentVideoSettings.m_SubtitleOn = !g_stSettings.m_currentVideoSettings.m_SubtitleOn;
      g_application.m_pPlayer->SetSubtitleVisible(g_stSettings.m_currentVideoSettings.m_SubtitleOn);
      if (!g_stSettings.m_currentVideoSettings.m_SubtitleCached && g_stSettings.m_currentVideoSettings.m_SubtitleOn)
      {
        g_application.Restart(true); // cache subtitles
        Close();
    }
    }
    return true;
    break;

  case ACTION_NEXT_SUBTITLE:
    {
      if (g_application.m_pPlayer->GetSubtitleCount() == 1)
        return true;

      g_stSettings.m_currentVideoSettings.m_SubtitleStream++;
      if (g_stSettings.m_currentVideoSettings.m_SubtitleStream >= g_application.m_pPlayer->GetSubtitleCount())
        g_stSettings.m_currentVideoSettings.m_SubtitleStream = 0;
      g_application.m_pPlayer->SetSubtitle(g_stSettings.m_currentVideoSettings.m_SubtitleStream);
      return true;
    }
    return true;
    break;

  case ACTION_SUBTITLE_DELAY_MIN:
    g_stSettings.m_currentVideoSettings.m_SubtitleDelay -= 0.1f;
    if (g_stSettings.m_currentVideoSettings.m_SubtitleDelay < -g_advancedSettings.m_videoSubsDelayRange)
      g_stSettings.m_currentVideoSettings.m_SubtitleDelay = -g_advancedSettings.m_videoSubsDelayRange;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->SetSubTitleDelay(g_stSettings.m_currentVideoSettings.m_SubtitleDelay);

    CGUIDialogSlider::Display(22006, g_stSettings.m_currentVideoSettings.m_SubtitleDelay,
                                    -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                     g_advancedSettings.m_videoSubsDelayRange, this);

    return true;
    break;
  case ACTION_SUBTITLE_DELAY_PLUS:
    g_stSettings.m_currentVideoSettings.m_SubtitleDelay += 0.1f;
    if (g_stSettings.m_currentVideoSettings.m_SubtitleDelay > g_advancedSettings.m_videoSubsDelayRange)
      g_stSettings.m_currentVideoSettings.m_SubtitleDelay = g_advancedSettings.m_videoSubsDelayRange;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->SetSubTitleDelay(g_stSettings.m_currentVideoSettings.m_SubtitleDelay);

    CGUIDialogSlider::Display(22006, g_stSettings.m_currentVideoSettings.m_SubtitleDelay,
                                    -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                     g_advancedSettings.m_videoSubsDelayRange, this);
    return true;
    break;
  case ACTION_SUBTITLE_DELAY:
    CGUIDialogSlider::ShowAndGetInput(g_localizeStrings.Get(22006), g_stSettings.m_currentVideoSettings.m_SubtitleDelay,
                                                                   -g_advancedSettings.m_videoSubsDelayRange, 0.1f,
                                                                    g_advancedSettings.m_videoSubsDelayRange, this, (void *)&action.id);
    return true;
    break;
  case ACTION_AUDIO_DELAY:
    CGUIDialogSlider::ShowAndGetInput(g_localizeStrings.Get(297), g_stSettings.m_currentVideoSettings.m_AudioDelay,
                                                                 -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                                                  g_advancedSettings.m_videoAudioDelayRange, this, (void *)&action.id);
    return true;
    break;
  case ACTION_AUDIO_DELAY_MIN:
    g_stSettings.m_currentVideoSettings.m_AudioDelay -= 0.025f;
    if (g_stSettings.m_currentVideoSettings.m_AudioDelay < -g_advancedSettings.m_videoAudioDelayRange)
      g_stSettings.m_currentVideoSettings.m_AudioDelay = -g_advancedSettings.m_videoAudioDelayRange;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->SetAVDelay(g_stSettings.m_currentVideoSettings.m_AudioDelay);

    CGUIDialogSlider::Display(297, g_stSettings.m_currentVideoSettings.m_AudioDelay,
                                  -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                   g_advancedSettings.m_videoAudioDelayRange, this);
    return true;
    break;
  case ACTION_AUDIO_DELAY_PLUS:
    g_stSettings.m_currentVideoSettings.m_AudioDelay += 0.025f;
    if (g_stSettings.m_currentVideoSettings.m_AudioDelay > g_advancedSettings.m_videoAudioDelayRange)
      g_stSettings.m_currentVideoSettings.m_AudioDelay = g_advancedSettings.m_videoAudioDelayRange;
    if (g_application.m_pPlayer)
      g_application.m_pPlayer->SetAVDelay(g_stSettings.m_currentVideoSettings.m_AudioDelay);

    CGUIDialogSlider::Display(297, g_stSettings.m_currentVideoSettings.m_AudioDelay,
                                  -g_advancedSettings.m_videoAudioDelayRange, 0.025f,
                                   g_advancedSettings.m_videoAudioDelayRange, this);
    return true;
    break;
  case ACTION_AUDIO_NEXT_LANGUAGE:
      if (g_application.m_pPlayer->GetAudioStreamCount() == 1)
        return true;

      g_stSettings.m_currentVideoSettings.m_AudioStream++;
      if (g_stSettings.m_currentVideoSettings.m_AudioStream >= g_application.m_pPlayer->GetAudioStreamCount())
        g_stSettings.m_currentVideoSettings.m_AudioStream = 0;
      g_application.m_pPlayer->SetAudioStream(g_stSettings.m_currentVideoSettings.m_AudioStream);    // Set the audio stream to the one selected
    return true;
    break;
  case REMOTE_0:
  case REMOTE_1:
  case REMOTE_2:
  case REMOTE_3:
  case REMOTE_4:
  case REMOTE_5:
  case REMOTE_6:
  case REMOTE_7:
  case REMOTE_8:
  case REMOTE_9:
    {
      if (g_application.CurrentFileItem().IsLiveTV())
      {
        int channelNr = -1;

        CStdString strChannel;
        strChannel.Format("%i", action.id - REMOTE_0);
        if (CGUIDialogNumeric::ShowAndGetNumber(strChannel, g_localizeStrings.Get(19000)))
          channelNr = atoi(strChannel.c_str());

        if (channelNr > 0)
        {
          CAction action;
          action.id = ACTION_CHANNEL_SWITCH;
          action.amount1 = (float)channelNr;
          OnAction(action);
        }
      }
      else
      {
        ChangetheTimeCode(action.id);
      }
    return true;
    }
    break;

  case ACTION_ASPECT_RATIO:
    { // toggle the aspect ratio mode (only if the info is onscreen)
      if (m_bShowViewModeInfo)
      {
#ifdef HAS_VIDEO_PLAYBACK
        g_renderManager.SetViewMode(++g_stSettings.m_currentVideoSettings.m_ViewMode);
#endif
      }
      m_bShowViewModeInfo = true;
      m_dwShowViewModeTimeout = CTimeUtils::GetTimeMS();
    }
    return true;
    break;
  case ACTION_SMALL_STEP_BACK:
    {
      int orgpos = (int)g_application.GetTime();
      int jumpsize = g_advancedSettings.m_videoSmallStepBackSeconds; // secs
      int setpos = (orgpos > jumpsize) ? orgpos - jumpsize : 0;
        g_application.SeekTime((double)setpos);

      //Make sure gui items are visible
      g_infoManager.SetDisplayAfterSeek();
    }
    return true;
    break;
  }
  return CGUIWindow::OnAction(action);
}

void CGUIWindowFullScreen::OnWindowLoaded()
{
  CGUIWindow::OnWindowLoaded();

  CGUIProgressControl* pProgress = (CGUIProgressControl*)GetControl(CONTROL_PROGRESS);
  if(pProgress)
  {
    if( pProgress->GetInfo() == 0 || pProgress->GetVisibleCondition() == 0)
    {
      pProgress->SetInfo(PLAYER_PROGRESS);
      pProgress->SetVisibleCondition(PLAYER_DISPLAY_AFTER_SEEK, false);
      pProgress->SetVisible(true);
    }
  }

  CGUILabelControl *pLabel = (CGUILabelControl*)GetControl(LABEL_CURRENT_TIME);
  if(pLabel && pLabel->GetVisibleCondition() == 0)
  {
    pLabel->SetVisibleCondition(PLAYER_DISPLAY_AFTER_SEEK, false);
    pLabel->SetVisible(true);
    pLabel->SetLabel("$INFO(VIDEOPLAYER.TIME) / $INFO(VIDEOPLAYER.DURATION)");
  }
}

bool CGUIWindowFullScreen::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_WINDOW_INIT:
    {
      // check whether we've come back here from a window during which time we've actually
      // stopped playing videos
      if (message.GetParam1() == WINDOW_INVALID && !g_application.IsPlayingVideo())
      { // why are we here if nothing is playing???
        g_windowManager.PreviousWindow();
        return true;
      }
      m_bLastRender = false;
      g_infoManager.SetShowInfo(false);
      g_infoManager.SetShowCodec(false);
      m_bShowCurrentTime = false;
      g_infoManager.SetDisplayAfterSeek(0); // Make sure display after seek is off.

#ifdef HAS_XBOX_HARDWARE
      //  Disable nav sounds if spindown is active as they are loaded
      //  from HDD all the time.
      if (
        !g_application.CurrentFileItem().IsHD() &&
        (g_guiSettings.GetInt("harddisk.remoteplayspindown") || g_guiSettings.GetInt("harddisk.spindowntime"))
      )
      {
        if (!g_guiSettings.GetBool("lookandfeel.soundsduringplayback"))
          g_audioManager.Enable(false);
      }
#endif

      // setup the brightness, contrast and resolution
      CUtil::SetBrightnessContrastGammaPercent(g_stSettings.m_currentVideoSettings.m_Brightness, g_stSettings.m_currentVideoSettings.m_Contrast, g_stSettings.m_currentVideoSettings.m_Gamma, false);

      // switch resolution
      g_graphicsContext.SetFullScreenVideo(true);

#ifdef HAS_EMBEDDED
      g_Windowing.ClearBuffers(0, 0, 0, 1.0);
#endif

#ifdef HAS_VIDEO_PLAYBACK
      // make sure renderer is uptospeed
      g_renderManager.Update(false);
#endif
      // now call the base class to load our windows
      CGUIWindow::OnMessage(message);

      m_bShowViewModeInfo = false;

      if (CUtil::IsUsingTTFSubtitles())
      {
        CSingleLock lock (m_fontLock);

        CStdString fontPath = "special://xbmc/media/Fonts/";
#ifndef HAS_EMBEDDED
        fontPath += g_guiSettings.GetString("subtitles.font");
#else
        CStdString charset = g_guiSettings.GetString("subtitles.charset");
        if(charset.IsEmpty() || charset == "DEFAULT")
        {
          charset = "CP1250";
        }
        fontPath += g_charsetConverter.getSubtitleFontByCharsetName(charset);
#endif

        // We scale based on PAL4x3 - this at least ensures all sizing is constant across resolutions.
        // it doesn't preserve aspect, however, so make sure we choose aspect as 1/scalingpixelratio
        g_graphicsContext.SetSkinResolution(RES_PAL_4x3);
        float aspect = 1.0f / g_graphicsContext.GetScalingPixelRatio();
        CGUIFont *subFont = g_fontManager.LoadTTF("__subtitle__", fontPath, color[g_guiSettings.GetInt("subtitles.color")], 0, g_guiSettings.GetInt("subtitles.height"), g_guiSettings.GetInt("subtitles.style"), 1.0f, aspect, RES_PAL_4x3);
        g_graphicsContext.SetSkinResolution(RES_HDTV_720p);
        if (!subFont)
          CLog::Log(LOGERROR, "CGUIWindowFullScreen::OnMessage(WINDOW_INIT) - Unable to load subtitle font");
        else
          m_subsLayout = new CGUITextLayout(subFont, true);
      }
      else
        m_subsLayout = NULL;

      return true;
    }
  case GUI_MSG_WINDOW_DEINIT:
    {
      CGUIWindow::OnMessage(message);

      CGUIDialog *pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_OSD_TELETEXT);
      if (pDialog) pDialog->Close(true);
      CGUIDialogSlider *slider = (CGUIDialogSlider *)g_windowManager.GetWindow(WINDOW_DIALOG_SLIDER);
      if (slider) slider->Close(true);
      pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_OSD);
      if (pDialog) pDialog->Close(true);
      pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_FULLSCREEN_INFO);
      if (pDialog) pDialog->Close(true);
      pDialog = (CGUIDialog *)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_VIDEO_CTX);
      if (pDialog) pDialog->Close(true);

      FreeResources(true);

      CSingleLock lock (g_graphicsContext);
      CUtil::RestoreBrightnessContrastGamma();
      g_graphicsContext.SetFullScreenVideo(false);
      lock.Leave();

#ifdef HAS_VIDEO_PLAYBACK
      // make sure renderer is uptospeed
      g_renderManager.Update(false);
#endif

      CSingleLock lockFont(m_fontLock);
      if (m_subsLayout)
      {
        g_fontManager.Unload("__subtitle__");
        delete m_subsLayout;
        m_subsLayout = NULL;
      }

      return true;
    }
  case GUI_MSG_SETFOCUS:
  case GUI_MSG_LOSTFOCUS:
    if (message.GetSenderId() != WINDOW_FULLSCREEN_VIDEO) return true;
    break;
  }

  return CGUIWindow::OnMessage(message);
}

bool CGUIWindowFullScreen::OnMouse(const CPoint &point)
{
  if (g_Mouse.bClick[MOUSE_RIGHT_BUTTON])
  { // no control found to absorb this click - go back to GUI
    CAction action;
    action.id = ACTION_SHOW_GUI;
    OnAction(action);
    return true;
  }
  if (g_Mouse.bClick[MOUSE_LEFT_BUTTON])
  { // no control found to absorb this click - pause video
    CAction action;
    action.id = ACTION_PAUSE;
    return g_application.OnAction(action);
  }
  
  if (g_Mouse.HasMoved())
  { // movement - toggle the OSD
    CGUIDialogBoxeeVideoCtx *pOSD = (CGUIDialogBoxeeVideoCtx *)g_windowManager.GetWindow(WINDOW_DIALOG_BOXEE_VIDEO_CTX);
    if (pOSD)
    {
      const CFileItem &item = g_application.CurrentFileItem();      
      pOSD->SetItem(item);
      pOSD->SetAutoClose(3000);
      pOSD->DoModal();
    }
  }

  if (g_Mouse.GetWheel())
  { // Mouse wheel
    int wheel = abs(g_Mouse.GetWheel());
    CAction action;
    action.amount1 = 0.5f * (float)wheel;
    action.id = g_Mouse.GetWheel() > 0 ? ACTION_ANALOG_SEEK_FORWARD : ACTION_ANALOG_SEEK_BACK;
    return g_application.OnAction(action);
  }
  return true;
}

// Override of Render() - RenderFullScreen() is where the action takes place
// this is called from the rendermanager, normally we won't come this way
// as player thread will handle rendering, and call this itself.
void CGUIWindowFullScreen::Render()
{
#ifdef HAS_VIDEO_PLAYBACK
  g_renderManager.RenderUpdate(true);
#endif
#ifndef HAS_XBOX_HARDWARE
  // win32 video rendering uses this path all the time (it doesn't render from the player directly)
  // so at this point we should renderfullscreen info as well.
  if (NeedRenderFullScreen())
    RenderFullScreen();
#endif
}

bool CGUIWindowFullScreen::NeedRenderFullScreen()
{
  CSingleLock lock (g_graphicsContext);
  if (g_application.m_pPlayer)
  {
    if (g_application.m_pPlayer->IsPaused() ) return true;
    if (g_application.m_pPlayer->IsCaching() ) return true;
    if (!g_application.m_pPlayer->IsPlaying() ) return true;
  }
  if (g_application.GetPlaySpeed() != 1) return true;
  if (m_timeCodeShow) return true;
  if (g_infoManager.GetBool(PLAYER_SHOWCODEC)) return true;
  if (g_infoManager.GetBool(PLAYER_SHOWINFO)) return true;
  if (g_infoManager.GetBool(PLAYER_SHOWAUDIOCODECLOGO)) return true;
  if (IsAnimating(ANIM_TYPE_HIDDEN)) return true; // for the above info conditions
  if (m_bShowViewModeInfo) return true;
  if (m_bShowCurrentTime) return true;
  if (g_infoManager.GetDisplayAfterSeek()) return true;
  if (g_infoManager.GetBool(PLAYER_SEEKBAR, GetID())) return true;
  if (CUtil::IsUsingTTFSubtitles() && g_application.m_pPlayer &&
      (g_application.m_pPlayer->GetSubtitleVisible() || g_application.m_pPlayer->GetSubtitleForced()) && m_subsLayout)
    return true;
  if (m_bLastRender)
  {
    m_bLastRender = false;
  }

  return false;
}

void CGUIWindowFullScreen::RenderFullScreen()
{
  static Uint32 lastRender = SDL_GetTicks();

  bool renderLabel = false;
  unsigned now = SDL_GetTicks();
  if (now - lastRender > 500)
  {
    renderLabel = true;
    lastRender = now;
  }

 #ifndef HAS_EMBEDDED
       renderLabel = true;
 #endif


  if (g_application.GetPlaySpeed() != 1)
    g_infoManager.SetDisplayAfterSeek();
  if (m_bShowCurrentTime)
    g_infoManager.SetDisplayAfterSeek();

  m_bLastRender = true;
  if (!g_application.m_pPlayer) return ;

  if( g_application.m_pPlayer->IsCaching() )
  {
    g_infoManager.SetDisplayAfterSeek(0); //Make sure these stuff aren't visible now
    SET_CONTROL_VISIBLE(LABEL_BUFFERING);
  }
  else 
  {
    SET_CONTROL_HIDDEN(LABEL_BUFFERING);
  }

  //------------------------
  bool bShowCodec = g_infoManager.GetBool(PLAYER_SHOWCODEC);
  if (bShowCodec && renderLabel)
  {
    // show audio codec info
    CStdString strAudio, strVideo, strGeneral;
    g_application.m_pPlayer->GetAudioInfo(strAudio);
    {
      CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1);
      msg.SetLabel(strAudio);
      OnMessage(msg);
    }
    // show video codec info
    g_application.m_pPlayer->GetVideoInfo(strVideo);
    {
      CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2);
      msg.SetLabel(strVideo);
      OnMessage(msg);
    }
    // show general info
    g_application.m_pPlayer->GetGeneralInfo(strGeneral);
    {
      CStdString strGeneralFPS;
#ifdef __APPLE__
      // We show CPU usage for the entire process, as it's arguably more useful.
      double dCPU = m_resourceCounter.GetCPUUsage();
      CStdString strCores;
      strCores.Format("cpu: %.0f%%", dCPU);
#else
      CStdString strCores = g_cpuInfo.GetCoresUsageString();
#endif
      int missedvblanks;
      int    refreshrate;
      double clockspeed;
      CStdString strClock;
      
      if (g_VideoReferenceClock.GetClockInfo(missedvblanks, clockspeed, refreshrate))
        strClock.Format("S( refresh:%i missed:%i speed:%+.3f%% %s )"
                       , refreshrate
                       , missedvblanks
                       , clockspeed - 100.0
                       , g_renderManager.GetVSyncState().c_str());

      strGeneralFPS.Format("%s\nW( fps:%02.2f %s ) %s"
                         , strGeneral.c_str()
                         , g_infoManager.GetFPS()
                         , strCores.c_str(), strClock.c_str() );
#ifdef HAS_INTEL_SMD
      unsigned int videoCur, videoMax, audioCur, audioMax;
      videoCur = videoMax = audioCur = audioMax = 0;

      if(g_application.m_pPlayer->IsDirectRendering())
        g_IntelSMDGlobals.GetPortStatus(g_IntelSMDGlobals.GetVidDecInput(), videoCur, videoMax);
      else
      {
        videoCur = 0;
        videoMax = 0;
      }

      ismd_port_handle_t audio_input;
      audio_input = g_IntelSMDGlobals.GetAudioDevicePort(g_IntelSMDGlobals.GetPrimaryAudioDevice());
      g_IntelSMDGlobals.GetPortStatus(audio_input, audioCur, audioMax);

      CStdString playerTimeStr = "N/A";
      CStdString audioTimeStr = "N/A";
      CStdString videoTimeStr = "N/A";

      __int64 playerTime = g_application.m_pPlayer->GetTime();

      double audioTime = g_IntelSMDGlobals.IsmdToDvdPts(g_IntelSMDGlobals.GetAudioCurrentTime());
      double videoTime = g_IntelSMDGlobals.IsmdToDvdPts(g_IntelSMDGlobals.GetVideoCurrentTime());

      StringUtils::MilisecondsToTimeString((int)playerTime, playerTimeStr);

      if(audioTime != DVD_NOPTS_VALUE)
        StringUtils::MilisecondsToTimeString((int)(audioTime/1000.0), audioTimeStr);

      if(videoTime != DVD_NOPTS_VALUE)
        StringUtils::MilisecondsToTimeString((int)(videoTime/1000.0), videoTimeStr);

      strGeneralFPS.Format("%s\nW( DVDPlayer: %s SMD Audio: %s %d/%d SMD Video %s %d/%d %s ) %s"
                               , strGeneral.c_str()
                               , playerTimeStr.c_str()
                               ,  audioTimeStr.c_str()
                               ,  audioCur, audioMax
                               ,  videoTimeStr.c_str()
                               ,  videoCur, videoMax
                               , strCores.c_str()
                               , strClock.c_str() );
#endif

      CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW3);
      msg.SetLabel(strGeneralFPS);
      OnMessage(msg);
    }
  }
  //----------------------
  // ViewMode Information
  //----------------------
  if (m_bShowViewModeInfo && CTimeUtils::GetTimeMS() - m_dwShowViewModeTimeout > 2500)
  {
    m_bShowViewModeInfo = false;
  }
  if (m_bShowViewModeInfo)
  {
    {
      // get the "View Mode" string
      CStdString strTitle = g_localizeStrings.Get(629);
      CStdString strMode = g_localizeStrings.Get(630 + g_stSettings.m_currentVideoSettings.m_ViewMode);
      CStdString strInfo;
      strInfo.Format("%s : %s", strTitle.c_str(), strMode.c_str());
      CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1);
      msg.SetLabel(strInfo);
      OnMessage(msg);
    }
    // show sizing information
    CRect SrcRect, DestRect;
    float fAR;
    g_application.m_pPlayer->GetVideoRect(SrcRect, DestRect);
    g_application.m_pPlayer->GetVideoAspectRatio(fAR);
    {
      CStdString strSizing;
      strSizing.Format("Sizing: (%i,%i)->(%i,%i) (Zoom x%2.2f) AR:%2.2f:1 (Pixels: %2.2f:1)",
                       (int)SrcRect.Width(), (int)SrcRect.Height(),
                       (int)DestRect.Width(), (int)DestRect.Height(), g_stSettings.m_fZoomAmount, fAR*g_stSettings.m_fPixelRatio, g_stSettings.m_fPixelRatio);
      CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW2);
      msg.SetLabel(strSizing);
      OnMessage(msg);
    }
    // show resolution information
    int iResolution = g_graphicsContext.GetVideoResolution();
    {
      CStdString strStatus;
      strStatus.Format("%s %ix%i@%.2fHz %s",  
        g_localizeStrings.Get(13287), g_settings.m_ResInfo[iResolution].iWidth, 
        g_settings.m_ResInfo[iResolution].iHeight, g_settings.m_ResInfo[iResolution].fRefreshRate, 
        g_settings.m_ResInfo[iResolution].strMode.c_str());
      CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW3);
      msg.SetLabel(strStatus);
      OnMessage(msg);
    }
  }

  g_graphicsContext.ApplyGuiTransform();

  RenderTTFSubtitles();

  if (m_timeCodeShow && m_timeCodePosition != 0)
  {
    if ( (CTimeUtils::GetTimeMS() - m_timeCodeTimeout) >= 2500)
    {
      m_timeCodeShow = false;
      m_timeCodePosition = 0;
    }
    CStdString strDispTime = "hh:mm";

    CGUIMessage msg(GUI_MSG_LABEL_SET, GetID(), LABEL_ROW1);
    for (int count = 0; count < m_timeCodePosition; count++)
    {
      if (m_timeCodeStamp[count] == -1)
        strDispTime[count] = ':';
      else
        strDispTime[count] = (char)m_timeCodeStamp[count] + 48;
    }
    strDispTime += "/" + g_infoManager.GetLabel(PLAYER_DURATION) + " [" + g_infoManager.GetLabel(PLAYER_TIME) + "]"; // duration [ time ]
    msg.SetLabel(strDispTime);
    OnMessage(msg);
  }

  if(g_application.m_pPlayer && g_application.m_pPlayer->IsPaused() && !g_application.m_pPlayer->IsCaching() && !g_infoManager.m_performingSeek)
  {
    SET_CONTROL_VISIBLE(CONTROL_PAUSE_INDICATOR);
  }
  else
  {
    SET_CONTROL_HIDDEN(CONTROL_PAUSE_INDICATOR);
  }

  // show audio codec logo
  if(g_infoManager.IsShowPlaybackAudioCodecLogo())
  {
    if ( (CTimeUtils::GetTimeMS() - g_infoManager.GetAudioCodecLogoStartTime()) >= SHOW_SUDIO_CODEC_LOGO_IN_MS)
    {
      g_infoManager.SetShowAudioCodecLogo(false);
    }
  }

  if (bShowCodec || m_bShowViewModeInfo)
  {
    SET_CONTROL_VISIBLE(LABEL_ROW1);
    SET_CONTROL_VISIBLE(LABEL_ROW2);
    SET_CONTROL_VISIBLE(LABEL_ROW3);
    SET_CONTROL_VISIBLE(BLUE_BAR);
  }
  else if (m_timeCodeShow)
  {
    SET_CONTROL_VISIBLE(LABEL_ROW1);
    SET_CONTROL_HIDDEN(LABEL_ROW2);
    SET_CONTROL_HIDDEN(LABEL_ROW3);
    SET_CONTROL_VISIBLE(BLUE_BAR);
  }
  else
  {
    SET_CONTROL_HIDDEN(LABEL_ROW1);
    SET_CONTROL_HIDDEN(LABEL_ROW2);
    SET_CONTROL_HIDDEN(LABEL_ROW3);
    SET_CONTROL_HIDDEN(BLUE_BAR);
  }

  CGUIWindow::Render();
  
  g_graphicsContext.RestoreGuiTransform();
}

void CGUIWindowFullScreen::RenderTTFSubtitles()
{
  if (!g_application.m_pPlayer)
    return;

  if ((g_application.GetCurrentPlayer() == EPC_MPLAYER || g_application.GetCurrentPlayer() == EPC_DVDPLAYER) &&
      CUtil::IsUsingTTFSubtitles()
      && (g_application.m_pPlayer->GetSubtitleVisible() || g_application.m_pPlayer->GetSubtitleForced()))
  {
    CSingleLock lock (m_fontLock);

    if(!m_subsLayout)
      return;

    CStdString subtitleText;
    if (g_application.m_pPlayer->GetCurrentSubtitle(subtitleText))
    {
      // Remove HTML-like tags from the subtitles until
      subtitleText.Replace("\\r", "");
      subtitleText.Replace("\r", "");
      subtitleText.Replace("\\n", "[CR]");
      subtitleText.Replace("\n", "[CR]");
      subtitleText.Replace("<br>", "[CR]");
      subtitleText.Replace("\\N", "[CR]");
      subtitleText.Replace("<i>", "[I]");
      subtitleText.Replace("</i>", "[/I]");
      subtitleText.Replace("<b>", "[B]");
      subtitleText.Replace("</b>", "[/B]");
      subtitleText.Replace("<u>", "");
      subtitleText.Replace("<p>", "");
      subtitleText.Replace("<P>", "");
      subtitleText.Replace("&nbsp;", "");
      subtitleText.Replace("</u>", "");
      subtitleText.Replace("</i", "[/I]"); // handle tags which aren't closed properly (happens).
      subtitleText.Replace("</b", "[/B]");
      subtitleText.Replace("</u", "");

      RESOLUTION res = g_graphicsContext.GetVideoResolution();

      int iStyle = g_guiSettings.GetInt("subtitles.style");
      if ((iStyle & FONT_STYLE_BOLD) == FONT_STYLE_BOLD)
        subtitleText = CStdString("[B]") + subtitleText + CStdString("[/B]");

      if ((iStyle & FONT_STYLE_ITALICS) == FONT_STYLE_ITALICS)
        subtitleText = CStdString("[I]") + subtitleText + CStdString("[/I]");

      float maxWidth = (float) g_settings.m_ResInfo[res].Overscan.right - g_settings.m_ResInfo[res].Overscan.left;
      m_subsLayout->Update(subtitleText, maxWidth * 0.9f, false, true); // true to force LTR reading order (most Hebrew subs are this format)
      
      float textWidth, textHeight;
      m_subsLayout->GetTextExtent(textWidth, textHeight);
      textWidth /= g_graphicsContext.GetGUIScaleX();
      textHeight /= g_graphicsContext.GetGUIScaleY();
      float x = maxWidth * 0.5f + g_settings.m_ResInfo[res].Overscan.left;
      float y = g_settings.m_ResInfo[res].iSubtitles - textHeight;

       m_subsLayout->RenderOutline(x, y, 0, 0xFF000000, 3, XBFONT_CENTER_X, maxWidth);
    }
  }
}

void CGUIWindowFullScreen::ChangetheTimeCode(int remote)
{
  if (remote >= 58 && remote <= 67) //Make sure it's only for the remote
  {
    m_timeCodeShow = true;
    m_timeCodeTimeout = CTimeUtils::GetTimeMS();
    int itime = remote - 58;
    if (m_timeCodePosition <= 4 && m_timeCodePosition != 2)
    {
      m_timeCodeStamp[m_timeCodePosition++] = itime;
      if (m_timeCodePosition == 2)
        m_timeCodeStamp[m_timeCodePosition++] = -1;
    }
    if (m_timeCodePosition > 4)
    {
      long itotal, ih, im, is = 0;
      ih = (m_timeCodeStamp[0] - 0) * 10;
      ih += (m_timeCodeStamp[1] - 0);
      im = (m_timeCodeStamp[3] - 0) * 10;
      im += (m_timeCodeStamp[4] - 0);
      im *= 60;
      ih *= 3600;
      itotal = ih + im + is;

      if (itotal < g_application.GetTotalTime())
        g_application.SeekTime((double)itotal);

      m_timeCodePosition = 0;
      m_timeCodeShow = false;
    }
  }
}

void CGUIWindowFullScreen::Seek(bool bPlus, bool bLargeStep)
{
  if(g_application.m_pPlayer->CanSeek())
  {
    //g_application.m_pPlayer->Seek(bPlus, bLargeStep);

    ShowOSD(bPlus ? CSeekDirection::FORWARD : CSeekDirection::BACKWARD);

    // Make sure gui items are visible.
    g_infoManager.SetDisplayAfterSeek();    
  }
}

void CGUIWindowFullScreen::SeekChapter(int iChapter)
{
  if(g_application.m_pPlayer->CanSeek())
  {
    g_application.m_pPlayer->SeekChapter(iChapter);
 
    // Make sure gui items are visible.
    g_infoManager.SetDisplayAfterSeek();    
  }
}

void CGUIWindowFullScreen::OnSliderChange(void *data, CGUISliderControl *slider)
{
  if (!slider)
    return;

  slider->SetTextValue(CGUIDialogAudioSubtitleSettings::FormatDelay(slider->GetFloatValue(), 0.025f));
  if (data && g_application.m_pPlayer)
  {
    if (*(int *)data == ACTION_AUDIO_DELAY)
    {
      g_stSettings.m_currentVideoSettings.m_AudioDelay = slider->GetFloatValue();
      g_application.m_pPlayer->SetAVDelay(g_stSettings.m_currentVideoSettings.m_AudioDelay);
    }
    else if (*(int *)data == ACTION_SUBTITLE_DELAY)
    {
      g_stSettings.m_currentVideoSettings.m_SubtitleDelay = slider->GetFloatValue();
      g_application.m_pPlayer->SetSubTitleDelay(g_stSettings.m_currentVideoSettings.m_SubtitleDelay);
    }
  }
}

