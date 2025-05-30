/* 
 * Motif
 *
 * Copyright (c) 1987-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these librararies and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
*/ 

/*
 * Included Files:
 */

/*
 * Value definitions:
 */



/******************************<->*************************************
 *
 *  Window manager resource names ...
 *
 *
 *  Description:
 *  -----------
 * 
 ******************************<->***********************************/

/* mwm specific appearance and behavior resources: */

#ifdef WSM
#define WmNall				"all"
#endif /* WSM */
#define WmNautoKeyFocus			"autoKeyFocus"
#define WmNautoRaiseDelay		"autoRaiseDelay"
#ifdef WSM
#define WmNbackdropDirectories		"backdropDirectories"
#endif /* WSM */
#define WmNbitmapDirectory		"bitmapDirectory"
#ifdef MINIMAL_DT
#define WmNblinkOnExec			"blinkOnExec"
#endif /* MINIMAL_DT */
#define WmNbuttonBindings		"buttonBindings"
#define WmNcleanText			"cleanText"
#define WmNclientAutoPlace		"clientAutoPlace"
#define WmNcolormapFocusPolicy		"colormapFocusPolicy"
#define WmNconfigFile			"configFile"
#ifdef WSM
#define WmNcppCommand			"cppCommand"
#endif /* WSM */
#define WmNdeiconifyKeyFocus		"deiconifyKeyFocus"
#define WmNdoubleClickTime		"doubleClickTime"
#define WmNenableWarp			"enableWarp"
#define WmNenforceKeyFocus		"enforceKeyFocus"
#define WmNfadeNormalIcon		"fadeNormalIcon"
#define WmNfeedbackGeometry		"feedbackGeometry"
#define WmNframeBorderWidth		"frameBorderWidth"
#ifdef WSM
#define WmNframeExternalShadowWidth	"frameExternalShadowWidth"
#define WmNgeometry			"geometry"
#endif /* WSM */
#define WmNiconAutoPlace		"iconAutoPlace"
#define WmNiconBoxGeometry		"iconBoxGeometry"
#define WmNiconBoxLayout		"iconBoxLayout"
#define WmNiconBoxName			"iconBoxName"
#define WmNiconBoxSBDisplayPolicy	"iconBoxSBDisplayPolicy"
#define WmNiconBoxScheme		"iconBoxScheme"
#define WmNiconBoxTitle			"iconBoxTitle"
#define WmNiconClick			"iconClick"
#define WmNiconDecoration		"iconDecoration"
#ifdef WSM
#define WmNiconExternalShadowWidth	"iconExternalShadowWidth"
#endif /* WSM */
#define WmNiconImageMaximum		"iconImageMaximum"
#define WmNiconImageMinimum		"iconImageMinimum"
#define WmNiconPlacement		"iconPlacement"
#define WmNiconPlacementMargin		"iconPlacementMargin"
#ifdef WSM
#define WmNimage			"image"
#define WmNimageBackground		"imageBackground"
#define WmNimageForeground		"imageForeground"
#endif /* WSM */
#define WmNinteractivePlacement		"interactivePlacement"
#define WmNkeyBindings			"keyBindings"
#define WmNkeyboardFocusPolicy		"keyboardFocusPolicy"
#define WmNlimitResize			"limitResize"
#define WmNlowerOnIconify		"lowerOnIconify"
#ifdef WSM
#define WmNmarqueeSelectGranularity	"marqueeSelectGranularity"
#endif /* WSM */
#define WmNmaximumMaximumSize		"maximumMaximumSize"
#define WmNmoveThreshold		"moveThreshold"
#define WmNmultiScreen			"multiScreen"
#define WmNpassButtons			"passButtons"
#define WmNpassSelectButton		"passSelectButton"
#define WmNpositionOnScreen		"positionOnScreen"
#define WmNquitTimeout			"quitTimeout"
#define WmNraiseKeyFocus		"raiseKeyFocus"
#ifdef WSM
#define WmNrefreshByClearing		"refreshByClearing"
#endif
#define WmNresizeBorderWidth		"resizeBorderWidth"
#define WmNresizeCursors		"resizeCursors"
#define WmNrootButtonClick		"rootButtonClick"
#ifndef WSM
# define WmNrootMenu			"rootMenu"
#endif /* WSM */
#ifdef WSM
#define WmNsecondariesOnTop		"secondariesOnTop"
#define WmNsessionVersion               "sessionVersion"
#endif /* WSM */
#ifndef WSM
#define WmNsessionClientDB		"sessionClientDB"
#endif /* ! WSM */
#define WmNshowFeedback			"showFeedback"
#ifdef WSM
#define WmNshowNames			"showNames"
#endif /* WSM */
#define WmNstartupKeyFocus		"startupKeyFocus"
#define WmNsystemButtonClick		"wMenuButtonClick"
#define WmNsystemButtonClick2		"wMenuButtonClick2"
#define WmNtransientDecoration		"transientDecoration"
#define WmNtransientFunctions		"transientFunctions"
#define WmNuseIconBox			"useIconBox"
#define WmNuseWindowOutline		"useWindowOutline"
#define WmNoutlineWidth			"outlineWidth"
#ifdef MINIMAL_DT
#define WmNdtLite			"useDtLite"
#endif /* MINIMAL_DT */
#define WmNmoveOpaque                   "moveOpaque"
#define WmNframeStyle			"frameStyle"
#define WmNutilityDecoration    "utilityDecoration"
#define WmNutilityFunctions     "utilityFunctions"
#define WmNprimaryXineramaScreen	"primaryXineramaScreen"
#define WmNxineramaFollowPointer    "xineramaFollowPointer"
#define WmNxineramaIconifyToPrimary "xineramaIconifyToPrimary"

