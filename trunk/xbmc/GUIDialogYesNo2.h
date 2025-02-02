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

#include "GUIDialogBoxBase.h"

class CGUIDialogYesNo2 :
      public CGUIDialogBoxBase
{
public:
  CGUIDialogYesNo2(void);
  virtual ~CGUIDialogYesNo2(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction& action);
  
  static bool ShowAndGetInput(int heading, int line, int iNoLabel=-1, int iYesLabel=-1);
  static bool ShowAndGetInput(int heading, int line, bool& bCanceled);
  static bool ShowAndGetInput(int heading, int line, int iNoLabel, int iYesLabel, bool& bCanceled, unsigned int autoCloseTime = 0, int defaultButton=-1);
  static bool ShowAndGetInput(const CStdString& heading, const CStdString& line);
  static bool ShowAndGetInput(const CStdString& heading, const CStdString& line, bool &bCanceled);
  static bool ShowAndGetInput(const CStdString& heading, const CStdString& line, int defaultButton);
  static bool ShowAndGetInput(const CStdString& heading, const CStdString& line, const CStdString& noLabel, const CStdString& yesLabel);
  static bool ShowAndGetInput(const CStdString& heading, const CStdString& line, const CStdString& noLabel, const CStdString& yesLabel, bool &bCanceled, int defaultButton = -1);

protected:
  bool m_bCanceled;
};