/* conponent appearance resources: */

#define WmNactiveBackground		"activeBackground"
#define WmNactiveBackgroundPixmap	"activeBackgroundPixmap"
#define WmNactiveBottomShadowColor 	"activeBottomShadowColor"
#define WmNactiveBottomShadowPixmap	"activeBottomShadowPixmap"
#define WmNactiveForeground		"activeForeground"
#define WmNactiveTopShadowColor		"activeTopShadowColor"
#define WmNactiveTopShadowPixmap	"activeTopShadowPixmap"
#define WmNbackground			"background"
#define WmNbackgroundPixmap		"backgroundPixmap"
#define WmNbottomShadowColor 		"bottomShadowColor"
#define WmNbottomShadowPixmap		"bottomShadowPixmap"
#define WmNfont				"font"
#define WmNforeground			"foreground"
#define WmNsaveUnder			"saveUnder"
#define WmNtopShadowColor		"topShadowColor"
#define WmNtopShadowPixmap		"topShadowPixmap"
#define WmNtitleLeft            "titleLeft"

/* mwm - client specific resources: */

#ifdef WSM
#define WmNabsentMapBehavior		"absentMapBehavior"
#endif /* WSM */
#define WmNclientDecoration		"clientDecoration"
#define WmNclientFunctions		"clientFunctions"
#define WmNfocusAutoRaise		"focusAutoRaise"
#ifdef WSM
#define WmNhelpResources                "helpResources"
#endif /* WSM */
#define WmNiconImage			"iconImage"
#define WmNiconImageBackground		"iconImageBackground"
#define WmNiconImageBottomShadowColor	"iconImageBottomShadowColor"
#define WmNiconImageBottomShadowPixmap	"iconImageBottomShadowPixmap"
#define WmNiconImageForeground		"iconImageForeground"
#define WmNiconImageTopShadowColor	"iconImageTopShadowColor"
#define WmNiconImageTopShadowPixmap	"iconImageTopShadowPixmap"
#define WmNignoreWMSaveHints            "ignoreWMSaveHints"
#ifdef WSM
#define WmNinitialWorkspace             "initialWorkspace"
#endif /* WSM */
#define WmNmatteBackground		"matteBackground"
#define WmNmatteBottomShadowColor	"matteBottomShadowColor"
#define WmNmatteBottomShadowPixmap	"matteBottomShadowPixmap"
#define WmNmatteForeground		"matteForeground"
#define WmNmatteTopShadowColor		"matteTopShadowColor"
#define WmNmatteTopShadowPixmap		"matteTopShadowPixmap"
#define WmNmatteWidth			"matteWidth"
#define WmNmaximumClientSize		"maximumClientSize"
#define WmNscreenList			"screenList"
#define WmNscreens			"screens"
#define WmNsystemMenu			"windowMenu"
#define WmNuseClientIcon		"useClientIcon"
#define WmNusePPosition			"usePPosition"
#define WmNoverrideGeometry		"overrideGeometry"
#ifdef WSM
#define WmNworkspaceList	        "workspaceList"
#define WmNworkspaceCount	        "workspaceCount"
#endif /* WSM */

/* window manager part resource names: */

#define WmNclient			"client"
#define WmNfeedback			"feedback"
#define WmNicon				"icon"
#define WmNmenu				"menu"
#define WmNtitle			"title"
#define WmNdefaults			"defaults"
#ifdef WSM
#define WmNbackdrop			"backdrop"
#define WmNcolorSetId			"colorSetId"
#define WmNfrontPanel			"frontPanel"
#define WmNworkspaceController		"workspaceController"
#define WmNworkspacePresence		"workspacePresence"
#define WmNworkspaceSwitch		"workspaceSwitch"
#endif /* WSM */

/* window manager client resource names: */

#define WmNiconBox			"iconbox"
#define WmNconfirmbox			"confirmbox"
#ifdef WSM
#define WmNswitcher			"switcher"
#endif /* WSM */



/*************************************<->*************************************
 *
 *  Window manager resource classes ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/

/* mwm specific appearance and behavior resources: */

#define WmCAutoKeyFocus			"AutoKeyFocus"
#define WmCAutoRaiseDelay		"AutoRaiseDelay"
#ifdef WSM
#define WmCBackdropDirectories		"BackdropDirectories"
#endif /* WSM */
#define WmCBitmapDirectory		"BitmapDirectory"
#ifdef MINIMAL_DT
#define WmCBlinkOnExec			"BlinkOnExec"
#endif /* MINIMAL_DT */
#define WmCButtonBindings		"ButtonBindings"
#define WmCCleanText			"CleanText"
#define WmCClientAutoPlace		"ClientAutoPlace"
#define WmCColormapFocusPolicy		"ColormapFocusPolicy"
#define WmCConfigFile			"ConfigFile"
#ifdef WSM
#define WmCCppCommand			"CppCommand"
#endif /* WSM */
#define WmCDeiconifyKeyFocus		"DeiconifyKeyFocus"
#define WmCDoubleClickTime		"DoubleClickTime"
#define WmCEnableWarp			"EnableWarp"
#define WmCEnforceKeyFocus		"EnforceKeyFocus"
#define WmCFadeNormalIcon		"FadeNormalIcon"
#define WmCFeedbackGeometry		"FeedbackGeometry"
#define WmCFrameBorderWidth		"FrameBorderWidth"
#ifdef WSM
#define WmCFrameExternalShadowWidth	"FrameExternalShadowWidth"
#define WmCGeometry			"Geometry"
#endif /* WSM */
#define WmCIconAutoPlace		"IconAutoPlace"
#define WmCIconBoxGeometry		"IconBoxGeometry"
#define WmCIconBoxLayout		"IconBoxLayout"
#define WmCIconBoxName			"IconBoxName"
#define WmCIconBoxSBDisplayPolicy	"IconBoxSBDisplayPolicy"
#define WmCIconBoxScheme		"IconBoxScheme"
#define WmCIconBoxTitle			"IconBoxTitle"
#define WmCIconClick			"IconClick"
#define WmCIconDecoration		"IconDecoration"
#ifdef WSM
#define WmCIconExternalShadowWidth	"IconExternalShadowWidth"
#endif /* WSM */
#define WmCIconImageMaximum		"IconImageMaximum"
#define WmCIconImageMinimum		"IconImageMinimum"
#define WmCIconPlacement		"IconPlacement"
#define WmCIconPlacementMargin		"IconPlacementMargin"
#ifdef WSM
#define WmCImage			"Image"
#define WmCImageBackground		"ImageBackground"
#define WmCImageForeground		"ImageForeground"
#endif /* WSM */
#define WmCInteractivePlacement		"InteractivePlacement"
#define WmCKeyBindings			"KeyBindings"
#define WmCKeyboardFocusPolicy		"KeyboardFocusPolicy"
#define WmCLimitResize			"LimitResize"
#define WmCLowerOnIconify		"LowerOnIconify"
#ifdef WSM
#define WmCMarqueeSelectGranularity	"MarqueeSelectGranularity"
#endif /* WSM */
#define WmCMaximumMaximumSize		"MaximumMaximumSize"
#define WmCMoveThreshold		"MoveThreshold"
#define WmCMultiScreen			"MultiScreen"
#define WmCPassButtons			"PassButtons"
#define WmCPassSelectButton		"PassSelectButton"
#define WmCPositionOnScreen		"PositionOnScreen"
#define WmCQuitTimeout			"QuitTimeout"
#define WmCRaiseKeyFocus		"RaiseKeyFocus"
#ifdef WSM
#define WmCRefreshByClearing		"RefreshByClearing"
#endif
#define WmCResizeBorderWidth		"ResizeBorderWidth"
#define WmCResizeCursors		"ResizeCursors"
#define WmCRootButtonClick		"RootButtonClick"
#ifndef WSM
# define WmCRootMenu			"RootMenu"
#endif /* WSM */
#ifdef WSM
#define WmCSecondariesOnTop		"SecondariesOnTop"
#define WmCSessionVersion               "SessionVersion"
#endif /* WSM */
#ifndef WSM
#define WmCSessionClientDB		"SessionClientDB"
#endif /* ! WSM */
#define WmCScreenList			"ScreenList"
#define WmCScreens			"Screens"
#define WmCShowFeedback			"ShowFeedback"
#ifdef WSM
#define WmCShowNames			"ShowNames"
#endif /* WSM */
#define WmCStartupKeyFocus		"StartupKeyFocus"
#define WmCSystemButtonClick		"WMenuButtonClick"
#define WmCSystemButtonClick2		"WMenuButtonClick2"
#define WmCTransientDecoration		"TransientDecoration"
#define WmCTransientFunctions		"TransientFunctions"
#define WmCUseIconBox			"UseIconBox"
#define WmCUseWindowOutline		"UseWindowOutline"
#define WmCOutlineWidth			"OutlineWidth"
#ifdef MINIMAL_DT
#define WmCDtLite			"UseDtLite"
#endif /* MINIMAL_DT */
#define WmCMoveOpaque                   "MoveOpaque"
#define WmCFrameStyle			"FrameStyle"
#ifdef WSM
#define WmCWorkspaceList	        "WorkspaceList"
#define WmCWorkspaceCount	        "WorkspaceCount"
#endif /* WSM */
#define WmCUtilityDecoration     "UtilityDecoration"
#define WmCUtilityFunctions      "UtilityFunctions"
#define WmCPrimaryXineramaScreen	"PrimaryXineramaScreen"
#define WmCXineramaFollowPointer    "XineramaFollowPointer"
#define WmCXineramaIconifyToPrimary "XineramaIconifyToPrimary"

/* component appearance resources: */

#define WmCActiveBackground		"ActiveBackground"
#define WmCActiveBackgroundPixmap	"ActiveBackgroundPixmap"
#define WmCActiveBottomShadowColor 	"ActiveBottomShadowColor"
#define WmCActiveBottomShadowPixmap	"ActiveBottomShadowPixmap"
#define WmCActiveForeground		"ActiveForeground"
#define WmCActiveTopShadowColor		"ActiveTopShadowColor"
#define WmCActiveTopShadowPixmap	"ActiveTopShadowPixmap"
#define WmCBackground			"Background"
#define WmCBackgroundPixmap		"BackgroundPixmap"
#define WmCBottomShadowColor 		"BottomShadowColor"
#define WmCBottomShadowPixmap		"BottomShadowPixmap"
#define WmCFont				"Font"
#define WmCForeground			"Foreground"
#define WmCSaveUnder			"SaveUnder"
#define WmCTopShadowColor		"TopShadowColor"
#define WmCTopShadowPixmap		"TopShadowPixmap"
#define WmCTitleLeft            "TitleLeft"

/* mwm - client specific resources: */

#ifdef WSM
#define WmCAbsentMapBehavior		"AbsentMapBehavior"
#endif /* WSM */
#define WmCClientDecoration		"ClientDecoration"
#define WmCClientFunctions		"ClientFunctions"
#define WmCFocusAutoRaise		"FocusAutoRaise"
#ifdef WSM
#define WmCHelpResources                "HelpResources"
#endif /* WSM */
#define WmCIconImage			"IconImage"
#define WmCIconImageBackground		"IconImageBackground"
#define WmCIconImageBottomShadowColor	"IconImageBottomShadowColor"
#define WmCIconImageBottomShadowPixmap	"IconImageBottomShadowPixmap"
#define WmCIconImageForeground		"IconImageForeground"
#define WmCIconImageTopShadowColor	"IconImageTopShadowColor"
#define WmCIconImageTopShadowPixmap	"IconImageTopShadowPixmap"
#define WmCIgnoreWMSaveHints            "IgnoreWMSaveHints"
#ifdef WSM
#define WmCInitialWorkspace             "InitialWorkspace"
#endif /* WSM */
#define WmCMatteBackground		"MatteBackground"
#define WmCMatteBottomShadowColor	"MatteBottomShadowColor"
#define WmCMatteBottomShadowPixmap	"MatteBottomShadowPixmap"
#define WmCMatteForeground		"MatteForeground"
#define WmCMatteTopShadowColor		"MatteTopShadowColor"
#define WmCMatteTopShadowPixmap		"MatteTopShadowPixmap"
#define WmCMatteWidth			"MatteWidth"
#define WmCMaximumClientSize		"MaximumClientSize"
#define WmCSystemMenu			"WindowMenu"
#define WmCUseClientIcon		"UseClientIcon"
#define WmCUsePPosition			"UsePPosition"
#define WmCOverrideGeometry		"OverrideGeometry"

/* window manager part resource names: */

#define WmCClient			"Client"
#define WmCFeedback			"Feedback"
#define WmCIcon				"Icon"
#define WmCMenu				"Menu"
#define WmCTitle			"Title"
#define WmCDefaults			"Defaults"
#ifdef WSM
#define WmCBackdrop			"Backdrop"
#define WmCColorSetId			"ColorSetId"
#define WmCFrontPanel			"FrontPanel"
#define WmCWorkspaceController		"WorkspaceController"
#define WmCWorkspacePresence		"WorkspacePresence"
#define WmCWorkspaceSwitch		"WorkspaceSwitch"
#endif /* WSM */

/* window manager client resource names: */

#define WmCIconBox			"Iconbox"
#define WmCConfirmbox			"Confirmbox"
#ifdef WSM
#define WmCSwitcher			"Switcher"
#endif /* WSM */



/*************************************<->*************************************
 *
 *  Window manager resource converter names ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/

#ifdef WSM
#define WmRAbsentMapBehavior            "WmAMBehavior"
#endif /* WSM */
#define WmRCFocusPolicy			"WmCFocus"
#define WmRClientDecor			"WmCDecor"
#define WmRClientFunction		"WmCFunc"
#define WmRFrameStyle			"WmFrameStyle"
#define WmRIconBoxLayout		"WmIBLayout"
#define WmRIconDecor			"WmIDecor"
#define WmRIconPlacement		"WmIPlace"
#define WmRKFocusPolicy			"WmKFocus"
#define WmRSize				"WmSize"
#define WmRShowFeedback			"WmShowFeedback"
#define WmRUsePPosition			"WmUsePPosition"



/*************************************<->*************************************
 *
 *  Window manager resource set definitions and default resource values ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/




/*************************************<->*************************************
 *
 *  Mwm resource description file definitions ...
 *
 *
 *  Description:
 *  -----------
 * 
 *************************************<->***********************************/

/* Configuration resource types: */

#define CRS_BUTTON		(1L << 0)
#define CRS_KEY			(1L << 1)
#define CRS_MENU		(1L << 2)
#define CRS_ACCEL		(1L << 3)
#define CRS_ANY			(CRS_BUTTON | CRS_KEY | CRS_MENU | CRS_ACCEL)

/**************************    eof  ************************/
