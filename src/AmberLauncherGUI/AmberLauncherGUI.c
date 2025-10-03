#include "core/core.hxx"
#include <core/stdint.h>
#include "core/hfile.h"
#include "draw2d/draw2d.hxx"
#include "draw2d/guictx.hxx"
#include <ext/miniz.h>
#include "gui/gui.hxx"
#include "osbs/osbs.hxx"
#include "sewer/types.hxx"
#include "version.h"
#include <AmberLauncherGUI.h>
#include <AmberLauncherCore.h>

#include <inet/inet.h>
#include <inet/httpreq.h>
#include <encode/json.h>
#include <core/common.h>
#include <core/appcore.h>

#include <nappgui.h>
#include <res_app.h>

#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/******************************************************************************
 * WINDOWS
 ******************************************************************************/

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>

static void
_Win32_SetupConsole(void)
{
    if (AttachConsole(ATTACH_PARENT_PROCESS)) 
    {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
        freopen("CONIN$", "r", stdin);

        setvbuf(stdout, NULL, _IONBF, 0);
    }
}

#endif

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static const char *
_sDefaultUpdaterRemoteRootURL = 
    "https://mightandmagicmod.com/updater/";

static const char *_sDefaultUpdaterLauncherManifestURL = 
    "https://mightandmagicmod.com/updater/al_manifest.json";

static const char *_sDefaultUpdaterModManifestURL = 
    "https://mightandmagicmod.com/updater/mod_manifest.json";

static const char *_sLua_URL_UPDATER_ROOT =
    "URL_UPDATER_ROOT";
static const char *_sLua_URL_UPDATER_LAUNCHER_MANIFEST =
    "URL_UPDATER_LAUNCHER_MANIFEST";
static const char *_sLua_URL_UPDATER_MOD_MANIFEST =
    "URL_UPDATER_MOD_MANIFEST";
static const char *_sLua_LAUNCHER_VERSION =
    "LAUNCHER_VERSION";
static const char *_sLua_MOD_VERSION =
    "GAME_MOD_VERSION";

const char* EUIEventTypeStrings[] = {
    "NULL",
    "PRINT",
    "MAIN",
    "PLAY",
    "EXITAPP",
    "AUTOCONFIG",
    "MODAL_CLOSE",
    "MODAL_MESSAGE",
    "MODAL_QUESTION",
    "MODAL_GAMENOTFOUND",
    "MODAL_TWEAKER",
    "MODAL_LOCALISATION",
    "MODAL_OPTIONS",
    "MODAL_MODS",
    "MODAL_TOOLS",
    "MODAL_UPDATER",
    NULL
};

#define AL_PRINTF_BUFFER_SIZE               2048

#define PANEL_DEFAULT_W                     640.f
#define PANEL_DEFAULT_H                     480.f
#define LAYOUT_DEFAULT_MARGIN               4.f
#define TITLE_PNG_W                         672.f
#define TITLE_PNG_H                         200.f
#define ICO_PNG_W                           32.f
#define ICO_PNG_H                           32.f
#define IMAGE_DEFAULT_W                     480.f
#define IMAGE_DEFAULT_H                     320.f

/* shortcut */
static const real32_t _fDefMarg             = 4.0f;
static const real32_t _fMinBtnWidth         = 128.f;

#define MODAL_OPT_NULL                      1000
#define MODAL_OPT_A                         1001
#define MODAL_OPT_B                         1002
#define MODAL_OPT_MAX                       1030
#define MODAL_CANCEL                        1031
#define MODAL_ACCEPT                        1032
#define MODAL_BROWSE                        1033
#define MODAL_PREVIOUS                      1034
#define MODAL_NEXT                          1035
#define MODAL_UPDATE_APP                    1036

/******************************************************************************
 * STATIC DECLARATIONS
 ******************************************************************************/

 /**
  * @relatedalso GUI
  * @brief       Prints text to stdout and to currently active textview widget
  *
  * @param       fmt
  * @param       ...
  */
static void
_al_printf(AppGUI *pApp, const char* fmt, ...);

/**
 * @relatedalso Window
 * @brief       Centers window on screen
 *
 * @param       pWindow
 */
static void
_window_center(Window* pWindow);

/**
 * @relatedalso Panel
 * @brief       Returns root panel of the launcher
 * 
 * @param       pApp 
 * @return      Panel* 
 */
static Panel*
_Panel_GetRoot(AppGUI *pApp);

/**
 * @relatedalso GUI
 * @brief       Initializes NappGUI instance
 * 
 * @return      App* 
 */
static AppGUI*
_Nappgui_Start(void);

/**
 * @relatedalso GUI
 * @brief       Terminates and cleans up NappGUI instance
 *
 * @param       pApp
 */
static void 
_Nappgui_End(AppGUI **pApp);

/**
 * @relatedalso GUI
 * @brief       Shows modal window
 *
 * @param       pApp
 * @param       pPanel
 * @param       sTitle
 */
static uint32_t
_Nappgui_ShowModal( AppGUI *pApp, Panel *pPanel, const char* sTitle);

static unsigned int
_RowForPage(unsigned int dPage, unsigned int dPageMax);

static Panel*
_Panel_TweakerDescription(AppGUI *pApp);

static Panel*
_Panel_TweakerOptionsSelector(AppGUI *pApp);

static Panel*
_Panel_TweakerButtons(AppGUI *pApp);

static void
_LocalisationFillPopup(AppGUI *pApp, PopUp *pPop, ELocTier eTier);

static uint32_t
_GUIThread_Main_Null(GUIAsyncTaskData *pThreadData);

static void
_GUIThread_End_PanelSetMain(GUIAsyncTaskData *pThreadData, const uint32_t dRValue);

/******************************************************************************
 * STATIC CALLBACK DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso GUI
 * @brief       Callback for sidebar buttons on main menu
 *
 * @param       pApp
 * @param       e
 */
static void
_Callback_OnButtonMainWindow(AppGUI* pApp, Event* e);

/**
 * @relatedalso GUI
 * @brief       Invokes cleanup on manual closing of the main window
 *
 * @param       pApp
 * @param       e
 */
static void
_Callback_OnCloseMainWindow(AppGUI* pApp, Event* e);

/**
 * @relatedalso GUI
 * @brief       Invokes cleanup on manual closing of the main window
 *
 * @param       pApp
 * @param       e
 */
static void
_Callback_OnCloseModalWindow(AppGUI* pApp, Event* e);

/**
 * @relatedalso GUI
 * @brief       Related to various UI events called from (mostly) Lua
 *
 * @param       pApp
 * @param       eEventType         EUserEventType
 * @param       pUserData
 */
static SVarKeyBundle
_Callback_UIEvent(
    AppCore *pAppCore,
    EUIEventType eEventType,
    const SVar *pUserData,
    const unsigned int dNumArgs);


static void
_Callback_LocalisationPopupSelector(AppGUI *pApp, Event *e);

/******************************************************************************
 * HEADER DEFINITIONS
 ******************************************************************************/

Panel *
Panel_Set(AppGUI *pApp, const EPanelType eType, FPanelFlags dFlags)
{
    Panel *pPanel       = NULL;

    pApp->eCurrentPanel = eType;

    if (IS_VALID(pApp->pWindows->pLayoutWindow))
    {
        layout_show_col(pApp->pWindows->pLayoutWindow, 0, TRUE);
    }

    switch (eType)
    {
    case CPANEL_NULL:
        pPanel = Panel_GetNull(pApp);
        break;

    case CPANEL_AUTOCONFIG:
        if (IS_VALID(pApp->pWindows->pLayoutWindow))
        {
            layout_show_col(pApp->pWindows->pLayoutWindow, 0, FALSE);
        }
        pPanel = Panel_GetAutoConfigure(pApp,dFlags);
        break;

    case CPANEL_MAIN:
        pPanel = Panel_GetMain(pApp);
        break;

    case CPANEL_IMAGEDEMO:
        pPanel = Panel_GetImageDemo(pApp);
        break;

    case CPANEL_MODAL:
        pPanel = Panel_GetNull(pApp);
        break;

    case CPANEL_MAX:
    default:
        pPanel = Panel_GetNull(pApp);
        break;
    }

    pApp->pWindows->pPanelMain = pPanel;
    layout_panel_replace(pApp->pWindows->pLayoutMain, pPanel, 0, 1);

    return pPanel;
}

Panel* 
Panel_GetNull(AppGUI* pApp)
{
    Panel   *pPanelMain  = panel_create();
    Layout  *pLayoutMain = layout_create(1, 1);
    Label   *pLabelNull  = label_create();

    /* Label: Null */
    label_text(pLabelNull, "Null");
    label_multiline(pLabelNull, TRUE);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, TITLE_PNG_W);
    layout_halign(pLayoutMain, 0, 0, ekCENTER);
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_margin(pLayoutMain, 4);
    layout_label(pLayoutMain, pLabelNull,0, 0);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);
    return pPanelMain;
}

Panel* 
Panel_GetAutoConfigure(AppGUI* pApp, FPanelFlags dFlags)
{
    Panel       *pPanelMain  = panel_create();
    Layout      *pLayoutMain = layout_create(1,3);
    Layout      *pLayoutTxt  = layout_create(1,1);
    Label       *pLabelGreet = label_create();
    Button      *pButtonCfg  = button_push();
    TextView    *pConsole    = textview_create();
    Font        *pFontGreet  = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFontCfg    = font_system(24, ekFNORMAL | ekFPIXELS);
    bool_t      bIsStateEnd  = FLAG_HAS(dFlags, FLAG_PANEL_STATE_END);

    if (bIsStateEnd)
    {
        if (IS_VALID(pApp->pWidgets->pTextView))
        {
            const char_t *sPreviousLog = textview_get_text(pApp->pWidgets->pTextView);
            textview_printf(pConsole, "%s", sPreviousLog);
        }
    }
    
    pApp->pWidgets->pTextView = pConsole;

    /* Label: Greetings Text */
    label_text(pLabelGreet, bIsStateEnd ? TXT_CONFIGSUCCESS : TXT_CONFIGINTRO);
    label_multiline(pLabelGreet, TRUE);
    label_font(pLabelGreet, pFontGreet);

    /* Button: Configure */
    button_text(pButtonCfg, bIsStateEnd ? TXT_BTN_NEXT : TXT_BTN_CONFIGURE);
    button_font(pButtonCfg, pFontCfg);
    button_vpadding (pButtonCfg, 16);
    button_tag(pButtonCfg, dFlags);
    button_OnClick(pButtonCfg, listener(pApp, Callback_OnButtonConfigure, AppGUI));
    
    /* Layout: Text */
    layout_margin(pLayoutTxt, 4);
    layout_valign(pLayoutTxt, 0, 0, ekTOP);
    layout_hsize(pLayoutTxt, 0, TITLE_PNG_W);
    layout_label(pLayoutTxt, pLabelGreet, 0, 0);

    /* Layout: Main */
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_valign(pLayoutMain, 0, 1, ekTOP);
    layout_valign(pLayoutMain, 0, 2, ekBOTTOM);
    layout_layout(pLayoutMain, pLayoutTxt, 0, 0);
    layout_textview(pLayoutMain, pConsole, 0, 1);
    layout_button(pLayoutMain,pButtonCfg,0, 2);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    /* Cleanup */
    font_destroy(&pFontGreet);
    font_destroy(&pFontCfg);

    return pPanelMain;
}

Panel* 
Panel_GetMain(AppGUI* pApp)
{
    Panel   *pPanelMain  = panel_create();
    Layout  *pLayoutMain = layout_create(1, 3);
    Layout  *pLayoutTxt  = layout_create(1,1);
    Label   *pLabelTemp  = label_create();
    Button  *pButtonPlay = button_push();
    Font    *pFontPlay   = font_system(24, ekFNORMAL | ekFPIXELS);

    pApp->pWidgets->pTextView = NULL;

    /* Label: Temp */
    label_text(pLabelTemp, TXT_MAININTRO);
    label_multiline(pLabelTemp, TRUE);

    /* Button: Play */
    button_text(pButtonPlay, TXT_BTN_PLAY);
    button_font(pButtonPlay, pFontPlay);
    button_vpadding (pButtonPlay, 16);
    button_OnClick(pButtonPlay, listener(pApp, Callback_OnButtonPlay, AppGUI));
    
    /* Layout: Text */
    layout_margin(pLayoutTxt, 4);
    layout_valign(pLayoutTxt, 0, 0, ekTOP);
    layout_hsize(pLayoutTxt, 0, TITLE_PNG_W);
    layout_label(pLayoutTxt, pLabelTemp, 0, 0);

    /* Layout: Main */
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_valign(pLayoutMain, 0, 1, ekJUSTIFY);
    layout_valign(pLayoutMain, 0, 2, ekBOTTOM);
    layout_hsize(pLayoutMain, 0, TITLE_PNG_W);
    layout_vsize(pLayoutMain, 1, 32);

    layout_layout(pLayoutMain, pLayoutTxt, 0, 0);
    layout_button(pLayoutMain, pButtonPlay, 0, 2);

    panel_layout(pPanelMain, pLayoutMain);

    /* Cleanup */
    font_destroy(&pFontPlay);

    unref(pApp);
    return pPanelMain;
}

Panel*
Panel_GetImageDemo(AppGUI *pApp)
{
    Panel       *pPanelMain     = panel_create();
    Layout      *pLayoutMain    = layout_create(1, 1);
    ImageView   *pImageView     = imageview_create();
    Image       *pImage         = NULL;
    ferror_t    eError          = 0;

    /* Image */
    if (IS_VALID(pApp->pWidgets->pString))
    {
        pImage = image_from_file(tc(pApp->pWidgets->pString), &eError);
        assert(eError == ekFOK);
    }

    /* Image View */
    imageview_scale(pImageView, ekGUI_SCALE_AUTO);
    imageview_size(pImageView, s2df(IMAGE_DEFAULT_W,IMAGE_DEFAULT_H));
    imageview_image(pImageView, eError == ekFOK ? pImage : NULL);

    /* Layout: Main */
    layout_imageview(pLayoutMain, pImageView, 0, 0);

    panel_layout(pPanelMain, pLayoutMain);

    if (IS_VALID(pImage))
    {
        image_destroy(&pImage);
    }

    unref(pApp);
    return pPanelMain;
}

Panel*
Panel_GetModalDebug(AppGUI* pApp)
{
    Panel       *pPanelMain     = panel_create();
    Layout      *pLayoutMain    = layout_create(1,2);
    Layout      *pLayoutConsole = layout_create(2,1);
    TextView    *pTextView      = textview_create();
    Edit        *pEditInput     = edit_create();
    Button      *pButtonSubmit  = button_push();

    pApp->pDebugData->pConsoleTextView  = pTextView;
    pApp->pDebugData->pConsoleInput     = pEditInput;

    /* Text View: Console */
    textview_wrap(pTextView, TRUE);

    /* Button: Submit */
    button_text(pButtonSubmit, TXT_BTN_SUBMIT);
    button_tag(pButtonSubmit, MODAL_ACCEPT);
    button_OnClick(pButtonSubmit, listener(pApp, Callback_OnButtonModalDebug, AppGUI));

    /* Layout: Console */
    layout_hsize(pLayoutConsole, 0, 384);
    layout_hsize(pLayoutConsole, 1, 128);
    layout_edit(pLayoutConsole, pEditInput, 0, 0);
    layout_button(pLayoutConsole, pButtonSubmit, 1, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, 512);
    layout_textview(pLayoutMain, pTextView, 0, 0);
    layout_layout(pLayoutMain, pLayoutConsole, 0,1);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

static void
_Callback_OnButtonModalModManager(AppGUI *pApp, Event *e)
{
    Button *pButton             = event_sender(e, Button);
    const uint32_t dButtonTag   = button_get_tag(pButton);

    /* Gather all data from ui widgets */
    if (dButtonTag == MODAL_ACCEPT)
    {
        const ListBox *pListbox = (const ListBox*)pApp->pWidgets->pListBox;
        unsigned int i;

        for(i = 0; i < MAX_MOD_ELEMS; i++)
        {
            GUIModElement *pElem = &pApp->pElementSets->pModElementArray[i];

            if (str_empty(pElem->sID))
            {
                continue;
            }

            pElem->bActive = listbox_checked(pListbox, i);
        }
    }

    pApp->pWindows->pLayoutExtra  = NULL;
    pApp->pWidgets->pListBox      = NULL;
    window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
}

Panel*
Panel_GetModalMessage(AppGUI* pApp)
{
    Panel   *pPanelMain     = panel_create();
    Layout  *pLayoutMain    = layout_create(1,2);
    Layout  *pLayoutButton  = layout_create(1,1);
    Label   *pLabelMessage  = label_create();
    Button  *pButtonOK      = button_push();

    /* Label: Message */
    label_text(pLabelMessage, TXT_NULL);
    label_multiline(pLabelMessage, TRUE);
    if (pApp)
    {
        if (pApp->pWidgets->pString)
        {
            label_text(pLabelMessage, tc(pApp->pWidgets->pString));
        }
    }

    /* Button: OK */
    button_text(pButtonOK, TXT_BTN_OK);
    button_tag(pButtonOK, MODAL_ACCEPT);
    button_OnClick(pButtonOK, listener(pApp, Callback_OnButtonModalMessage, AppGUI));

    /* Layout: Button */
    layout_hsize(pLayoutButton, 0, IMAGE_DEFAULT_W / 2.f);
    layout_vsize(pLayoutButton, 0, IMAGE_DEFAULT_H / 2.f);
    layout_valign(pLayoutButton, 0, 0, ekCENTER);
    layout_margin(pLayoutButton, LAYOUT_DEFAULT_MARGIN);
    layout_button(pLayoutButton, pButtonOK, 0, 0);

    /* Layout: Main */
    layout_margin(pLayoutMain, LAYOUT_DEFAULT_MARGIN);
    layout_label(pLayoutMain, pLabelMessage,0,0);
    layout_layout(pLayoutMain, pLayoutButton, 0, 1);

    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

Panel*
Panel_GetModalQuestion(AppGUI* pApp)
{
    Panel   *pPanelMain     = panel_create();
    Layout  *pLayoutMain    = layout_create(1,2);
    Layout  *pLayoutMessage = layout_create(1,1);
    Layout  *pLayoutButtons = layout_create(4,1);
    Label   *pLabelMessage  = label_create();
    Button  *pButtonContinue= button_push();
    Button  *pButtonCancel  = button_push();

    /* Label: Message */
    label_text(pLabelMessage, TXT_NULL);
    label_multiline(pLabelMessage, TRUE);
    if (pApp)
    {
        if (pApp->pWidgets->pString)
        {
            label_text(pLabelMessage, tc(pApp->pWidgets->pString));
        }
    }

    /* Button: OK */
    button_text(pButtonContinue, TXT_BTN_ACCEPT);
    button_tag(pButtonContinue, MODAL_ACCEPT);
    button_OnClick(pButtonContinue, listener(pApp, Callback_OnButtonModalQuestion, AppGUI));

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalQuestion, AppGUI));

    /* Layout: Message */
    layout_hsize(pLayoutMessage, 0, IMAGE_DEFAULT_W / 2.f);
    layout_vsize(pLayoutMessage, 0, IMAGE_DEFAULT_H / 3.f);
    layout_valign(pLayoutMessage, 0, 0, ekTOP);
    layout_margin(pLayoutMessage, LAYOUT_DEFAULT_MARGIN);
    layout_label(pLayoutMessage, pLabelMessage,0,0);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    layout_halign(pLayoutButtons, 3, 0, ekRIGHT);
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonContinue, 3, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, IMAGE_DEFAULT_W / 2.f);
    layout_vsize(pLayoutMain, 0, IMAGE_DEFAULT_H / 3.f);
    layout_margin(pLayoutMain, LAYOUT_DEFAULT_MARGIN);
    layout_layout(pLayoutMain, pLayoutMessage,0,0);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 1);

    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

Panel*
Panel_GetModalGameNotFound(AppGUI* pApp)
{
    Panel       *pPanelMain     = panel_create();
    Layout      *pLayoutMain    = layout_create(1,3);
    Layout      *pLayoutPath    = layout_create(2,1);
    Layout      *pLayoutTxt     = layout_create(1,1);
    Layout      *pLayoutButtons = layout_create(4, 1);
    Layout      *pLayoutEdit    = layout_create(1,1);
    Layout      *pLayoutBrowse  = layout_create(1,1);
    Edit        *pEditPath      = edit_create();
    Label       *pLabelInfo     = label_create();
    Button      *pButtonSubmit  = button_push();
    Button      *pButtonCancel  = button_push();
    Button      *pButtonBrowse  = button_push();
    Font        *pFontInfo      = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFontSubmit    = font_system(20, ekFNORMAL | ekFPIXELS);

    /* Label: Greetings Text */
    label_text(pLabelInfo, TXT_GAMENOTFOUND);
    label_multiline(pLabelInfo, TRUE);
    label_font(pLabelInfo, pFontInfo);

    /* Edit: Game Path */
    pApp->pWidgets->pEdit = pEditPath;

    /* Button: Submit */
    button_text(pButtonSubmit, TXT_BTN_SUBMIT);
    button_font(pButtonSubmit, pFontSubmit);
    button_vpadding (pButtonSubmit, 16);
    button_tag(pButtonSubmit, MODAL_ACCEPT);
    button_min_width(pButtonSubmit, _fMinBtnWidth);
    button_OnClick(pButtonSubmit, listener(pApp, Callback_OnButtonModalGameNotFound, AppGUI));

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_font(pButtonCancel, pFontSubmit);
    button_vpadding (pButtonCancel, 16);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_min_width(pButtonCancel, _fMinBtnWidth);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalGameNotFound, AppGUI));

    /* Button: Browse (file) */
    button_text(pButtonBrowse, TXT_BTN_BROWSE);
    button_font(pButtonBrowse, pFontInfo);
    button_tag(pButtonBrowse, MODAL_BROWSE);
    button_OnClick(pButtonBrowse, listener(pApp, Callback_OnButtonModalGameNotFound, AppGUI));

    /* Layout: Text */
    layout_hsize(pLayoutTxt, 0, IMAGE_DEFAULT_W);
    layout_vsize(pLayoutTxt, 0, IMAGE_DEFAULT_H / 2.f);
    layout_valign(pLayoutTxt, 0, 0, ekTOP);
    layout_margin(pLayoutTxt, 4);
    layout_label(pLayoutTxt, pLabelInfo, 0, 0);

    /* Layout: Game Path Edit widget */
    layout_margin4(pLayoutEdit, _fDefMarg, _fDefMarg, _fDefMarg, 0.f);
    layout_edit(pLayoutEdit, pEditPath, 0,0);

    /* Layout: Game Path Browse Button widget */
    layout_margin4(pLayoutBrowse, _fDefMarg, 0.f, _fDefMarg, _fDefMarg);
    layout_button(pLayoutBrowse, pButtonBrowse, 0, 0);

    /* Layout: Game Path */
    layout_hsize(pLayoutPath, 0, IMAGE_DEFAULT_W / 2.f);
    layout_margin(pLayoutPath, LAYOUT_DEFAULT_MARGIN);
    layout_layout(pLayoutPath, pLayoutEdit, 0,0);
    layout_layout(pLayoutPath, pLayoutBrowse, 1, 0);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    layout_halign(pLayoutButtons, 3, 0, ekRIGHT);
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonSubmit, 3, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, IMAGE_DEFAULT_W);
    layout_vsize(pLayoutMain, 0, IMAGE_DEFAULT_H / 2.f);
    layout_layout(pLayoutMain, pLayoutTxt, 0, 0);
    layout_layout(pLayoutMain, pLayoutPath, 0, 1);
    layout_layout(pLayoutMain,pLayoutButtons,0, 2);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    if (pApp->pWindows->pWindowModal != NULL)
    {
        window_defbutton(pApp->pWindows->pWindowModal, pButtonSubmit);
    }

    /* Cleanup */
    font_destroy(&pFontInfo);
    font_destroy(&pFontSubmit);

    return pPanelMain;
}

static Panel*
_Panel_TweakerDescription(AppGUI *pApp)
{
    const GUITweaker* pTweaker;
    Panel       *pPanelMain         = panel_create();
    Layout      *pLayoutMain        = layout_create(1,1);
    Label       *pLabelDescription  = label_create();

    pTweaker = &pApp->tGUITweaker[pApp->dPage];
    assert(IS_VALID(pTweaker));

    /* Label: Feature Description */
    label_multiline(pLabelDescription, TRUE);
    label_text(pLabelDescription, tc(pTweaker->sTweakerInfo));

    /* Layout: Main */
    layout_vexpand(pLayoutMain, 0);
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_margin(pLayoutMain, LAYOUT_DEFAULT_MARGIN);
    layout_hsize(pLayoutMain, 0, PANEL_DEFAULT_W - (_fDefMarg * 2.f));
    layout_vsize(pLayoutMain, 0, PANEL_DEFAULT_H / 5.f);
    layout_label(pLayoutMain, pLabelDescription, 0, 0);

    /* Panel: Main */
    panel_size(pPanelMain, s2df(
        PANEL_DEFAULT_W - (_fDefMarg * 2.f), 
        (PANEL_DEFAULT_H / 5.f) - (_fDefMarg * 4.f))
    );
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

static Panel*
_Panel_TweakerOptionsSelector(AppGUI *pApp)
{
    const GUITweaker* pTweaker;
    Panel       *pPanelMain         = panel_create();
    Layout      *pLayoutMain        = layout_create(APP_MAX_ELEMENTS,1);
    Label       *pLabelError;
    Button      *pButtonOpt[APP_MAX_ELEMENTS];
    unsigned int i;

    pTweaker = &pApp->tGUITweaker[pApp->dPage];
    assert(IS_VALID(pTweaker));

    /* Buttons: Tweak Options */
    for (i = 0; i < pTweaker->dMaxOptions; i++)
    {
        if (!str_empty(pTweaker->sImageTitle[i]))
        {
            pButtonOpt[i] = button_radio();
            button_text(pButtonOpt[i], tc(pTweaker->sImageTitle[i]));
            button_tag(pButtonOpt[i], (MODAL_OPT_A + i) );
            button_OnClick(pButtonOpt[i], listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
        }
    }

    /* Set active button */
    if (pTweaker->sImageTitle[pTweaker->dSelectedOption])
    {
        button_state(pButtonOpt[pTweaker->dSelectedOption], ekGUI_ON);
    }
    else if (pTweaker->sImageTitle[0])
    {
        button_state(pButtonOpt[0], ekGUI_ON);
    }

    /* Layout: Selector */
    for (i = 0; i < pTweaker->dMaxOptions; i++)
    {
        if (!str_empty(pTweaker->sImageTitle[i]))
        {
            layout_button(pLayoutMain, pButtonOpt[i], i,0);
            layout_hsize(pLayoutMain, i, 150);
            layout_halign(pLayoutMain, i, 0, ekLEFT);
        }
    }
    layout_hmargin(pLayoutMain, 0, 5);
    layout_margin4(pLayoutMain, 5, 10, 10, 10);
    layout_skcolor(pLayoutMain, gui_line_color());

    /* Label: Error about option absence */
    if (str_empty(pTweaker->sImageTitle[0]))
    {
        pLabelError = label_create();
        label_text(pLabelError, "Error: No options provided!");
        layout_label(pLayoutMain, pLabelError, 0, 0);
    }

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

/* 0 = single page, 1 = first, 2 = middle, 3 = last               */
static unsigned int
_RowForPage(unsigned int dPage, unsigned int dPageMax)
{
    if (dPageMax == 1)              return 0;       /* single page  */
    if (dPage == 0)                 return 1;       /* first page   */
    if (dPage == dPageMax - 1)      return 3;       /* last page    */
    return 2;                                       /* middle page  */
}

static Panel*
_Panel_TweakerButtons(AppGUI *pApp)
{
    Panel       *pPanelMain             = panel_create();
    Layout      *pLayoutMain            = layout_create(2,1);
    Layout      *pLayoutDynamicButtons  = layout_create(1,4);
    Layout      *pLayoutSinglePage      = layout_create(6,1);
    Layout      *pLayoutFirstPage       = layout_create(6,1);
    Layout      *pLayoutNPage           = layout_create(6,1);
    Layout      *pLayoutLastPage        = layout_create(6,1);
    Button      *pButtonCancel          = button_push();
    Button      *pButtonPrev            = button_push();
    Button      *pButtonLastPrev        = button_push();
    Button      *pButtonFirstNext       = button_push();
    Button      *pButtonNext            = button_push();
    Button      *pButtonConfirm         = button_push();
    Button      *pButtonFirstConfirm    = button_push();
    const unsigned int dRowForPage = 
        _RowForPage(pApp->dPage, pApp->dPageMax);
    unsigned int dRow;

    pApp->pWindows->pLayoutExtra  = pLayoutDynamicButtons;

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonCancel, _fMinBtnWidth);

    /* Button: Confirm */
    button_text(pButtonConfirm, TXT_BTN_ACCEPT);
    button_tag(pButtonConfirm, MODAL_ACCEPT);
    button_OnClick(pButtonConfirm, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonConfirm, _fMinBtnWidth);

    button_text(pButtonFirstConfirm, TXT_BTN_ACCEPT);
    button_tag(pButtonFirstConfirm, MODAL_ACCEPT);
    button_OnClick(pButtonFirstConfirm, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonFirstConfirm, _fMinBtnWidth);

    /* Button: Previous */
    button_text(pButtonPrev, TXT_BTN_PREVIOUS);
    button_tag(pButtonPrev, MODAL_PREVIOUS);
    button_OnClick(pButtonPrev, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonPrev, _fMinBtnWidth);

    button_text(pButtonLastPrev, TXT_BTN_PREVIOUS);
    button_tag(pButtonLastPrev, MODAL_PREVIOUS);
    button_OnClick(pButtonLastPrev, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonLastPrev, _fMinBtnWidth);

    /* Button: Next */
    button_text(pButtonFirstNext, TXT_BTN_NEXT);
    button_tag(pButtonFirstNext, MODAL_NEXT);
    button_OnClick(pButtonFirstNext, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonFirstNext, _fMinBtnWidth);

    button_text(pButtonNext, TXT_BTN_NEXT);
    button_tag(pButtonNext, MODAL_NEXT);
    button_OnClick(pButtonNext, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
    button_min_width(pButtonNext, _fMinBtnWidth);

    /* Layout: Single Page */
    layout_button(pLayoutSinglePage, pButtonFirstConfirm, 5,0);

    /* Layout: First Page */
    layout_button(pLayoutFirstPage, pButtonFirstNext, 5, 0);

    /* Layout: N Page */
    layout_button(pLayoutNPage, pButtonPrev, 4, 0);
    layout_button(pLayoutNPage, pButtonNext, 5, 0);

    /* Layout: Last Page */
    layout_button(pLayoutLastPage, pButtonLastPrev, 4, 0);
    layout_button(pLayoutLastPage, pButtonConfirm, 5, 0);

    /* Layout: Dynamic Buttons */
    layout_hsize(pLayoutDynamicButtons, 0, IMAGE_DEFAULT_W - _fMinBtnWidth);
    layout_layout(pLayoutDynamicButtons, pLayoutSinglePage, 0, 0);
    layout_layout(pLayoutDynamicButtons, pLayoutFirstPage, 0, 1);
    layout_layout(pLayoutDynamicButtons, pLayoutNPage, 0, 2);
    layout_layout(pLayoutDynamicButtons, pLayoutLastPage, 0, 3);

    /* Layout: Main */
    layout_margin(pLayoutMain, LAYOUT_DEFAULT_MARGIN);
    layout_button(pLayoutMain, pButtonCancel, 0, 0);
    layout_layout(pLayoutMain, pLayoutDynamicButtons, 1, 0);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    /* Show appropriate button layout */
    for (dRow = 0; dRow < 4; ++dRow)
    {
        layout_show_row(pApp->pWindows->pLayoutExtra, dRow, dRow == dRowForPage);
    }

    return pPanelMain;
}

Panel*
Panel_GetModalTweaker(AppGUI *pApp)
{
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelDescription  = _Panel_TweakerDescription(pApp);
    Panel       *pPanelSelector     = _Panel_TweakerOptionsSelector(pApp);
    Panel       *pPanelImage        = Panel_GetImageDemo(pApp);
    Panel       *pPanelButtons      = _Panel_TweakerButtons(pApp);
    Layout      *pLayoutMain        = layout_create(1,5);

    pApp->pWindows->pLayoutModalMain = pLayoutMain;

    /* Layout: Main */
    /* layout_hsize(pLayoutMain, 0, TITLE_PNG_W);
    layout_vsize(pLayoutMain, 0, TITLE_PNG_H);
    layout_halign(pLayoutMain, 0, 0, ekJUSTIFY);
    layout_valign(pLayoutMain, 0, 0, ekJUSTIFY);*/
    layout_halign(pLayoutMain, 0, 0, ekJUSTIFY);
    layout_valign(pLayoutMain, 0, 0, ekJUSTIFY);

    /* Panels */
    layout_panel(pLayoutMain, pPanelDescription, 0, 0);
    layout_panel(pLayoutMain, pPanelSelector, 0, 1);
    layout_panel(pLayoutMain, pPanelImage, 0, 2);
    layout_panel(pLayoutMain, pPanelButtons, 0, 3);

    /* Panel: Main */
    panel_size(pPanelMain, s2df(PANEL_DEFAULT_W, PANEL_DEFAULT_H));
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

static void
_Callback_LocalisationPopupSelector(AppGUI *pApp, Event *e)
{
    /* Since there's no way to "tag" popups, just acquire data from all widgets */
    pApp->pLocalizations->dLocaleSelected[LOC_CORE] = popup_get_selected(pApp->pWidgets->pPopupCore);
    pApp->pLocalizations->dLocaleSelected[LOC_MOD]  = popup_get_selected(pApp->pWidgets->pPopupMod);

    view_update(pApp->pWidgets->pLocaleView);

    unref(e);

    /* gui_language(lang); */
}

static void 
_LocalisationFillPopup(AppGUI *pApp, PopUp *pPop, ELocTier eTier)
{
    size_t i;

    popup_clear(pPop);
    for (i = 0; i < pApp->pLocalizations->dLocaleCount; ++i)
    {
        const LangInfo *li = &pApp->pLocalizations->tLocale[i];
        bool_t bOk = (eTier == LOC_CORE  && li->bCore ) ||
                 (eTier == LOC_MOD   && li->bMod  );

        if (bOk)
        {
            popup_add_elem(pPop, li->sCode, NULL);
        }
    }
    popup_selected(pPop, 0);
}

Panel*
Panel_GetModalLocalisation(AppGUI *pApp)
{
    Panel       *pPanelMain         = panel_create();
    /* Panel       *pPanelImage        = Panel_GetImageDemo(pApp); */
    Layout      *pLayoutMain        = layout_create(1,6);
    Layout      *pLayoutDescription = layout_create(1,1);
    Layout      *pLayoutPopups      = layout_create(3,2);
    Layout      *pLayoutButtons     = layout_create(4,1);
    Label       *pLabelDescription  = label_create();
    Label       *pLabelLocCore      = label_create();
    Label       *pLabelLocMod       = label_create();
    View        *pViewPreview       = view_create();
    Button      *pButtonAccept      = button_push();
    Button      *pButtonCancel      = button_push();
    PopUp       *pPopUpCore         = popup_create();
    PopUp       *pPopUpMod          = popup_create();

    pApp->pWidgets->pPopupCore      = pPopUpCore;
    pApp->pWidgets->pPopupMod       = pPopUpMod;
    pApp->pWidgets->pLocaleView     = pViewPreview;

    /* Label: Description */
    label_multiline(pLabelDescription, TRUE);
    label_text(pLabelDescription, TXT_LOCALISATION);

    /* Label: Languages */
    label_text(pLabelLocCore, TXT_LOC_CORE);
    label_text(pLabelLocMod, TXT_LOC_MOD);
    label_align(pLabelLocCore, ekLEFT);
    label_align(pLabelLocMod, ekLEFT);

    /* Popups: Languages */
    _LocalisationFillPopup(pApp, pPopUpCore,  LOC_CORE);
    _LocalisationFillPopup(pApp, pPopUpMod,   LOC_MOD);

    /* gui_image(USA_PNG) */
    /* popup_add_elem(pPopUpCore, TXT_LOC_ENGLISH, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_SPANISH, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_FRENCH, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_POLISH, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_CZECH, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_HUNGARIAN, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_UKRAINIAN, NULL);
    popup_add_elem(pPopUpCore, TXT_LOC_RUSSIAN, NULL); */

    popup_OnSelect(pPopUpCore, listener(pApp, _Callback_LocalisationPopupSelector, AppGUI));

    /* popup_add_elem(pPopUpMod, TXT_LOC_ENGLISH, NULL); */
    popup_OnSelect(pPopUpMod, listener(pApp, _Callback_LocalisationPopupSelector, AppGUI));

    /* View: Localisation image preview */
    view_size(pViewPreview, s2df(640, 360));
    view_OnDraw(pViewPreview, listener(pApp, Callback_OnDrawLocalisation, AppGUI));

    /* Button: Accept */
    button_text(pButtonAccept, TXT_BTN_ACCEPT);
    button_tag(pButtonAccept, MODAL_ACCEPT);
    button_min_width(pButtonAccept, _fMinBtnWidth);
    button_OnClick(pButtonAccept, listener(pApp, Callback_OnButtonModalLocalisation, AppGUI));

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_min_width(pButtonCancel, _fMinBtnWidth);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalLocalisation, AppGUI));

    /* Layout: Description */
    layout_margin(pLayoutDescription, LAYOUT_DEFAULT_MARGIN);
    layout_hsize(pLayoutDescription, 0, PANEL_DEFAULT_W - (LAYOUT_DEFAULT_MARGIN * 2.f));
    layout_valign(pLayoutDescription, 0, 0, ekTOP);
    layout_label(pLayoutDescription, pLabelDescription, 0, 0);

    /* Layout: Popups */
    layout_margin(pLayoutPopups, LAYOUT_DEFAULT_MARGIN);
    layout_halign(pLayoutPopups, 0, 0, ekLEFT);
    layout_halign(pLayoutPopups, 1, 0, ekLEFT);
    layout_hsize(pLayoutPopups, 0, PANEL_DEFAULT_W / 2.f);
    layout_hsize(pLayoutPopups, 1, PANEL_DEFAULT_W / 2.f);
    layout_label(pLayoutPopups, pLabelLocCore, 0, 0);
    layout_label(pLayoutPopups, pLabelLocMod, 1, 0);
    layout_popup(pLayoutPopups, pPopUpCore, 0, 1);
    layout_popup(pLayoutPopups, pPopUpMod, 1, 1);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    layout_halign(pLayoutButtons, 3, 0, ekRIGHT);
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonAccept, 3, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, PANEL_DEFAULT_W);

    layout_layout(pLayoutMain, pLayoutDescription, 0, 0);
    layout_layout(pLayoutMain, pLayoutPopups, 0,1);
    layout_view(pLayoutMain, pViewPreview, 0, 4);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 5);
    /* layout_panel(pLayoutMain, pPanelImage, 0, 4); */

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

static Panel*
_Panel_GetModalOptionSubpanel(AppGUI *pApp)
{
    Panel       *pPanelMain  = panel_scroll(FALSE,TRUE);
    Layout      *pLayoutMain = layout_create(1,MAX_OPT_ELEMS);
    Font        *pFontBold   = font_system(16, ekFBOLD | ekFPIXELS);

    unsigned int i      = 0;
    unsigned int j      = 0;
    unsigned int dRow   = 0;

    const unsigned int dCorruptedRow = 999;

    GUIOptElement *pElem;

    cassert_no_null(pApp->pElementSets->pOptElementArray);

    /* Layout: Main */
    layout_skcolor(pLayoutMain, gui_line_color());
    layout_margin4(pLayoutMain, _fDefMarg, _fDefMarg * 4.f, _fDefMarg, _fDefMarg);

    for(i = 0; i < MAX_OPT_ELEMS; i ++)
    {
        Label  *pLabel          = label_create();
        Layout *pLayoutLabelRow = layout_create(2, 1);
        unsigned int dCurRow    = dRow;

        pElem = &pApp->pElementSets->pOptElementArray[i];
        assert(IS_VALID(pElem));

        /* Default row widgets */
        label_font(pLabel, pFontBold);
        label_text(pLabel, tc(pElem->sTitle));

        layout_halign(pLayoutLabelRow, 0, 0, ekLEFT);
        layout_hexpand(pLayoutLabelRow, 0);
        layout_hsize(pLayoutLabelRow, 0, 384.f);
        layout_margin4(pLayoutLabelRow, dRow > 0 ? _fDefMarg : 0.f, 0.f, _fDefMarg, 0.f);
        layout_label(pLayoutLabelRow, pLabel, 0, 0);

        /* Custom widgets */
        switch(pElem->eType)
        {
            case UI_WIDGET_RADIO:
                {
                    Layout *pLayoutButton   = layout_create(pElem->dNumOfOptions*2, 1);
                    Layout *pLayoutExtraRow = layout_create(1,1);

                    /* Store layout instead of button because there are multiple buttons present there */
                    pElem->pElement = pLayoutButton;

                    layout_margin4(pLayoutButton, 0.f, LAYOUT_DEFAULT_MARGIN, 0.f, 0.f);

                    for(j = 0; j < pElem->dNumOfOptions; j++)
                    {
                        Label *pLabelOpt    = label_create();
                        Button *pButton     = button_radio();
                        unsigned int pos    = j * 2;

                        label_text(pLabelOpt, tc(pElem->sOptTitle[j]));
                        label_align(pLabelOpt, ekLEFT);

                        button_tag(pButton, (MODAL_OPT_A + i) );
                        if (pElem->dChoice == j)
                        {
                            button_state(pButton, ekGUI_ON);
                        }

                        layout_label(pLayoutButton, pLabelOpt, pos, 0);
                        layout_button(pLayoutButton, pButton, pos + 1, 0);

                        layout_hexpand(pLayoutButton, pos);
                        layout_hexpand(pLayoutButton, pos+1);
                        layout_halign(pLayoutButton, pos, 0, ekLEFT);
                        layout_halign(pLayoutButton, pos+1, 0, ekLEFT);
                        layout_hsize(pLayoutButton, pos, 128);
                        layout_hsize(pLayoutButton, pos+1, 64);
                    }

                    
                    layout_margin(pLayoutButton, LAYOUT_DEFAULT_MARGIN);
                    layout_skcolor(pLayoutButton, gui_line_color());

                    layout_hsize(pLayoutExtraRow, 0, 512);
                    layout_layout(pLayoutExtraRow, pLayoutButton, 0, 0);
                    layout_layout(pLayoutMain, pLayoutExtraRow, 0, dRow+1);

                    dRow += 2;
                }
                break;
            case UI_WIDGET_CHECKBOX:
                {
                    Button *pCheckbox  = button_check();

                    pElem->pElement    = pCheckbox;

                    button_text(pCheckbox, tc(pElem->sOptTitle[0]));

                    layout_halign(pLayoutLabelRow, 1, 0, ekRIGHT);
                    layout_button(pLayoutLabelRow, pCheckbox, 1, 0);

                    if (pElem->dChoice > 0)
                    {
                        button_state(pCheckbox, ekGUI_ON);
                    }

                    dRow++;
                }
                break;
            case UI_WIDGET_POPUP:
                {
                    PopUp *pPopup = popup_create();

                    pElem->pElement = pPopup;

                    for(j = 0; j < pElem->dNumOfOptions; j++)
                    {
                        popup_add_elem(pPopup, tc(pElem->sOptTitle[j]), NULL);
                    }

                    popup_selected(pPopup, pElem->dChoice);
                    layout_popup(pLayoutLabelRow, pPopup, 1, 0);
                    layout_hsize(pLayoutLabelRow, 1, 192.f);

                    dRow++;
                }
                break;
            case UI_WIDGET_EDIT:
                {
                    Edit  *pEdit    = edit_create();

                    pElem->pElement = pEdit;

                    edit_text(pEdit, tc(pElem->sOptTitle[0]));

                    layout_edit(pLayoutLabelRow, pEdit, 1, 0);
                    layout_hsize(pLayoutLabelRow, 1, 192.f);

                    dRow++;
                }
                break;
            case UI_WIDGET_LISTBOX:
                {
                    ListBox *pListbox        = listbox_create();
                    Layout  *pLayoutExtraRow = layout_create(1,1);

                    pElem->pElement = pListbox;

                    listbox_checkbox(pListbox, TRUE);
                    for(j = 0; j < pElem->dNumOfOptions; j++)
                    {
                        const bool_t bCheckbox = (pElem->dChoice & (1U << j)) != 0;

                        listbox_add_elem(pListbox, tc(pElem->sOptTitle[j]), NULL);
                        listbox_check(pListbox, j, bCheckbox);
                    }

                    layout_listbox(pLayoutLabelRow, pListbox, 0, dRow+1);

                    layout_hsize(pLayoutExtraRow, 0, 256.f);
                    layout_listbox(pLayoutExtraRow, pListbox, 0, 0);
                    layout_layout(pLayoutMain, pLayoutExtraRow, 0, dRow+1);

                    dRow += 2;
                }
                break;
            case UI_WIDGET_NULL:
            case UI_WIDGET_MAX:
            default:
                dCurRow = dCorruptedRow; /* wrong/corrupted widget */
                break;
        }

        if (dCurRow != dCorruptedRow)
        {
            layout_layout(pLayoutMain, pLayoutLabelRow, 0, dCurRow);
        }
    }

    /* Panel: Main */
    panel_size(pPanelMain,
        s2df(0.f, PANEL_DEFAULT_H - (PANEL_DEFAULT_H / 8.f)));
    panel_layout(pPanelMain, pLayoutMain);

    /* cleanup */
    font_destroy(&pFontBold);

    return pPanelMain;
}

Panel*
Panel_GetModalOptions(AppGUI *pApp)
{
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelSubpanel     = _Panel_GetModalOptionSubpanel(pApp);
    Layout      *pLayoutMain        = layout_create(1,2);
    Layout      *pLayoutButtons     = layout_create(4,1);
    Button      *pButtonAccept      = button_push();
    Button      *pButtonCancel      = button_push();

    /* Button: Accept */
    button_text(pButtonAccept, TXT_BTN_ACCEPT);
    button_tag(pButtonAccept, MODAL_ACCEPT);
    button_OnClick(pButtonAccept, listener(pApp, Callback_OnButtonModalOptions, AppGUI));
    button_min_width(pButtonAccept, 128.f);

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalOptions, AppGUI));
    button_min_width(pButtonCancel, 128.f);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    layout_halign(pLayoutButtons, 3, 0, ekRIGHT);
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonAccept, 3, 0);
    
    /* Layout: Main */
    layout_margin(pLayoutMain, LAYOUT_DEFAULT_MARGIN);
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_valign(pLayoutMain, 0, 1, ekBOTTOM);
    layout_panel(pLayoutMain, pPanelSubpanel, 0,0);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 1);

    /* Panel: Main */
    panel_size(pPanelMain,s2df(PANEL_DEFAULT_W, PANEL_DEFAULT_H));
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

static void
_Callback_OnLabelModManagerSubpanel(String *pLabelText, Event *e)
{
    osapp_open_url(tc(pLabelText));
    unref(e);
}

static Panel*
_Panel_GetModalModManagerSubpanel(AppGUI *pApp, const unsigned int dModIndex)
{
    Panel       *pPanelMain         = panel_scroll(FALSE,TRUE);
    Layout      *pLayoutMain        = layout_create(1,1);
    Layout      *pLayoutScreen      = layout_create(1,1);
    Layout      *pLayoutModPreview  = layout_create(1, 6);
    Layout      *pLayoutDescription = layout_create(1,1);
    Label       *pLabelModTitle     = label_create();
    Label       *pLabelModAuthor    = label_create();
    Label       *pLabelModDesc      = label_create();
    Label       *pLabelModVersion   = label_create();
    Label       *pLabelModWebsite   = label_create();
    ImageView   *pImageViewScreen   = imageview_create();
    Font        *pFontBold          = font_system(16, ekFBOLD | ekFPIXELS);
    Font        *pFontItalic        = font_system(16, ekFITALIC | ekFPIXELS );
    Image       *pImage;
    ferror_t    eError;

    GUIModElement *pElem;

    cassert_no_null(pApp->pElementSets->pModElementArray);
    pElem = &pApp->pElementSets->pModElementArray[dModIndex];
    cassert_no_null(pElem);

    /* Labels: Mod credentials */
    label_text(pLabelModTitle, tc(pElem->sName));
    label_font(pLabelModTitle, pFontBold);
    label_text(pLabelModAuthor, tc(pElem->sAuthor));
    label_font(pLabelModAuthor, pFontItalic);
    label_text(pLabelModDesc, tc(pElem->sDescription));
    label_text(pLabelModVersion, tc(pElem->sVersion));
    label_font(pLabelModVersion, pFontItalic);
    label_text(pLabelModWebsite, tc(pElem->sWebsite));
    label_multiline(pLabelModWebsite, TRUE);
    label_multiline(pLabelModDesc, TRUE);
    label_style_over(pLabelModWebsite, ekFUNDERLINE);
    label_OnClick(pLabelModWebsite, listener(pElem->sWebsite, _Callback_OnLabelModManagerSubpanel, String));

    /* Image View: Mod Preview */
    pImage = image_from_file(tc(pElem->sScreenshot), &eError);
    imageview_image(pImageViewScreen, eError == ekFOK ? pImage : (const Image*)TITLE_JPG);
    imageview_size(pImageViewScreen, s2df(PANEL_DEFAULT_W / 2.f, 240));
    imageview_scale(pImageViewScreen, ekGUI_SCALE_ADJUST);

    /* Layout: Screenshot */
    layout_margin(pLayoutScreen, 0.f);
    layout_hsize(pLayoutScreen, 0, PANEL_DEFAULT_W / 2.f);
    layout_imageview(pLayoutScreen, pImageViewScreen, 0, 0);

    /* Layout: Mod Description */
    layout_hsize(pLayoutDescription, 0, (PANEL_DEFAULT_W / 2.f) - (_fDefMarg * 2.f));
    layout_margin4(pLayoutDescription, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_skcolor(pLayoutDescription, gui_line_color());
    layout_label(pLayoutDescription, pLabelModDesc, 0, 0);

    /* Layout: Mod Preview */
    layout_hsize(pLayoutModPreview, 0, PANEL_DEFAULT_W);
    layout_vsize(pLayoutModPreview, 0, 240);
    layout_vsize(pLayoutModPreview, 1, 32);
    layout_vsize(pLayoutModPreview, 2, 32);
    layout_vsize(pLayoutModPreview, 3, 32);
    layout_vsize(pLayoutModPreview, 4, 32);
    layout_valign(pLayoutModPreview, 0, 0, ekTOP);
    layout_valign(pLayoutModPreview, 0, 1, ekTOP);
    layout_valign(pLayoutModPreview, 0, 2, ekTOP);
    layout_valign(pLayoutModPreview, 0, 3, ekTOP);
    layout_halign(pLayoutModPreview, 0, 0, ekLEFT);
    layout_halign(pLayoutModPreview, 0, 1, ekLEFT);
    layout_halign(pLayoutModPreview, 0, 2, ekLEFT);
    layout_halign(pLayoutModPreview, 0, 3, ekLEFT);
    layout_halign(pLayoutModPreview, 0, 4, ekLEFT);
    
    
    layout_layout(pLayoutModPreview, pLayoutScreen, 0, 0);
    layout_label(pLayoutModPreview, pLabelModTitle, 0, 1);
    layout_label(pLayoutModPreview, pLabelModVersion, 0, 2);
    layout_label(pLayoutModPreview, pLabelModAuthor, 0, 3);
    layout_label(pLayoutModPreview, pLabelModWebsite, 0, 4);
    layout_layout(pLayoutModPreview, pLayoutDescription, 0, 5);

    /* Layout: Main */
    layout_margin4(pLayoutMain, 0, _fDefMarg, 0, _fDefMarg);
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_hsize(pLayoutModPreview, 0, PANEL_DEFAULT_W / 2.f);
    layout_layout(pLayoutMain, pLayoutModPreview, 0, 0);

    /* Panel: Main */
    panel_size(pPanelMain, 
        s2df((PANEL_DEFAULT_W / 2.f), PANEL_DEFAULT_H));
    panel_layout(pPanelMain, pLayoutMain);

    /* cleanup */
    font_destroy(&pFontBold);
    font_destroy(&pFontItalic);
    if (IS_VALID(pImage))
    {
        image_destroy(&pImage);
    }

    return pPanelMain;
}

static void
_Callback_OnModManagerListbox(AppGUI *pApp, Event *e)
{
    const EvButton *p = event_params(e, EvButton);

    if (p->index >= MAX_MOD_ELEMS)
    {
        return;
    }

    if (str_empty(pApp->pElementSets->pModElementArray[p->index].sID))
    {
        return;
    }

    layout_panel_replace(
        pApp->pWindows->pLayoutExtra,
        _Panel_GetModalModManagerSubpanel(pApp,p->index),
        1, 0);
    unref(e);
}

Panel*
Panel_GetModalModManager(AppGUI* pApp)
{
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelModPreview   = _Panel_GetModalModManagerSubpanel(pApp, 0);
    Layout      *pLayoutMain        = layout_create(1,2);
    Layout      *pLayoutButtons     = layout_create(2,1);
    Layout      *pLayoutCore        = layout_create(2,1);
    Layout      *pLayoutListboxes   = layout_create(1,1);
    Button      *pButtonCancel      = button_push();
    Button      *pButtonOK          = button_push();
    ListBox     *pListboxMods       = listbox_create();

    unsigned int i;

    pApp->pWindows->pLayoutExtra    = pLayoutCore;
    pApp->pWidgets->pListBox        = pListboxMods;

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, _Callback_OnButtonModalModManager, AppGUI));
    button_min_width(pButtonCancel, 128.f);

    /* Button: OK */
    button_text(pButtonOK, TXT_BTN_ACCEPT);
    button_tag(pButtonOK, MODAL_ACCEPT);
    button_OnClick(pButtonOK, listener(pApp, _Callback_OnButtonModalModManager, AppGUI));
    button_min_width(pButtonOK, 128.f);

    /* Listboxes: */
    listbox_checkbox(pListboxMods, TRUE);
    listbox_size(pListboxMods, s2df((PANEL_DEFAULT_W / 2.f) - 16.f, 480));
    listbox_OnSelect(pListboxMods, listener(pApp, _Callback_OnModManagerListbox, AppGUI));
    for(i = 0; i < MAX_MOD_ELEMS;i++)
    {
        const GUIModElement *pElem = &pApp->pElementSets->pModElementArray[i];

        if (str_empty(pElem->sID))
        {
            continue;
        }

        listbox_add_elem(pListboxMods, tc(pElem->sName), NULL);
        listbox_check(pListboxMods, i, pElem->bActive);
    }

    if (listbox_count(pListboxMods) > 0)
    {
        listbox_select(pListboxMods, 0, TRUE);
    }

    /* Layout: Listboxes */
    layout_listbox(pLayoutListboxes, pListboxMods, 0, 0);

    /* Layout: Core */
    layout_hsize(pLayoutCore, 0, (PANEL_DEFAULT_W / 2.f) - 16.f);
    /* layout_hsize(pLayoutCore, 1, PANEL_DEFAULT_W / 2.f); */
    layout_valign(pLayoutCore, 0, 0, ekTOP);
    layout_valign(pLayoutCore, 1, 0, ekTOP);
    layout_layout(pLayoutCore, pLayoutListboxes, 0, 0);
    layout_panel(pLayoutCore, pPanelModPreview, 1, 0);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonOK, 1, 0);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    layout_halign(pLayoutButtons, 1, 0, ekRIGHT);

    /* Layout: Main */
    layout_margin(pLayoutMain, LAYOUT_DEFAULT_MARGIN);
    layout_layout(pLayoutMain, pLayoutCore,0, 0);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 1);

    /* Panel: Main */
    panel_size(pPanelMain,
        s2df(
            PANEL_DEFAULT_W + (LAYOUT_DEFAULT_MARGIN * 2.f), 
            PANEL_DEFAULT_H + (PANEL_DEFAULT_H / 8.f)));
    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

static void
_Callback_OnButtonModalToolsLuaCall(GUIToolElement *pElem, Event *e)
{
    AmberLauncher_ExecuteSVarLuaFunction(
        pElem->pAppCore,
        &pElem->tLuaRef
    );
    unref(e);
}

static Panel*
_Panel_GetModalToolsSubpanel(AppGUI *pApp)
{
    Panel       *pPanelMain  = panel_scroll(FALSE,TRUE);
    Layout      *pLayoutMain = layout_create(1,16);
    Font        *pFontBold   = font_system(18, ekFBOLD | ekFPIXELS);

    unsigned int i = 0;

    GUIToolElement *pElem;
    cassert_no_null(pApp->pElementSets->pToolElementArray);

    for(i = 0; i < MAX_TOOL_ELEMS; i ++)
    {
        Image       *pImageIco;
        Image       *pImageIcoDark;
        ImageView   *pImageViewIco;
        Button      *pButtonLaunch;
        Label       *pLabelTitle;
        Label       *pLabelDescription;
        Layout      *pLayoutImage;
        Layout      *pLayoutCell;
        Layout      *pLayoutLabels;
        ferror_t    eError;

        pElem = &pApp->pElementSets->pToolElementArray[i];
        assert(IS_VALID(pElem));

        if (pElem->dID <= 0)
        {
            /* Empty ID */
            break;
        }
        pElem->pAppCore     = pApp->pAppCore;

        pImageViewIco       = imageview_create();
        pButtonLaunch       = button_flat();
        pLabelTitle         = label_create();
        pLabelDescription   = label_create();
        pLayoutImage        = layout_create(1,1);
        pLayoutCell         = layout_create(3, 1);
        pLayoutLabels       = layout_create(1, 2);

        /* Image */
        pImageIco = image_from_file(tc(pElem->sIcon), &eError);
        assert(eError == ekFOK);
        pImageIcoDark = image_from_file(tc(pElem->sIconDark), &eError);

        /* Setup widgets */
        imageview_image(pImageViewIco, gui_dark_mode() ?
            (eError == ekFOK ? pImageIcoDark : pImageIco) : pImageIco);
        imageview_size(pImageViewIco, s2df(48.f, 48.f));

        button_image(pButtonLaunch, gui_dark_mode() ? 
        (const Image*)ICO_RUN_DARK_PNG : (const Image*)ICO_RUN_PNG);
        button_OnClick(
            pButtonLaunch,
            listener(
                pElem,
                _Callback_OnButtonModalToolsLuaCall,
                GUIToolElement
            )
        );

        label_text(pLabelTitle, tc(pElem->sTitle));
        label_font(pLabelTitle, pFontBold);

        label_text(pLabelDescription, tc(pElem->sDescription));
        label_multiline(pLabelDescription, TRUE);

        /* Layout: Image */
        layout_margin4(pLayoutImage, 0, 8, 0, 0);
        layout_valign(pLayoutImage, 0, 0, ekTOP);
        layout_imageview(pLayoutImage, pImageViewIco, 0, 0);

        /* Layout: Label */
        layout_valign(pLayoutLabels, 0, 0, ekTOP);
        layout_valign(pLayoutLabels, 0, 1, ekTOP);
        layout_hsize(pLayoutLabels, 0, TITLE_PNG_W);
        layout_label(pLayoutLabels, pLabelTitle, 0,0);
        layout_label(pLayoutLabels, pLabelDescription, 0,1);

        /* Layout: Cell */
        layout_valign(pLayoutCell, 0, 0, ekJUSTIFY);
        layout_margin(pLayoutCell, 8);
        layout_skcolor(pLayoutCell, gui_line_color());
        layout_layout(pLayoutCell, pLayoutImage, 0, 0);
        layout_layout(pLayoutCell, pLayoutLabels, 1, 0);
        layout_button(pLayoutCell, pButtonLaunch, 2, 0);

        /* Layout: Main */
        layout_layout(pLayoutMain, pLayoutCell, 0, i);

        if (IS_VALID(pImageIco))
        {
            image_destroy(&pImageIco);
        }
        if (IS_VALID(pImageIcoDark))
        {
            image_destroy(&pImageIcoDark);
        }
    }

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);
    font_destroy(&pFontBold);

    return pPanelMain;
}

Panel*
Panel_GetModalTools(AppGUI *pApp)
{
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelSubpanel     = _Panel_GetModalToolsSubpanel(pApp);
    Layout      *pLayoutMain        = layout_create(1,2);
    Layout      *pLayoutButtons     = layout_create(4,1);
    Button      *pButtonCancel      = button_push();

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalTools, AppGUI));
    button_min_width(pButtonCancel, 128.f);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    /* layout_halign(pLayoutButtons, 3, 0, ekRIGHT); */
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);

    /* Layout: Main */
    layout_panel(pLayoutMain, pPanelSubpanel, 0,0);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 1);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

static Panel*
_Panel_GetModalUpdaterSubpanel(AppGUI *pApp)
{
    Panel    *pPanelMain           = panel_create();
    Layout   *pLayoutMain          = layout_create(1,3);
    Layout   *pLayoutText          = layout_create(1,3);
    Layout   *pLayoutTextView      = layout_create(1,1);
    Layout   *pLayoutProgressbar   = layout_create(1,1);
    Label    *pLabelTitle          = label_create();
    Label    *pLabelInfo           = label_create();
    TextView *pTextView            = textview_create();
    Progress *pProgressbar         = progress_create();

    pApp->pWidgets->pProgressbar   = pProgressbar;

    /* Labels */
    label_text(pLabelTitle, TXT_LABEL_UPDATER_TITLE);
    label_text(pLabelInfo, TXT_UPDATERINFO);

    /* Text View: Log */
    pApp->pWidgets->pTextView = pTextView;
    textview_size(pTextView, s2df(480, 128));

    /* Progress: Download */
    progress_value(pProgressbar, 0.0f);

    /* Layout: Text */
    layout_margin(pLayoutText, LAYOUT_DEFAULT_MARGIN);
    layout_label(pLayoutText,pLabelTitle, 0, 0);
    layout_label(pLayoutText,pLabelInfo, 0, 2);

    /* Layout: Text View */
    layout_margin(pLayoutTextView, LAYOUT_DEFAULT_MARGIN);
    layout_textview(pLayoutTextView, pTextView, 0, 0);

    /* Layout: Progressbar */
    layout_margin(pLayoutProgressbar, LAYOUT_DEFAULT_MARGIN);
    layout_progress(pLayoutProgressbar, pProgressbar, 0, 0);

    /* Layout: Main */
    layout_layout(pLayoutMain, pLayoutText, 0, 0);
    layout_layout(pLayoutMain, pLayoutTextView, 0, 1);
    layout_layout(pLayoutMain, pLayoutProgressbar, 0, 2);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

Panel*
Panel_GetModalUpdater(AppGUI* pApp)
{
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelExt          = _Panel_GetModalUpdaterSubpanel(pApp);
    Layout      *pLayoutMain        = layout_create(1,3);
    Layout      *pLayoutExt         = layout_create(1,1);
    Layout      *pLayoutHeader      = layout_create(1,1);
    Layout      *pLayoutButtons     = layout_create(4,1);
    Button      *pButtonCancel      = button_push();
    Button      *pButtonUpdate      = button_push();
    ImageView   *pImageViewHeader   = imageview_create();

    /* Image View: Header Image */
    imageview_scale(pImageViewHeader, ekGUI_SCALE_ADJUST);
    imageview_size(pImageViewHeader, s2df(480,240));
    imageview_image(pImageViewHeader, (const Image*)UPDATE_HEADER_JPG);

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalUpdater, AppGUI));
    button_min_width(pButtonCancel, 128.f);

    /* Button: Update App */
    button_text(pButtonUpdate, TXT_BTN_UPDATE_APP);
    button_tag(pButtonUpdate, MODAL_UPDATE_APP);
    button_OnClick(pButtonUpdate, listener(pApp, Callback_OnButtonModalUpdater, AppGUI));
    button_min_width(pButtonUpdate, 128.f);

    /* Layout: External */
    layout_panel(pLayoutExt, pPanelExt, 0, 0);

    /* Layout: Header */
    layout_margin4(pLayoutHeader, LAYOUT_DEFAULT_MARGIN, 0.f, 0.f, 0.f);
    layout_imageview(pLayoutHeader, pImageViewHeader, 0, 0);

    /* Layout: Buttons */
    layout_margin4(pLayoutButtons, _fDefMarg * 2.f, _fDefMarg, _fDefMarg, _fDefMarg);
    layout_halign(pLayoutButtons, 0, 0, ekLEFT);
    layout_halign(pLayoutButtons, 3, 0, ekRIGHT);
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonUpdate, 3, 0);

    /* Layout: Main */
    layout_layout(pLayoutMain, pLayoutHeader, 0, 0);
    layout_layout(pLayoutMain, pLayoutExt, 0, 1);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 2);

    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

void
AutoUpdate_Init(void)
{
    dbind(InetUpdaterFile, String*, path);
    dbind(InetUpdaterFile, String*, sha256);
    dbind(InetUpdaterFile, uint32_t, size);

    dbind(InetUpdaterLauncherData, int32_t, version);
    dbind(InetUpdaterLauncherData, int32_t, build);

    dbind(InetUpdaterJSONData, String*, schema);
    dbind(InetUpdaterJSONData, String*, generated);
    dbind(InetUpdaterJSONData, InetUpdaterLauncherData*, launcher);
    dbind(InetUpdaterJSONData, ArrSt(InetUpdaterFile)*, files);
}

bool_t
AutoUpdate_CheckForUpdates(AppGUI *pApp)
{
    uint32_t            dResult;
    ierror_t            eInetError;
    bool_t              bUpdateRequired;
    Stream              *pJsonLauncherStream;
    Stream              *pJsonModStream;
    InetUpdaterJSONData *pJsonLauncher;
    InetUpdaterJSONData *pJsonMod;

    const SVar tLuaLauncherVersion      = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_LAUNCHER_VERSION);
    const SVar tLuaModVersion           = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_MOD_VERSION);
    const SVar tLuaLauncherManifestURL  = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_URL_UPDATER_LAUNCHER_MANIFEST);
    const SVar tLuaModManifestURL       = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_URL_UPDATER_MOD_MANIFEST);
    const int32_t dLauncherBuild        = str_to_i32(BUILD_NUMBER, 10, NULL);
    const int32_t dLauncherVersion      = SVAR_IS_DOUBLE(tLuaLauncherVersion) ? (int32_t)SVAR_GET_DOUBLE(tLuaLauncherVersion) : 0;
    const int32_t dModVersion           = SVAR_IS_DOUBLE(tLuaModVersion)      ? (int32_t)SVAR_GET_DOUBLE(tLuaModVersion) : 0;

    static const char *sSchemaFmt       = "• Schema: \t\t%s\n";
    static const char *sGeneratedFmt    = "• Generated: \t\t%s\n";
    static const char *sAppVersionFmt   = "• App version: \t\t%d\n";
    static const char *sNetVersionFmt   = "• Net version: \t\t%d\n";
    static const char *sAppBuildFmt     = "• App build: \t\t%d\n";
    static const char *sNetBuildFmt     = "• Net build: \t\t%d\n";
    static const char *sModVersionFmt   = "• Mod version: \t\t%d\n";
    static const char *sModNetVersionFmt= "• Mod Net version: \t%d\n";

    /* Launcher manifest */
    _al_printf( pApp,
        "[Updater] Fetching Launcher manifest at \n\t%s\n",
            SVAR_IS_CONSTCHAR(tLuaLauncherManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaLauncherManifestURL) :
            _sDefaultUpdaterLauncherManifestURL
        );

    pJsonLauncherStream = http_dget(
        SVAR_IS_CONSTCHAR(tLuaLauncherManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaLauncherManifestURL) :
            _sDefaultUpdaterLauncherManifestURL,
        &dResult,
        &eInetError);

    if (!pJsonLauncherStream)
    {
        _al_printf(pApp,
            "[Updater] Couldn't fetch al_manifest.json"
            "\n\tdResult: %d\n\teInetError: %d\n",
            dResult,
            eInetError);
        return FALSE;
    }
    pJsonLauncher = json_read(pJsonLauncherStream, NULL, InetUpdaterJSONData);

    /* Mod manifest */
    _al_printf( pApp,
        "[Updater] Fetching Mod manifest at \n\t%s\n",
            SVAR_IS_CONSTCHAR(tLuaModManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaModManifestURL) :
            _sDefaultUpdaterModManifestURL
        );

    pJsonModStream = http_dget(
        SVAR_IS_CONSTCHAR(tLuaModManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaModManifestURL) :
            _sDefaultUpdaterModManifestURL,
        &dResult,
        &eInetError);

    if (!pJsonModStream)
    {
        _al_printf( pApp,
                    "[Updater] Couldn't fetch mod_manifest.json"
                    "\n\tdResult: %d\n\teInetError: %d\n",
                    dResult,
                    eInetError);
        return FALSE;
    }
    pJsonMod        = json_read(pJsonModStream, NULL, InetUpdaterJSONData);

    /* Check for update and report */
    bUpdateRequired = (dLauncherBuild < pJsonLauncher->launcher->build ||
                        dLauncherVersion < pJsonLauncher->launcher->version ||
                        dModVersion < pJsonLauncher->launcher->version);

    _al_printf(pApp,
        "[Updater] Manifests retrieved successfuly:\n"
        "• Update required: \t%s\n",
        bUpdateRequired ? "true" : "false"
    );
    _al_printf(pApp, sSchemaFmt, tc(pJsonLauncher->schema));
    _al_printf(pApp, sGeneratedFmt, tc(pJsonLauncher->generated));
    _al_printf(pApp, sAppVersionFmt, dLauncherVersion);
    _al_printf(pApp, sNetVersionFmt, pJsonLauncher->launcher->version);
    _al_printf(pApp, sAppBuildFmt, dLauncherBuild);
    _al_printf(pApp, sNetBuildFmt, pJsonLauncher->launcher->build);
    _al_printf(pApp, sModVersionFmt, dModVersion);
    _al_printf(pApp, sModNetVersionFmt, pJsonMod->launcher->version);

    stm_close(&pJsonLauncherStream);
    stm_close(&pJsonModStream);

    json_destroy(&pJsonLauncher, InetUpdaterJSONData);
    json_destroy(&pJsonMod, InetUpdaterJSONData);

    return bUpdateRequired;
}

static void
_AutoUpdate_Update_DownloadFiles(
    AppGUI *pApp,
    InetUpdaterJSONData *pJson,
    const char *sRootURL,
    real32_t *fProgressIndex,
    real32_t fProgressMax,
    bool_t bForceDownload)
{
    arrst_foreach(elem, pJson->files, InetUpdaterFile)
        bool_t bFileExists    = hfile_exists(tc(elem->path),0);
        char *sSHA256Hash     = AmberLauncher_SHA256_HashFile(tc(elem->path));
        String *sDownloadLink = str_printf("%s%s", sRootURL, tc(elem->path));

        Stream *pDownloadFile;

        _al_printf(pApp, "[Updater] Downloading %s...\n", tc(elem->path));

        if (!bFileExists)
        {
            _al_printf(pApp, "[Updater] Local file %s doesn't exist!\n",
                sDownloadLink);

            pDownloadFile = http_dget(tc(sDownloadLink), NULL, NULL);
            if (pDownloadFile)
            {
                /* download file */
                Stream *pOutputFile = stm_to_file(tc(elem->path), NULL);
                stm_write(pOutputFile, 
                    stm_buffer(pDownloadFile),
                    stm_buffer_size (pDownloadFile));
                stm_close(&pOutputFile);
            }
        }
        else if (sSHA256Hash || bForceDownload)
        {
            _al_printf(pApp, "[Updater] sha256 %s:\n\t%s\n", 
                sDownloadLink, sSHA256Hash);

            if (str_cmp(elem->sha256, sSHA256Hash) != 0 || bForceDownload)
            {
                _al_printf(pApp, "[Updater] Remote file is different!\n");

                pDownloadFile = http_dget(tc(sDownloadLink), 
                    NULL,
                    NULL);
                if (pDownloadFile)
                {
                    Stream *pOutputFile;

                    /* stm_to_file doesn't overwrite as it promises to do, so, delete file.. */
                    bfile_delete(tc(elem->path), NULL);

                    pOutputFile = stm_to_file(tc(elem->path), NULL);
                    stm_write(pOutputFile, 
                        stm_buffer(pDownloadFile), 
                        stm_buffer_size (pDownloadFile));
                    stm_close(&pOutputFile);
                }
            }
        }
        else
        {
            _al_printf(pApp, "[Updater] unknown file error:\n\t%s(%s)\n", 
                sDownloadLink, sSHA256Hash);
        }
        str_destroy(&sDownloadLink);
        stm_close(&pDownloadFile);

        /* Track progress */
        *fProgressIndex += 1.0f;
        if (pApp->pWidgets->pProgressbar)
        {
            progress_value(pApp->pWidgets->pProgressbar, 
                *fProgressIndex / fProgressMax);
        }

        free(sSHA256Hash);
    arrst_end()
}

bool_t
AutoUpdate_Update(AppGUI *pApp, bool_t bForceDownload)
{
    uint32_t            dResult;
    ierror_t            eInetError;
    Stream              *pJsonLauncherStream;
    Stream              *pJsonModStream;
    InetUpdaterJSONData *pJsonLauncher;
    InetUpdaterJSONData *pJsonMod;
    real32_t            fProgressIndex;
    real32_t            fProgressMax;

    static const char *sFileArrayFmt = "• File: %s\n• • sha256: \n%s\n• • size: %u\n";

    const SVar tLuaLauncherManifestURL  = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_URL_UPDATER_LAUNCHER_MANIFEST);
    const SVar tLuaModManifestURL       = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_URL_UPDATER_MOD_MANIFEST);
    const SVar tLuaRootURL              = AmberLauncher_GetGlobalVariable(pApp->pAppCore, _sLua_URL_UPDATER_ROOT);

    /* Fire start event */
    AmberLauncher_Update(pApp->pAppCore, CFALSE);

    /* Launcher Manifest */
    _al_printf( pApp,
        "[Updater] Fetching Launcher manifest at \n\t%s\n",
            SVAR_IS_CONSTCHAR(tLuaLauncherManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaLauncherManifestURL) :
            _sDefaultUpdaterLauncherManifestURL
        );

    pJsonLauncherStream = http_dget(
        SVAR_IS_CONSTCHAR(tLuaLauncherManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaLauncherManifestURL) :
            _sDefaultUpdaterLauncherManifestURL,
        &dResult,
        &eInetError);

    if (!pJsonLauncherStream)
    {
        _al_printf( pApp,
                    "[Updater] Couldn't fetch al_manifest.json"
                    "\n\tdResult: %d\n\teInetError: %d\n",
                    dResult,
                    eInetError);
        return FALSE;
    }

    _al_printf( pApp,
                "[Updater] Launcher Manifest retrieved successfuly:\n"
                "• dResult: \t\t%d\n• eInetError: \t\t%d\n",
                dResult,
                eInetError);

    pJsonLauncher = json_read(pJsonLauncherStream, NULL, InetUpdaterJSONData);
    stm_close(&pJsonLauncherStream);

    /* Mod Manifest */
    _al_printf( pApp,
        "[Updater] Fetching Mod manifest at \n\t%s\n",
            SVAR_IS_CONSTCHAR(tLuaModManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaModManifestURL) :
            _sDefaultUpdaterModManifestURL
        );

    pJsonModStream = http_dget(
        SVAR_IS_CONSTCHAR(tLuaModManifestURL) ?
            SVAR_GET_CONSTCHAR(tLuaModManifestURL) :
            _sDefaultUpdaterModManifestURL,
        &dResult,
        &eInetError);

    if (!pJsonModStream)
    {
        _al_printf( pApp,
                    "[Updater] Couldn't fetch mod_manifest.json"
                    "\n\tdResult: %d\n\teInetError: %d\n",
                    dResult,
                    eInetError);
        return FALSE;
    }

    _al_printf( pApp,
                "[Updater] Mod Manifest retrieved successfuly:\n"
                "• dResult: \t\t%d\n• eInetError: \t\t%d\n",
                dResult,
                eInetError);

    pJsonMod = json_read(pJsonModStream, NULL, InetUpdaterJSONData);
    stm_close(&pJsonModStream);

    /* Print files */
    _al_printf(pApp, "Launcher files: \n");
    arrst_foreach(elem, pJsonLauncher->files, InetUpdaterFile)
        _al_printf(pApp, sFileArrayFmt, tc(elem->path), tc(elem->sha256), elem->size);
    arrst_end()
    _al_printf(pApp, "\n");
    _al_printf(pApp, "Mod files: \n");
    arrst_foreach(elem, pJsonMod->files, InetUpdaterFile)
        _al_printf(pApp, sFileArrayFmt, tc(elem->path), tc(elem->sha256), elem->size);
    arrst_end()
    _al_printf(pApp, "\n");

    /* Download files */
    fProgressIndex  = 0.0f;
    fProgressMax    = (real32_t)arrst_size(pJsonLauncher->files, InetUpdaterFile) + 
                        (real32_t)arrst_size(pJsonMod->files, InetUpdaterFile);

    if (pApp->pWidgets->pProgressbar)
    {
        progress_value(pApp->pWidgets->pProgressbar, 
            0.0f);
    }
    
    _al_printf(pApp, "[Updater] Remote Updater Server: %s\n",
        SVAR_IS_CONSTCHAR(tLuaRootURL) ?
            SVAR_GET_CONSTCHAR(tLuaRootURL) :
            _sDefaultUpdaterRemoteRootURL);

    _AutoUpdate_Update_DownloadFiles(
        pApp, 
        pJsonLauncher,
        SVAR_IS_CONSTCHAR(tLuaRootURL) ?
            SVAR_GET_CONSTCHAR(tLuaRootURL) :
            _sDefaultUpdaterRemoteRootURL,
        &fProgressIndex, 
        fProgressMax,
        bForceDownload);
    _AutoUpdate_Update_DownloadFiles(
        pApp, 
        pJsonMod,
        SVAR_IS_CONSTCHAR(tLuaRootURL) ?
            SVAR_GET_CONSTCHAR(tLuaRootURL) :
            _sDefaultUpdaterRemoteRootURL,
        &fProgressIndex, 
        fProgressMax,
        bForceDownload);

    json_destroy(&pJsonLauncher, InetUpdaterJSONData);
    json_destroy(&pJsonMod, InetUpdaterJSONData);

    _al_printf( pApp,"[Updater] Update done!\n");

    /* Fire end event */
    AmberLauncher_Update(pApp->pAppCore, CTRUE);

    return TRUE;
}

void
Callback_OnWindowHotkeyF6(AppGUI* pApp, Event *e)
{
    _Callback_UIEvent(pApp->pAppCore, UIEVENT_DEBUG, NULL, 0);
    unref(e);
}

void
Callback_OnButtonConfigure(AppGUI *pApp, Event *e)
{
    Button *pButton     = event_sender(e, Button);
    FPanelFlags dFlags  = button_get_tag(pButton);

    if (FLAG_HAS(dFlags, FLAG_PANEL_STATE_END))
    {
        GUIThread_SchedulePanelSet(pApp, CPANEL_MAIN, FLAG_PANEL_NONE);
        return;
    }

    AmberLauncher_ConfigureStart(pApp->pAppCore);
}

void
Callback_OnButtonModalDebug(AppGUI *pApp, Event *e)
{
    const char *sEditText;

    cassert_no_null(pApp->pDebugData->pConsoleTextView);

    sEditText = edit_get_text(pApp->pDebugData->pConsoleInput);

    _al_printf(pApp, "> %s\n", sEditText);

    AmberLauncher_ExecuteLua(pApp->pAppCore, sEditText);

    edit_text(pApp->pDebugData->pConsoleInput,"");
    unref(e);
}

void
Callback_OnButtonModalMessage(AppGUI *pApp, Event *e)
{
    Button *pButton = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);
    window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
    unref(pButton);
    unref(e);
}

void
Callback_OnButtonModalQuestion(AppGUI *pApp, Event *e)
{
    Button *pButton = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);
    window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
    unref(pButton);
    unref(e);
}

void
Callback_OnButtonModalGameNotFound(AppGUI *pApp, Event *e)
{
    const char *sFileFormat[] = {
        "exe",
    };
    Button *pButton     = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);

    /* Button clicked: Submit or Cancel */
    if (dButtonTag != MODAL_BROWSE)
    {
        window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
        return;
    }

    /* Button clicked: Browse */
    {
        const char *sBrowserPath = comwin_open_file(pApp->pWindows->pWindowModal, NULL, sFileFormat, 1, NULL);
        str_upd(&pApp->pWidgets->pString, sBrowserPath ? sBrowserPath : "");
    }

    if (pApp->pWidgets->pEdit)
    {
        edit_text(pApp->pWidgets->pEdit, tc(pApp->pWidgets->pString));
    }

    _al_printf(pApp, "Game path: %s\n", tc(pApp->pWidgets->pString));
}

void
Callback_OnButtonModalTweaker(AppGUI *pApp, Event *e)
{
    Button *pButton         = event_sender(e, Button);
    uint32_t dButtonTag     = button_get_tag(pButton);
    uint32_t dImageIndex    = (dButtonTag - MODAL_OPT_NULL) - 1;
    bool_t bPageSwitch      = FALSE;
    GUITweaker *pTweaker    = &pApp->tGUITweaker[pApp->dPage];

    /* assign option */
    if (dButtonTag < MODAL_OPT_MAX)
    {
        pTweaker->dSelectedOption = dImageIndex;
    }

    switch(dButtonTag)
    {
        case MODAL_CANCEL:
        case MODAL_ACCEPT:
            window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
            return;
        case MODAL_PREVIOUS:
        case MODAL_NEXT:
            {
                const int dStep = (dButtonTag == MODAL_NEXT) ? 1 : -1;
                pApp->dPage     = (unsigned int)bmath_clampd((int)pApp->dPage + dStep, 0, (int)pApp->dPageMax - 1);
                bPageSwitch     = TRUE;
                break;
            }
        default:
            break;
    }

    if (bPageSwitch == TRUE)
    {
        const int unsigned dRowForPage = _RowForPage(pApp->dPage, pApp->dPageMax);
        unsigned int dRow;

        pTweaker    = &pApp->tGUITweaker[pApp->dPage];
        dImageIndex = pApp->tGUITweaker[pApp->dPage].dSelectedOption;
        layout_panel_replace(pApp->pWindows->pLayoutModalMain, _Panel_TweakerDescription(pApp), 0, 0);
        layout_panel_replace(pApp->pWindows->pLayoutModalMain, _Panel_TweakerOptionsSelector(pApp), 0, 1);
        /* layout_panel_replace(pApp->pLayoutModalMain, _Panel_TweakerButtons(pApp), 0, 4); */

        /* Show appropriate button layout */
        for (dRow = 0; dRow < 4; ++dRow)
        {
            layout_show_row(pApp->pWindows->pLayoutExtra, dRow, dRow == dRowForPage);
        }
    }

    str_destopt(&pApp->pWidgets->pString);
    pApp->pWidgets->pString = NULL;
    if (pTweaker->sImagePath[dImageIndex])
    {
        pApp->pWidgets->pString = str_copy(pTweaker->sImagePath[dImageIndex]);
    }
    layout_panel_replace(pApp->pWindows->pLayoutModalMain, Panel_GetImageDemo(pApp), 0, 2);

    layout_update(pApp->pWindows->pLayoutModalMain);
}

void
Callback_OnButtonModalLocalisation(AppGUI *pApp, Event *e)
{
    Button *pButton     = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);

    pApp->pLocalizations->dLocaleSelected[LOC_CORE] = popup_get_selected(pApp->pWidgets->pPopupCore);
    pApp->pLocalizations->dLocaleSelected[LOC_MOD]  = popup_get_selected(pApp->pWidgets->pPopupMod);

    window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);

    unref(pButton);
    unref(e);
}

void
Callback_OnButtonModalOptions(AppGUI *pApp, Event *e)
{
    Button *pButton             = event_sender(e, Button);
    const uint32_t dButtonTag   = button_get_tag(pButton);

    if (dButtonTag > MODAL_OPT_MAX)
    {
        /* Gather all data from ui widgets */
        if (dButtonTag == MODAL_ACCEPT)
        {
            unsigned int i;
            for(i = 0; i < MAX_OPT_ELEMS; i++)
            {
                GUIOptElement *pElem = &pApp->pElementSets->pOptElementArray[i];

                if (pElem->eType == UI_WIDGET_NULL)
                {
                    continue;
                }

                switch(pElem->eType)
                {
                    case UI_WIDGET_RADIO:
                        {
                            unsigned int j;
                            Layout *pLayout = (Layout*)pElem->pElement;

                            /* Layout because it contains multiple buttons */
                            for(j = 0; j < pElem->dNumOfOptions; j++)
                            {
                                unsigned int pos           = j * 2;
                                const Button *pRadioButton = layout_get_button(pLayout, pos + 1, 0);
                                const gui_state_t eState   = button_get_state(pRadioButton);

                                if (eState == ekGUI_ON)
                                {
                                    pElem->dChoice = j;
                                    break;
                                }
                            }
                        }
                        break;
                    case UI_WIDGET_CHECKBOX:
                        {
                            const Button *pButtonUI = (const Button*)pElem->pElement;
                            unsigned int dIndex     = button_get_state(pButtonUI);
                            pElem->dChoice          = dIndex == ekGUI_OFF ? 0 : 1;
                        }
                        break;
                    case UI_WIDGET_POPUP:
                        {
                            const PopUp *pPopup = (const PopUp*)pElem->pElement;
                            unsigned int dIndex = popup_get_selected(pPopup);
                            pElem->dChoice      = dIndex;
                        }
                        break;
                    case UI_WIDGET_EDIT:
                        {
                            const Edit *pEdit    = (const Edit*)pElem->pElement;
                            const char_t *pValue = edit_get_text(pEdit);
                            str_upd(&pElem->sOutputString, pValue);
                        }
                        break;
                    case UI_WIDGET_LISTBOX:
                        {
                            const ListBox *pListbox = (const ListBox*)pElem->pElement;
                            unsigned int j;

                            for(j = 0; j < listbox_count(pListbox); j++)
                            {
                                const bool_t bIsChecked = listbox_checked(pListbox, j);
                                if (bIsChecked)
                                {
                                    pElem->dChoice |=  (1U << j);
                                }
                                else
                                {
                                    pElem->dChoice &= ~(1U << j);
                                }
                                /* str_upd(&pElem->sOutputString, listbox_get_text(pListbox, j)); */
                            }
                        }
                        break;
                    case UI_WIDGET_NULL:
                    case UI_WIDGET_MAX:
                    default:
                        break;
                }
            }
        }

        window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
        return;
    }
}

void
Callback_OnButtonModalUpdater(AppGUI* pApp, Event *e)
{
    Button *pButton     = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);

    switch(dButtonTag)
    {
        case MODAL_UPDATE_APP:
            {
                const bool_t bUpdateAvailable = AutoUpdate_CheckForUpdates(pApp);
                if (!bUpdateAvailable)
                {
                    break;
                }

                if (AutoUpdate_Update(pApp, FALSE))
                {
                    pApp->pWidgets->pTextView = NULL;
                    window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
                }
            }
            break;

        default:
            pApp->pWidgets->pTextView = NULL;
            window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
            break;
    }

    unref(pButton);
    unref(e);
}

void
Callback_OnButtonModalTools(AppGUI *pApp, Event *e)
{
    Button *pButton             = event_sender(e, Button);
    const uint32_t dButtonTag   = button_get_tag(pButton);

    if (dButtonTag > MODAL_OPT_MAX)
    {
        window_stop_modal(pApp->pWindows->pWindowModal, dButtonTag);
        return;
    }
}

void 
Callback_OnButtonPlay(AppGUI *pApp, Event *e)
{
    AmberLauncher_Play(pApp->pAppCore);
    unref(e);
}

void
Callback_OnDrawLocalisation(AppGUI *pApp, Event *e)
{
    ferror_t                    eError1;
    ferror_t                    eError2;
    Image                      *pImageBG   = NULL;
    Image                      *pImageFG   = NULL;
    const EvDraw               *pEvDraw    = event_params(e, EvDraw);
    DCtx                       *pCtx       = pEvDraw ? pEvDraw->ctx : NULL;
    const AppGUILocalization   *pLocs      = pApp->pLocalizations;

    assert(pLocs->tLocale[pLocs->dLocaleSelected[LOC_CORE]].bCore);
    assert(pLocs->tLocale[pLocs->dLocaleSelected[LOC_MOD]].bMod);

    pImageBG = image_from_file(
        pLocs->tLocale[pLocs->dLocaleSelected[LOC_CORE]].sCorePath, &eError1);
    pImageFG = image_from_file(
        pLocs->tLocale[pLocs->dLocaleSelected[LOC_MOD]].sModPath, &eError2);

    assert(eError1 == ekFOK);
    assert(eError2 == ekFOK);

    draw_clear(pCtx, color_rgb(0, 0, 0));

    if (IS_VALID(pImageBG))
    {
        draw_image(pCtx, pImageBG, 0, 0);
        draw_image(pCtx, pImageFG, 0, 0);
        image_destroy(&pImageBG);
    }
    image_destroy(&pImageFG);
    unref(pApp);
}


static uint32_t
_GUIThread_Main_Null(GUIAsyncTaskData *pThreadData)
{
    unref(pThreadData);
    bthread_sleep(64);
    return 1;
}

/* GUI thread (safe to touch UI) */
static void
_GUIThread_End_PanelSetMain(GUIAsyncTaskData *pThreadData, const uint32_t dRValue)
{
    Panel_Set(
        pThreadData->pApp,
        pThreadData->ePanelType,
        pThreadData->dPanelFlags
    );
    heap_delete(&pThreadData, GUIAsyncTaskData);

    unref(dRValue);
}

bool_t
GUIThread_SchedulePanelSet(AppGUI *pApp, EPanelType eType, FPanelFlags dFlags)
{
    GUIAsyncTaskData *pData;

    pData = heap_new(GUIAsyncTaskData);
    if (!IS_VALID(pData))
    {
        return FALSE;
    }

    pData->pApp        = pApp;
    pData->ePanelType  = eType;
    pData->dPanelFlags = dFlags;

    osapp_task(
        pData,
        0.0,
        _GUIThread_Main_Null,
        NULL,
        _GUIThread_End_PanelSetMain,
        GUIAsyncTaskData
    );

    return TRUE;
}

/******************************************************************************
 * STATIC DEFINITIONS
 ******************************************************************************/
static void
_al_printf(AppGUI *pApp, const char* fmt, ...)
{
    char buffer[AL_PRINTF_BUFFER_SIZE];
    va_list args;

    va_start(args, fmt);
    bstd_vsprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    bstd_printf("%s", buffer);

    cassert_no_null(pApp);
    cassert_no_null(pApp->pWidgets);

    if (pApp->pWidgets->pTextView)
    {
        textview_printf(pApp->pWidgets->pTextView, "%s\n", buffer);
        textview_scroll_caret(pApp->pWidgets->pTextView);
    }
}

static void
_window_center(Window *pWindow)
{
    S2Df tScreenSize;
    S2Df tWindowSize;
    V2Df tScreenPos;
    V2Df tWindowPos;
    V2Df tCenterPos;

    tScreenSize = gui_resolution();
    tWindowSize = window_get_size(pWindow);

    tScreenPos = v2df(tScreenSize.width, tScreenSize.height);
    tWindowPos = v2df(tWindowSize.width, tWindowSize.height);

    tScreenPos = v2d_mulf(&tScreenPos, 0.5f);
    tWindowPos = v2d_mulf(&tWindowPos, 0.5f);
    tCenterPos = v2d_subf(&tScreenPos, &tWindowPos);

    window_origin(pWindow, tCenterPos);
}

static Panel*
_Panel_GetRoot(AppGUI *pApp)
{
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelExt          = panel_create();
    Layout      *pLayoutCore        = layout_create(2,1);
    Layout      *pLayoutMain        = layout_create(1,2);
    Layout      *pLayoutButtons     = layout_create(1,3);
    Layout      *pLayoutButtonsTop  = layout_create(1,4);
    Layout      *pLayoutButtonsBot  = layout_create(1,4);
    Layout      *pLayoutImage       = layout_create(1,1);
    Layout      *pLayoutExt         = layout_create(1,1);
    ImageView   *pImageView         = imageview_create();
    Button      *pButtonSettings    = button_flat();
    Button      *pButtonMods        = button_flat();
    Button      *pButtonTools       = button_flat();
    Button      *pButtonWebHomepage = button_flat();
    Button      *pButtonWebDiscord  = button_flat();
    Button      *pButtonUpdate      = button_flat();

    bool_t bUpdateAvailable = AutoUpdate_CheckForUpdates(pApp);

    static const real32_t dButtonIconWidth   = ICO_PNG_W + (ICO_PNG_W / 2);
    static const real32_t dButtonIconHeight  = ICO_PNG_H + (ICO_PNG_H / 2);

    pApp->pWindows->pLayoutMain     = pLayoutMain;
    pApp->pWindows->pLayoutWindow   = pLayoutCore;

    /* Buttons */
    button_image(pButtonSettings, gui_dark_mode() ?
    (const Image*)ICO_GEAR_DARK_PNG : (const Image*)ICO_GEAR_PNG);
    button_tag(pButtonSettings, CSIDEBUTTON_SETTINGS);
    button_tooltip(pButtonSettings, TXT_TOOLTIP_SETTINGS);
    button_OnClick(pButtonSettings, listener(pApp, _Callback_OnButtonMainWindow, AppGUI) );

    button_image(pButtonMods, gui_dark_mode() ?
        (const Image*)ICO_PUZZLE_DARK_PNG : (const Image*)ICO_PUZZLE_PNG);
    button_tag(pButtonMods, CSIDEBUTTON_MODS);
    button_tooltip(pButtonMods, TXT_TOOLTIP_MODS);
    button_OnClick(pButtonMods, listener(pApp, _Callback_OnButtonMainWindow, AppGUI) );

    button_image(pButtonTools, gui_dark_mode() ?
    (const Image*)ICO_WRENCH_DARK_PNG : (const Image*)ICO_WRENCH_PNG);
    button_tag(pButtonTools, CSIDEBUTTON_TOOLS);
    button_tooltip(pButtonTools, TXT_TOOLTIP_TOOLS);
    button_OnClick(pButtonTools, listener(pApp, _Callback_OnButtonMainWindow, AppGUI) );

    button_image(pButtonWebHomepage, gui_dark_mode() ?
        (const Image*)ICO_WEB_DARK_PNG : (const Image*)ICO_WEB_PNG);
    button_tag(pButtonWebHomepage, CSIDEBUTTON_WEB_HOMEPAGE);
    button_tooltip(pButtonWebHomepage, TXT_TOOLTIP_HOMEPAGE);
    button_OnClick(pButtonWebHomepage, listener(pApp, _Callback_OnButtonMainWindow, AppGUI) );

    button_image(pButtonWebDiscord, gui_dark_mode() ?
        (const Image*)ICO_DISCORD_DARK_PNG : (const Image*)ICO_DISCORD_PNG);
    button_tag(pButtonWebDiscord, CSIDEBUTTON_WEB_DISCORD);
    button_tooltip(pButtonWebDiscord, TXT_TOOLTIP_DISCORD);
    button_OnClick(pButtonWebDiscord, listener(pApp, _Callback_OnButtonMainWindow, AppGUI) );

    if (gui_dark_mode())
    {
        button_image(pButtonUpdate, bUpdateAvailable ?
            (const Image*)ICO_UPDATE_AVAILABLE_DARK_PNG :
            (const Image*)ICO_UPDATE_DARK_PNG);
    }
    else
    {
        button_image(pButtonUpdate, bUpdateAvailable ?
            (const Image*)ICO_UPDATE_AVAILABLE_PNG :
            (const Image*)ICO_UPDATE_PNG);
    }

    button_tag(pButtonUpdate, CSIDEBUTTON_UPDATE);
    button_tooltip(pButtonUpdate, TXT_TOOLTIP_UPDATE);
    button_OnClick(pButtonUpdate, listener(pApp, _Callback_OnButtonMainWindow, AppGUI) );

    /* Image: Header */
    imageview_scale(pImageView, ekGUI_SCALE_AUTO);
    imageview_size(pImageView, s2df(TITLE_PNG_W,TITLE_PNG_H));
    imageview_image(pImageView, (const Image*)TITLE_JPG);

    /* Layout: Image */
    layout_hsize(pLayoutImage, 0, TITLE_PNG_W);
    layout_vsize(pLayoutImage, 0, TITLE_PNG_H);
    layout_imageview(pLayoutImage, pImageView, 0, 0);
    
    /* Layout: External */
    layout_hsize(pLayoutExt, 0, TITLE_PNG_W);
    layout_halign(pLayoutExt, 0, 0, ekLEFT);
    layout_valign(pLayoutExt, 0, 0, ekTOP);
    panel_layout(pPanelExt, pLayoutExt);

    /* Layout: Buttons Top */
    layout_valign(pLayoutButtonsTop, 0, 0, ekTOP);
    layout_hsize(pLayoutButtonsTop, 0, dButtonIconWidth);

    layout_button(pLayoutButtonsTop, pButtonSettings, 0, 0);
    layout_button(pLayoutButtonsTop, pButtonMods, 0, 1);
    layout_button(pLayoutButtonsTop, pButtonTools, 0, 2);
    unref(dButtonIconHeight);

    /* Layout: Buttons Bottom */
    layout_valign(pLayoutButtonsBot, 0, 0, ekBOTTOM);
    layout_hsize(pLayoutButtonsBot, 0, dButtonIconWidth);
    layout_button(pLayoutButtonsBot, pButtonWebDiscord, 0, 0);
    layout_button(pLayoutButtonsBot, pButtonWebHomepage, 0, 1);
    layout_button(pLayoutButtonsBot, pButtonUpdate, 0, 2);

    /* Layout: Buttons (Main) */
    layout_margin4(pLayoutButtons, 0, 4, 0, 0);
    layout_valign(pLayoutButtons, 0, 0, ekTOP);
    layout_valign(pLayoutButtons, 0, 1, ekJUSTIFY);
    layout_valign(pLayoutButtons, 0, 2, ekBOTTOM);
    layout_layout(pLayoutButtons, pLayoutButtonsTop,0, 0);
    layout_cell(pLayoutButtons, 0, 1);
    layout_vexpand(pLayoutButtons, 1);
    layout_layout(pLayoutButtons, pLayoutButtonsBot,0, 2);

    /* Layout: Main */
    layout_margin(pLayoutMain, 4);
    layout_hsize(pLayoutMain, 0, TITLE_PNG_W);
    layout_vsize(pLayoutMain, 0, TITLE_PNG_H);
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_halign(pLayoutMain, 0, 0, ekJUSTIFY);

    layout_layout(pLayoutMain, pLayoutImage, 0, 0);
    layout_panel(pLayoutMain, pPanelExt, 0, 1);

    /* Layout: Core */
    layout_valign(pLayoutCore, 0, 0, ekJUSTIFY);
    layout_valign(pLayoutCore, 1, 0, ekTOP);
    layout_hsize(pLayoutCore, 0, dButtonIconWidth);
    layout_hsize(pLayoutCore, 1, TITLE_PNG_W);
    layout_layout(pLayoutCore, pLayoutButtons, 0, 0);
    layout_layout(pLayoutCore, pLayoutMain, 1, 0);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutCore);

    /* Starting panel */
    Panel_Set(pApp, CPANEL_MAIN, FLAG_PANEL_NONE);

    return pPanelMain;
}

static AppGUI*
_Nappgui_Start(void)
{
    AppGUI *pApp;
    Panel *pPanel;

#ifdef _WIN32
    _Win32_SetupConsole();
#endif

    /* Core initialization */
    pApp = heap_new0(AppGUI);
    cassert_no_null(pApp);

    pApp->pAppCore            = AppCore_create();
    AppCore_init(pApp->pAppCore);
    pApp->pAppCore->pOwner    = (void*)pApp;
    pApp->pAppCore->cbUIEvent = _Callback_UIEvent;

    /* Debug */
    pApp->pDebugData     = heap_new0(GUIDebugData);
    cassert_no_null(pApp->pDebugData);

    /* AppGUI ... */
    pApp->pWindows       = heap_new0(AppGUIWindows);
    cassert_no_null(pApp->pWindows);

    pApp->pWidgets       = heap_new0(AppGUIWidgets);
    cassert_no_null(pApp->pWidgets);

    pApp->pElementSets   = heap_new0(AppGUIElementDB);
    cassert_no_null(pApp->pElementSets);

    pApp->pLocalizations = heap_new0(AppGUILocalization);
    cassert_no_null(pApp->pLocalizations);

    /* AppGUI > AppGUIElementDB */
    pApp->pElementSets->pOptElementArray    = heap_new_n0(MAX_OPT_ELEMS, GUIOptElement);
    cassert_no_null(pApp->pElementSets->pOptElementArray);

    pApp->pElementSets->pToolElementArray   = heap_new_n0(MAX_TOOL_ELEMS, GUIToolElement);
    cassert_no_null(pApp->pElementSets->pToolElementArray);

    pApp->pElementSets->pModElementArray    = heap_new_n0(MAX_MOD_ELEMS, GUIModElement);
    cassert_no_null(pApp->pElementSets->pModElementArray);

    /* NappGUI */
    inet_start();
    gui_respack(res_app_respack);
    gui_language("");

    /* AmberLauncher */
    AutoUpdate_Init();
    AmberLauncher_Start(pApp->pAppCore);

    /* Main Window */
    pPanel                  = _Panel_GetRoot(pApp);
    pApp->pWindows->pWindow = window_create(
        ekWINDOW_TITLE |
        ekWINDOW_EDGE |
        ekWINDOW_CLOSE |
        ekWINDOW_ESC |
        ekWINDOW_RETURN
    );

    window_hotkey(pApp->pWindows->pWindow, ekKEY_F6, 0,
                  listener(pApp, Callback_OnWindowHotkeyF6, AppGUI));
    window_panel(pApp->pWindows->pWindow, pPanel);
    window_title(pApp->pWindows->pWindow, TXT_TITLE_LAUNCHER);
    /* window_origin(pApp->pWindow, v2df(800, 400)); */
    window_OnClose(pApp->pWindows->pWindow,
                   listener(pApp, _Callback_OnCloseMainWindow, AppGUI));
    window_show(pApp->pWindows->pWindow);
    _window_center(pApp->pWindows->pWindow);

    return pApp;
}

static void 
_Nappgui_End(AppGUI **pApp)
{
    size_t i;
    size_t j;

    /* AppGUI > Tweaker */
    for(i = 0; i < APP_MAX_ELEMENTS; i++)
    {
        GUITweaker *pTweaker = &(*pApp)->tGUITweaker[i];
        str_destopt(&pTweaker->sTag);
        str_destopt(&pTweaker->sTweakerInfo);

        for(j = 0; j < APP_MAX_ELEMENTS; j++)
        {
            str_destopt(&pTweaker->sImageTitle[j]);
            str_destopt(&pTweaker->sImagePath[j]);
        }
    }

    /* NappGUI ... */
    inet_finish();

    /* Amber Launcher ... */
    AmberLauncher_End((*pApp)->pAppCore);

    /* AppGUI > Localisation */
    if (IS_VALID((*pApp)->pLocalizations))
    {
        heap_delete(&(*pApp)->pLocalizations, AppGUILocalization);
    }

    /* AppGUI > Element Sets */
    if (IS_VALID((*pApp)->pElementSets))
    {
        /* Options */
        for (i = 0; i < MAX_OPT_ELEMS; ++i)
        {
            GUIOptElement *pElem = &(*pApp)->pElementSets->pOptElementArray[i];

            str_destopt(&pElem->sKeyID);
            str_destopt(&pElem->sOutputString);
            str_destopt(&pElem->sTitle);

            for (j = 0; j < APP_MAX_ELEMENTS; ++j)
                str_destopt(&pElem->sOptTitle[j]);
        }
        heap_delete_n(&(*pApp)->pElementSets->pOptElementArray, MAX_OPT_ELEMS, GUIOptElement);
        (*pApp)->pElementSets->pOptElementArray = NULL;

        /* Tools */
        for (i = 0; i < MAX_TOOL_ELEMS; ++i)
        {
            GUIToolElement *pElem = &(*pApp)->pElementSets->pToolElementArray[i];

            str_destopt(&pElem->sIcon);
            str_destopt(&pElem->sIconDark);
            str_destopt(&pElem->sTitle);
            str_destopt(&pElem->sDescription);
        }
        heap_delete_n(&(*pApp)->pElementSets->pToolElementArray, MAX_TOOL_ELEMS, GUIToolElement);
        (*pApp)->pElementSets->pToolElementArray = NULL;

        /* Mods */
        for (i = 0; i < MAX_MOD_ELEMS; ++i)
        {
            GUIModElement *pElem = &(*pApp)->pElementSets->pModElementArray[i];

            str_destopt(&pElem->sID);
            str_destopt(&pElem->sName);
            str_destopt(&pElem->sAuthor);
            str_destopt(&pElem->sDescription);
            str_destopt(&pElem->sVersion);
            str_destopt(&pElem->sWebsite);
            str_destopt(&pElem->sRoot);
            str_destopt(&pElem->sGame);
            str_destopt(&pElem->sScreenshot);
        }
        heap_delete_n(&(*pApp)->pElementSets->pModElementArray, MAX_MOD_ELEMS, GUIModElement);
        (*pApp)->pElementSets->pModElementArray = NULL;

        /* DB */
        heap_delete(&(*pApp)->pElementSets, AppGUIElementDB);
    }

    /* AppGUI > Widgets */
    if (IS_VALID((*pApp)->pWidgets))
    {
        str_destopt(&(*pApp)->pWidgets->pString);
        heap_delete(&(*pApp)->pWidgets, AppGUIWidgets);
    }

    /* AppGUI > Windows */
    if (IS_VALID((*pApp)->pWindows))
    {
        if ((*pApp)->pWindows->pWindowModal)
        {
            window_destroy(&(*pApp)->pWindows->pWindowModal);
        }
        window_destroy(&(*pApp)->pWindows->pWindow);

        heap_delete(&(*pApp)->pWindows, AppGUIWindows);
    }

    /* Debug Data */
    str_destopt(&(*pApp)->pDebugData->sInputString);
    heap_delete(&(*pApp)->pDebugData, GUIDebugData);
    (*pApp)->pDebugData = NULL;

    /* Finalize */
    AppCore_free(&(*pApp)->pAppCore);
    heap_delete(pApp, AppGUI);
}

static uint32_t
_Nappgui_ShowModal( AppGUI *pApp, Panel *pPanel, const char* sTitle)
{
    uint32_t dRet;

    pApp->pWindows->pWindowModal = window_create(
        ekWINDOW_STD | ekWINDOW_CLOSE | ekWINDOW_ESC | ekWINDOW_RETURN);
    pApp->pWindows->pPanelModal = pPanel;

    window_panel(pApp->pWindows->pWindowModal, pPanel);
    window_title(pApp->pWindows->pWindowModal, sTitle);
    window_OnClose(pApp->pWindows->pWindowModal,
                   listener(pApp, _Callback_OnCloseModalWindow, AppGUI));
    _window_center(pApp->pWindows->pWindowModal);
    dRet = window_modal(pApp->pWindows->pWindowModal, pApp->pWindows->pWindow);
    window_destroy(&pApp->pWindows->pWindowModal);

    return dRet;
}

/******************************************************************************
 * STATIC CALLBACK DEFINITIONS
 ******************************************************************************/

static void
_Callback_OnButtonMainWindow(AppGUI* pApp, Event* e)
{
    Button *pButton     = event_sender(e, Button);
    ESidebarButton dTag = (ESidebarButton)button_get_tag(pButton);
    bool_t bVarRet;

    bVarRet = (bool_t)AmberLauncher_ProcessUISideButton(pApp->pAppCore, dTag);
    if (bVarRet == FALSE)
    {
        return;
    }

    switch(dTag)
    {
        case CSIDEBUTTON_MAIN:
            Panel_Set(pApp, CPANEL_MAIN, FLAG_PANEL_NONE);
            break;
        case CSIDEBUTTON_MODS:
            break;
        case CSIDEBUTTON_SETTINGS:
            break;
        case CSIDEBUTTON_TOOLS:
            break;
        case CSIDEBUTTON_UPDATE:
            break;
        case CSIDEBUTTON_WEB_HOMEPAGE:
            osapp_open_url("https://mightandmagicmod.com");
            break;
        case CSIDEBUTTON_WEB_DISCORD:
            osapp_open_url("https://discord.gg/MhmZGrGxV4");
            break;
        case CSIDEBUTTON_NULL:
        case CSIDEBUTTON_MAX:
        default:
            break;
    }
}

static void 
_Callback_OnCloseMainWindow(AppGUI *pApp, Event *e)
{
    const EvWinClose *p = event_params(e, EvWinClose);
    cassert_no_null(pApp);

    switch (p->origin)
    {
        case ekGUI_CLOSE_ESC:
        case ekGUI_CLOSE_INTRO:
        case ekGUI_CLOSE_BUTTON:
        case ekGUI_CLOSE_DEACT:
        default:
            osapp_finish();
            break;
    }
}

static void 
_Callback_OnCloseModalWindow(AppGUI *pApp, Event *e)
{
    const EvWinClose *p = event_params(e, EvWinClose);
    bool_t *pResult     = event_result(e, bool_t);
    cassert_no_null(pApp);

    switch (p->origin)
    {
        case ekGUI_CLOSE_ESC:
        case ekGUI_CLOSE_BUTTON:

            if (pApp->eCurrentUIEvent == UIEVENT_MODAL_UPDATER)
            {
                pApp->pWidgets->pTextView = NULL;
            }
            break;

        case ekGUI_CLOSE_INTRO:

            if (pApp->eCurrentUIEvent == UIEVENT_DEBUG)
            {
                *pResult = FALSE; /* prevents from closing window */

                Callback_OnButtonModalDebug(pApp, e);
                return;
            }
            break;
            
        case ekGUI_CLOSE_DEACT:
        default:
            *pResult = FALSE; /* prevents from closing window */
            break;
    }
}


static SVarTable*
_SVar_TableNew(size_t dInitialCap)
{
    SVarTable *pTable = (SVarTable*)calloc(1, sizeof(SVarTable));
    assert(IS_VALID(pTable));

    pTable->dCount    = 0;
    pTable->pEntries  = (SVarTableEntry*)calloc(dInitialCap ? dInitialCap : 1,
                          sizeof(SVarTableEntry));
    assert(IS_VALID(pTable->pEntries));

    return pTable;
}

static void
_SVar_TableAdd(SVarTable *pTable, const char *sKey, const SVar *pVal)
{
    pTable->pEntries[pTable->dCount].tKey.eType            = CTYPE_CONST_CHAR;
    pTable->pEntries[pTable->dCount].tKey.uData._constchar = sKey;
    pTable->pEntries[pTable->dCount].tKey.dSize            = strlen(sKey)+1;

    /* shallow-copy value */
    pTable->pEntries[pTable->dCount].tValue = *pVal;
    ++pTable->dCount;
}

static SVarKeyBundle
_Callback_UIEvent(
    AppCore *pAppCore,
    EUIEventType eEventType,
    const SVar *pUserData,
    const unsigned int dNumArgs)
{
    AppGUI *pApp;
    SVarKeyBundle tRetVal;
    uint32_t dModalRetVal;
    const char *sStatusKey = "status";

    SVARKEYB_INIT(tRetVal);

    assert(IS_VALID(pAppCore));
    pApp = (AppGUI*)pAppCore->pOwner;

    assert(eEventType >= 0);

    #ifdef __DEBUG__
    if (eEventType < UIEVENT_MAX)
    {
        _al_printf(pApp,"UI EVENT: %s\n", EUIEventTypeStrings[eEventType]);
    }
    #endif

    if (pApp->pWidgets->pString)
    {
        str_destroy(&pApp->pWidgets->pString);
    }

    pApp->eCurrentUIEvent = eEventType;

    switch(eEventType)
    {
        case UIEVENT_NULL:
            break;
        case UIEVENT_MODAL_CLOSE:
            window_stop_modal(pApp->pWindows->pWindowModal, ekGUI_CLOSE_ESC);
            break;
        case UIEVENT_DEBUG:
            {
                _Nappgui_ShowModal
                (
                    pApp,
                    Panel_GetModalDebug(pApp),
                    TXT_TITLE_DEBUG
                );
            }
            break;
        case UIEVENT_PRINT:
            {
                const char* sMessage;

                assert(pUserData);
                assert(dNumArgs >= 1);

                sMessage = SVAR_GET_CONSTCHAR(pUserData[0]);
                if (str_empty_c(sMessage))
                {
                    SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                    break;
                }

                _al_printf(pApp,"%s\n", sMessage);
                SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
            }
            break;

        case UIEVENT_MAIN:
            {
                GUIThread_SchedulePanelSet(pApp, CPANEL_MAIN, FLAG_PANEL_NONE);
                SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
            }
            break;

        case UIEVENT_PLAY:
            {
                AmberLauncher_Play(pApp->pAppCore);
                SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
            }
            break;
        case UIEVENT_EXITAPP:
            {
                osapp_finish();
            }
            break;

        case UIEVENT_AUTOCONFIG:
            {
                FPanelFlags dFlags = FLAG_PANEL_NONE;

                /* Button state - "Configure" or "Next"? */
                if (IS_VALID(pUserData) && dNumArgs >= 1)
                {
                    bool_t bButtonNext = (bool_t)SVAR_GET_BOOL(pUserData[0]);
                    if (bButtonNext)
                    {
                        FLAG_SET(dFlags, FLAG_PANEL_STATE_END);
                    }
                }

                GUIThread_SchedulePanelSet(pApp, CPANEL_AUTOCONFIG, dFlags);
                SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
            }
            break;

        case UIEVENT_MODAL_MESSAGE:
            {
                assert(pUserData);
                assert(dNumArgs >= 1);

                pApp->pWidgets->pString = str_c(SVAR_GET_CONSTCHAR(pUserData[0]));

                dModalRetVal = _Nappgui_ShowModal
                (
                    pApp,
                    Panel_GetModalMessage(pApp),
                    TXT_TITLE_MESSAGE
                );
                switch (dModalRetVal)
                {
                    default:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
                        break;
                }
            }
            break;

        case UIEVENT_MODAL_QUESTION:
            {
                assert(pUserData);
                assert(dNumArgs >= 1);

                pApp->pWidgets->pString = str_c(SVAR_GET_CONSTCHAR(pUserData[0]));

                dModalRetVal = _Nappgui_ShowModal
                (
                    pApp,
                    Panel_GetModalQuestion(pApp),
                    TXT_TITLE_QUESTION
                );
                switch (dModalRetVal)
                {
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        break;
                    default:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
                        break;
                }
            }
            break;

        case UIEVENT_MODAL_GAMENOTFOUND:
            {
                const char *sPathKey = "path";

                dModalRetVal = _Nappgui_ShowModal
                (
                    pApp,
                    Panel_GetModalGameNotFound(pApp),
                    TXT_TITLE_GAMENOTFOUND
                );
                switch (dModalRetVal)
                {
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        SVARKEYB_NULL(tRetVal, sPathKey);
                        break;
                    default:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
                        if (pApp->pWidgets->pString)
                        {
                            const char* pGamePath = tc(pApp->pWidgets->pString);
                            SVARKEYB_CONSTCHAR(tRetVal, sPathKey, pGamePath);
                            break;
                        }
                        SVARKEYB_NULL(tRetVal, sPathKey);
                        break;
                }
            }
            break;

        case UIEVENT_MODAL_TWEAKER:
            {
                size_t i;
                SVarTable *pTable = NULL;
                size_t dGroupCount = 0;

                assert(pUserData);
                assert(dNumArgs == 1);
                assert(pUserData[0].eType == CTYPE_LUATABLE);

                /* Idea: Process multidata lua table as argument */
                pTable = (SVarTable *)SVAR_GET_LUATABLE(pUserData[0]);

                _al_printf(pApp, "Processing %zu groups\n", pTable->dCount);

                /* Process each group */
                for (i = 0; i < pTable->dCount && i < APP_MAX_ELEMENTS; ++i)
                {
                    SVarTable *pGroupTable;
                    const char *sGroupInfo = NULL;
                    size_t k;
                    size_t dOptionIndex = 0;

                    if (pTable->pEntries[i].tValue.eType != CTYPE_LUATABLE)
                    {
                        continue;
                    }

                    pGroupTable = (SVarTable *)SVAR_GET_LUATABLE(pTable->pEntries[i].tValue);

                    /* Clear any existing data for this tweaker group */
                    if (pApp->tGUITweaker[i].sTweakerInfo)
                    {
                        str_destroy(&pApp->tGUITweaker[i].sTweakerInfo);
                        pApp->tGUITweaker[i].sTweakerInfo = NULL;
                    }
                    if (pApp->tGUITweaker[i].sTag)
                    {
                        str_destroy(&pApp->tGUITweaker[i].sTag);
                        pApp->tGUITweaker[i].sTag = NULL;
                    }

                    for (k = 0; k < APP_MAX_ELEMENTS; ++k)
                    {
                        if (pApp->tGUITweaker[i].sImageTitle[k])
                        {
                            str_destroy(&pApp->tGUITweaker[i].sImageTitle[k]);
                            pApp->tGUITweaker[i].sImageTitle[k] = NULL;
                        }
                        if (pApp->tGUITweaker[i].sImagePath[k])
                        {
                            str_destroy(&pApp->tGUITweaker[i].sImagePath[k]);
                            pApp->tGUITweaker[i].sImagePath[k] = NULL;
                        }
                    }

                    /* Extract group info and options */
                    for (k = 0; k < pGroupTable->dCount; ++k)
                    {
                        if (pGroupTable->pEntries[k].tValue.eType == CTYPE_CONST_CHAR)
                        {
                            const char *sKey = SVAR_GET_CONSTCHAR(pGroupTable->pEntries[k].tKey);

                            if (strcmp(sKey, "info") == 0)
                            {
                                sGroupInfo = SVAR_GET_CONSTCHAR(pGroupTable->pEntries[k].tValue);
                                pApp->tGUITweaker[i].sTweakerInfo = str_c(sGroupInfo);
                            }
                            else if (strcmp(sKey, "tag") == 0)
                            {
                                const char *sTagName = SVAR_GET_CONSTCHAR(pGroupTable->pEntries[k].tValue);
                                pApp->tGUITweaker[i].sTag = str_c(sTagName);
                            }
                        }
                        else if (pGroupTable->pEntries[k].tValue.eType == CTYPE_LUATABLE)
                        {
                            SVarTable *pOptionTable;
                            const char *sOptionName = NULL;
                            const char *sImagePath = NULL;
                            size_t m;

                            if (dOptionIndex >= APP_MAX_ELEMENTS)
                            {
                                continue;
                            }

                            pOptionTable = (SVarTable *)SVAR_GET_LUATABLE(pGroupTable->pEntries[k].tValue);

                            /* Extract option details */
                            for (m = 0; m < pOptionTable->dCount; ++m)
                            {
                                const char *sKey = SVAR_GET_CONSTCHAR(pOptionTable->pEntries[m].tKey);

                                if (strcmp(sKey, "optionName") == 0)
                                {
                                    sOptionName = SVAR_GET_CONSTCHAR(pOptionTable->pEntries[m].tValue);
                                }
                                else if (strcmp(sKey, "imagePath") == 0)
                                {
                                    sImagePath = SVAR_GET_CONSTCHAR(pOptionTable->pEntries[m].tValue);
                                }
                            }

                            /* Store option in current group's tweaker */
                            if (sOptionName)
                            {
                                pApp->tGUITweaker[i].sImageTitle[dOptionIndex] = str_c(sOptionName);
                            }
                            if (sImagePath)
                            {
                                pApp->tGUITweaker[i].sImagePath[dOptionIndex] = str_c(sImagePath);
                            }

                            _al_printf(pApp, "Group %zu - Option %zu: %s, Asset: %s\n",
                                        i + 1, dOptionIndex + 1,
                                        sOptionName ? sOptionName : "Unknown",
                                        sImagePath ? sImagePath : "No image");

                            dOptionIndex++;
                        }
                    }

                    pApp->tGUITweaker[i].dMaxOptions = (unsigned int)dOptionIndex;

                    _al_printf(pApp, "Group %zu Info: %s (%zu options)\n",
                                i + 1,
                                sGroupInfo ? sGroupInfo : "No description",
                                dOptionIndex);

                    dGroupCount++;
                }

                /* Set the current page to first group */
                pApp->dPage     = 0;
                pApp->dPageMax  = (unsigned int)dGroupCount;
                for(i = 0; i < APP_MAX_ELEMENTS; i++)
                {
                    pApp->tGUITweaker[i].dSelectedOption    = 0;
                }

                /* Set initial image to first option of first group if available */
                if (dGroupCount > 0 && pApp->tGUITweaker[0].sImagePath[0])
                {
                    str_upd(&pApp->pWidgets->pString, tc(pApp->tGUITweaker[0].sImagePath[0]));
                }

                _al_printf(pApp, "Total groups processed: %zu\n", dGroupCount);

                /* Show modal dialog */
                dModalRetVal = _Nappgui_ShowModal(
                    pApp,
                    Panel_GetModalTweaker(pApp),
                    TXT_TITLE_TWEAKER
                );

                switch (dModalRetVal)
                {
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        break;
                    default:
                        {
                            const char *sOptName[APP_MAX_ELEMENTS] =
                            {
                                "opt1",
                                "opt2",
                                "opt3",
                                "opt4",
                                "opt5",
                                "opt6",
                                "opt7",
                                "opt8",
                            };

                            SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
                            for(i = 0; i < pApp->dPageMax; i++)
                            {
                                const bool_t bIsEmpty = !str_empty(pApp->tGUITweaker[i].sTag);
                                SVARKEYB_INT(tRetVal, 
                                    bIsEmpty ? 
                                        tc(pApp->tGUITweaker[i].sTag) : 
                                        sOptName[i], 
                                    (int)pApp->tGUITweaker[i].dSelectedOption);
                            }
                        }
                        break;
                }
            }
            break;

        case UIEVENT_MODAL_LOCALISATION:
            {
                size_t i, k;
                SVarTable *pTable;

                assert(pUserData);
                assert(dNumArgs == 1);
                assert(pUserData[0].eType == CTYPE_LUATABLE);

                pTable = (SVarTable *)SVAR_GET_LUATABLE(pUserData[0]);

                /* Reset */
                pApp->pLocalizations->dLocaleCount = 0;
                memset(pApp->pLocalizations->dLocaleSelected, 0, sizeof(pApp->pLocalizations->dLocaleSelected));

                /* Fill pApp->tLocale[] from Lua */
                for (i = 0; i < pTable->dCount && i < APP_MAX_LOCALES; ++i)
                {
                    SVarTable *pLoc;
                    const char_t *sCode;
                    const char_t *sCorePath;
                    const char_t *sModPath;

                    if (pTable->pEntries[i].tValue.eType != CTYPE_LUATABLE)
                    {
                        continue;
                    }

                    pLoc = (SVarTable *)SVAR_GET_LUATABLE(
                                            pTable->pEntries[i].tValue);

                    /* read one localisation record */
                    for (k = 0; k < pLoc->dCount; ++k)
                    {
                        const char *key =
                            SVAR_GET_CONSTCHAR(pLoc->pEntries[k].tKey);

                        if (strcmp(key, "code") == 0)
                            sCode = SVAR_GET_CONSTCHAR(pLoc->pEntries[k].tValue);
                        else if (strcmp(key, "core") == 0)
                            sCorePath = SVAR_GET_CONSTCHAR(pLoc->pEntries[k].tValue);
                        else if (strcmp(key, "mod") == 0)
                            sModPath = SVAR_GET_CONSTCHAR(pLoc->pEntries[k].tValue);
                    }

                    if (sCode)
                    {
                        LangInfo* dst = &pApp->pLocalizations
                                             ->tLocale[pApp->pLocalizations
                                                           ->dLocaleCount++];
                        dst->sCode      = sCode;
                        dst->sCorePath  = sCorePath;
                        dst->sModPath   = sModPath;
                        dst->bCore      = !str_empty_c(sCorePath);
                        dst->bMod       = !str_empty_c(sModPath);

                        _al_printf(pApp,
                            "Language detected: %s\n\tCore path: %s\n\tMod Path: %s\n",
                            sCode,sCorePath,sModPath);
                    }
                }

                /* note: pop-ups will be filled from tLocale[] */
                dModalRetVal = _Nappgui_ShowModal(
                                    pApp,
                                    Panel_GetModalLocalisation(pApp),
                                    TXT_TITLE_LOCALISATION );

                switch (dModalRetVal)
                {
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        break;

                    default:
                    {
                        /* gather what the user picked */
                        uint32_t dCore  = pApp->pLocalizations->dLocaleSelected[LOC_CORE];
                        uint32_t dMod   = pApp->pLocalizations->dLocaleSelected[LOC_MOD];
                        SVARKEYB_BOOL(tRetVal,sStatusKey, CTRUE);
                        SVARKEYB_CONSTCHAR(tRetVal,"core",  pApp->pLocalizations->tLocale[dCore].sCode);
                        SVARKEYB_CONSTCHAR(tRetVal,"mod",   pApp->pLocalizations->tLocale[dMod].sCode);
                    }
                        break;
                }
            }
            break;

        case UIEVENT_MODAL_OPTIONS:
            {
                static const char *sKeyTitle      = "title";
                static const char *sKeyID         = "id";
                static const char *sKeyType       = "type";
                static const char *sKeyOptTitle   = "optTitle";
                static const char *sKeyDefaultVal = "default";

                SVarTable     *pRootTable;
                size_t         dElemCount;
                size_t         dSection;
                unsigned int   i;
                unsigned int   j;

                /* pre-checks + init */
                cassert_no_null(pUserData);
                assert(dNumArgs == 1);
                assert(pUserData[0].eType == CTYPE_LUATABLE);

                pRootTable  = (SVarTable *)SVAR_GET_LUATABLE(pUserData[0]);
                dElemCount  = 0U;

                for (i = 0; i < MAX_OPT_ELEMS; ++i)
                {
                    GUIOptElement *pElem = &pApp->pElementSets->pOptElementArray[i];

                    str_destopt(&pElem->sKeyID);
                    str_destopt(&pElem->sOutputString);
                    str_destopt(&pElem->sTitle);

                    for (j = 0; j < APP_MAX_ELEMENTS; ++j)
                        str_destopt(&pElem->sOptTitle[j]);
                }
                memset(pApp->pElementSets->pOptElementArray, 0, sizeof(GUIOptElement) * MAX_OPT_ELEMS);

                /* root table */
                for (dSection = 0;
                    dSection < pRootTable->dCount && dElemCount < MAX_OPT_ELEMS;
                    ++dSection)
                {
                    SVarTable *pSection;
                    size_t     dEntry;

                    if (pRootTable->pEntries[dSection].tValue.eType != CTYPE_LUATABLE)
                    {
                        continue;
                    }

                    pSection = (SVarTable *)SVAR_GET_LUATABLE(
                                            pRootTable->pEntries[dSection].tValue);

                    /* per-table sub entries processing */
                    for (dEntry = 0;
                        dEntry < pSection->dCount && dElemCount < MAX_OPT_ELEMS;
                        ++dEntry)
                    {
                        SVarTable     *pElem;
                        GUIOptElement *pDst;
                        size_t         iField;
                        size_t         dOptIndex;
                        bool_t         bTypeWasSet;

                        if (pSection->pEntries[dEntry].tValue.eType != CTYPE_LUATABLE)
                        {
                            continue;
                        }

                        pElem               = (SVarTable *)SVAR_GET_LUATABLE(
                                                pSection->pEntries[dEntry].tValue);
                        pDst                = &pApp->pElementSets->pOptElementArray[dElemCount];

                        memset(pDst, 0, sizeof(GUIOptElement));
                        dOptIndex   = 0U;
                        bTypeWasSet = FALSE;

                        /* walk through every key/value in the element table */
                        for (iField = 0; iField < pElem->dCount; ++iField)
                        {
                            const char *sValKey;

                            sValKey = SVAR_GET_CONSTCHAR(pElem->pEntries[iField].tKey);

                            if (strcmp(sValKey, sKeyTitle) == 0)
                            {
                                pDst->sTitle =
                                    str_c(SVAR_GET_CONSTCHAR(pElem->pEntries[iField].tValue));
                            }
                            else if (strcmp(sValKey, sKeyID) == 0)
                            {
                                pDst->sKeyID =
                                    str_c(SVAR_GET_CONSTCHAR(pElem->pEntries[iField].tValue));
                            }
                            else if (strcmp(sValKey, sKeyType) == 0)
                            {
                                /* enum comes from Lua as a double */
                                pDst->eType  = (EUIWidgetType)
                                            SVAR_GET_DOUBLE(pElem->pEntries[iField].tValue);
                                bTypeWasSet  = TRUE;
                            }
                            else if (strncmp(sValKey, sKeyOptTitle, 8) == 0)
                            {
                                switch (pElem->pEntries[iField].tValue.eType)
                                {
                                    case CTYPE_CONST_CHAR:
                                        pDst->sOptTitle[dOptIndex++] =
                                            str_c(SVAR_GET_CONSTCHAR(
                                                    pElem->pEntries[iField].tValue));
                                        break;

                                    case CTYPE_DOUBLE:
                                        pDst->sOptTitle[dOptIndex++] =
                                            str_printf("%.4g",
                                                    SVAR_GET_DOUBLE(
                                                        pElem->pEntries[iField].tValue));
                                        break;

                                    case CTYPE_LUATABLE:
                                    {
                                        /* Handle optTitle = { "A", "B", ... } */
                                        SVarTable *pOpts;
                                        size_t     q;

                                        pOpts = (SVarTable *)SVAR_GET_LUATABLE(
                                                                pElem->pEntries[iField].tValue);

                                        for (q = 0;
                                            q < pOpts->dCount && dOptIndex < APP_MAX_ELEMENTS;
                                            ++q)
                                        {
                                            if (pOpts->pEntries[q].tValue.eType == CTYPE_CONST_CHAR)
                                            {
                                                pDst->sOptTitle[dOptIndex++] =
                                                    str_c(SVAR_GET_CONSTCHAR(
                                                            pOpts->pEntries[q].tValue));
                                            }
                                        }
                                    }
                                    break;

                                    case CTYPE_NULL:
                                    case CTYPE_CHAR:
                                    case CTYPE_UNSIGNED_CHAR:
                                    case CTYPE_SHORT:
                                    case CTYPE_UNSIGNED_SHORT:
                                    case CTYPE_BOOL:
                                    case CTYPE_INT:
                                    case CTYPE_UNSIGNED_INT:
                                    case CTYPE_LONG:
                                    case CTYPE_FLOAT:
                                    case CTYPE_LONG_DOUBLE:
                                    case CTYPE_UNSIGNED_LONG:
                                    case CTYPE_LONG_LONG:
                                    case CTYPE_UNSIGNED_LONG_LONG:
                                    case CTYPE_VOID:
                                    case CTYPE_LUAREF:
                                    case CTYPE_MAX:
                                    default:
                                        /* ignore unsupported types */
                                        break;
                                }
                            }
                            else if (strcmp(sValKey, sKeyDefaultVal) == 0)
                            {
                                const SVar *pVar = &pElem->pEntries[iField].tValue;

                                if (SVAR_IS_DOUBLE(*pVar))
                                {
                                    pDst->dChoice =
                                        (unsigned int)SVAR_GET_DOUBLE(*pVar);
                                }
                                else if (SVAR_IS_CONSTCHAR(*pVar))
                                {
                                    pDst->sOptTitle[0] =
                                        str_c(SVAR_GET_CONSTCHAR(*pVar));
                                    dOptIndex = 1U;
                                }
                            }
                        }

                        pDst->dNumOfOptions = (unsigned int)dOptIndex;
                        if (!bTypeWasSet)
                        {
                            pDst->eType = UI_WIDGET_NULL;
                        }

#ifdef __DEBUG__
                        _al_printf( pApp,
                                    "Elem %zu: title='%s' type=%d opts=%u\n",
                                    dElemCount,
                                    (pDst->sTitle ? tc(pDst->sTitle) : "(null)"),
                                    (int)pDst->eType,
                                    pDst->dNumOfOptions);
#endif

                        ++dElemCount;
                    }
                }

                /* Modal */
                dModalRetVal = _Nappgui_ShowModal(
                                pApp,
                                Panel_GetModalOptions(pApp),
                                TXT_TITLE_OPTIONS);

                switch (dModalRetVal)
                {
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        break;

                    default:
                    {
                        unsigned int u;

                        SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);

                        for (u = 0; u < dElemCount; ++u)
                        {
                            const GUIOptElement *pElem   = &pApp->pElementSets->pOptElementArray[u];
                            const char          *sOutKey =
                                (pElem->sKeyID != NULL) ? tc(pElem->sKeyID) : tc(pElem->sTitle);

                            switch (pElem->eType)
                            {
                                case UI_WIDGET_EDIT:
                                {
                                    const char *sVal =
                                        (pElem->sOutputString != NULL) ? tc(pElem->sOutputString) :
                                        (pElem->sOptTitle[0]  != NULL) ? tc(pElem->sOptTitle[0]) : "";
                                    SVARKEYB_CONSTCHAR(tRetVal, sOutKey, sVal);
                                }
                                break;

                                case UI_WIDGET_LISTBOX:
                                case UI_WIDGET_POPUP:
                                case UI_WIDGET_RADIO:
                                case UI_WIDGET_CHECKBOX:
                                    SVARKEYB_INT(tRetVal, sOutKey, (int)pElem->dChoice);
                                    break;

                                case UI_WIDGET_NULL:
                                case UI_WIDGET_MAX:
                                default:
                                    break;
                            }
                        }
                    }
                    break;
                }
            }
            break;

        case UIEVENT_MODAL_MODS:
            {
                static const char *sKeyAvail     = "available";
                static const char *sKeyActive    = "active";
                static const char *sKeyID        = "id";
                static const char *sKeyName      = "name";
                static const char *sKeyAuthor    = "author";
                static const char *sKeyDesc      = "description";
                static const char *sKeyVersion   = "version";
                static const char *sKeyWebsite   = "website";
                static const char *sKeyRoot      = "root";
                static const char *sKeyGame      = "game";
                static const char *sKeyScreenshot= "screenshot";
                static const char *sKeyOnInst    = "onInstall";
                static const char *sKeyOnUninst  = "onUninstall";
                static const char *sKeyOptions   = "options";

                SVarTable *pRoot        = NULL;
                SVarTable *pActive      = NULL;
                unsigned int dModCount  = 0U;
                size_t i;

                cassert_no_null(pUserData);
                assert(dNumArgs == 1);
                assert(pUserData[0].eType == CTYPE_LUATABLE);

                pRoot = (SVarTable *)SVAR_GET_LUATABLE(pUserData[0]);

                for (i = 0; i < MAX_MOD_ELEMS; ++i)
                {
                    GUIModElement *pElem = &pApp->pElementSets->pModElementArray[i];

                    str_destopt(&pElem->sID);
                    str_destopt(&pElem->sName);
                    str_destopt(&pElem->sAuthor);
                    str_destopt(&pElem->sDescription);
                    str_destopt(&pElem->sVersion);
                    str_destopt(&pElem->sWebsite);
                    str_destopt(&pElem->sRoot);
                    str_destopt(&pElem->sGame);
                    str_destopt(&pElem->sScreenshot);
                }
                memset(pApp->pElementSets->pModElementArray, 0, sizeof(GUIModElement) * MAX_MOD_ELEMS);

                /* locate active list first */
                for (i = 0; i < pRoot->dCount; ++i)
                {
                    if (strcmp(SVAR_GET_CONSTCHAR(pRoot->pEntries[i].tKey), sKeyActive) == 0 &&
                        pRoot->pEntries[i].tValue.eType == CTYPE_LUATABLE)
                    {
                        pActive = (SVarTable *)SVAR_GET_LUATABLE(pRoot->pEntries[i].tValue);
                        break;
                    }
                }

                /* process available mods */
                for (i = 0; i < pRoot->dCount && dModCount < MAX_MOD_ELEMS; ++i)
                {
                    if (strcmp(SVAR_GET_CONSTCHAR(pRoot->pEntries[i].tKey), sKeyAvail) != 0 ||
                        pRoot->pEntries[i].tValue.eType != CTYPE_LUATABLE)
                        continue;

                    {
                        SVarTable *pAvail = (SVarTable *)SVAR_GET_LUATABLE(pRoot->pEntries[i].tValue);
                        size_t     m;

                        for (m = 0; m < pAvail->dCount && dModCount < MAX_MOD_ELEMS; ++m)
                        {
                            if (pAvail->pEntries[m].tValue.eType != CTYPE_LUATABLE)
                                continue;

                            {
                                SVarTable      *pMod = (SVarTable *)SVAR_GET_LUATABLE(pAvail->pEntries[m].tValue);
                                GUIModElement  *pDst = &pApp->pElementSets->pModElementArray[dModCount];
                                size_t          k;

                                memset(pDst, 0, sizeof(GUIModElement));

                                for (k = 0; k < pMod->dCount; ++k)
                                {
                                    const char *sKey = SVAR_GET_CONSTCHAR(pMod->pEntries[k].tKey);

                                    if (strcmp(sKey, sKeyID) == 0)
                                        pDst->sID = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyName) == 0)
                                        pDst->sName = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyAuthor) == 0)
                                        pDst->sAuthor = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyDesc) == 0)
                                        pDst->sDescription = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyVersion) == 0)
                                        pDst->sVersion = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyWebsite) == 0)
                                        pDst->sWebsite = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyRoot) == 0)
                                        pDst->sRoot = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyGame) == 0)
                                        pDst->sGame = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyScreenshot) == 0)
                                        pDst->sScreenshot = str_c(SVAR_GET_CONSTCHAR(pMod->pEntries[k].tValue));
                                    else if (strcmp(sKey, sKeyOnInst) == 0 &&
                                            pMod->pEntries[k].tValue.eType == CTYPE_LUAREF)
                                        pDst->pOnInstallCb = &pMod->pEntries[k].tValue;
                                    else if (strcmp(sKey, sKeyOnUninst) == 0 &&
                                            pMod->pEntries[k].tValue.eType == CTYPE_LUAREF)
                                        pDst->pOnUninstallCb = &pMod->pEntries[k].tValue;
                                    else if (strcmp(sKey, sKeyOptions) == 0 &&
                                            pMod->pEntries[k].tValue.eType == CTYPE_LUATABLE)
                                        pDst->pOptionsTable = (SVarTable *)SVAR_GET_LUATABLE(pMod->pEntries[k].tValue);
                                }

                                /* mark active */
                                if (pActive && pDst->sID)
                                {
                                    size_t a;
                                    for (a = 0; a < pActive->dCount; ++a)
                                    {
                                        if (pActive->pEntries[a].tValue.eType == CTYPE_CONST_CHAR &&
                                            strcmp(tc(pDst->sID),
                                                SVAR_GET_CONSTCHAR(pActive->pEntries[a].tValue)) == 0)
                                        {
                                            pDst->bActive = TRUE;
                                            break;
                                        }
                                    }
                                }
#ifdef __DEBUG__
                                _al_printf(pApp,
                                    "Mod %zu: id='%s' active=%d\n",
                                    dModCount,
                                    pDst->sID ? tc(pDst->sID) : "(null)",
                                    (int)pDst->bActive);
#endif
                                ++dModCount;
                            }
                        }
                    }
                }

                dModalRetVal = _Nappgui_ShowModal(
                                    pApp,
                                    Panel_GetModalModManager(pApp),
                                    TXT_TITLE_MODS);

                switch (dModalRetVal)
                {
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        break;

                    default:
                    {
                        size_t u;
                        SVarTable *pActiveTbl;
                        SVarTable *pOptionsTbl;

                        SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);

                        pActiveTbl  = _SVar_TableNew(dModCount);
                        pOptionsTbl = _SVar_TableNew(dModCount);
                        
                        /* populate temporary tables */
                        for (u = 0; u < dModCount; ++u)
                        {
                            GUIModElement *pMod = &pApp->pElementSets->pModElementArray[u];
                            if (!pMod->sID)
                                continue;

                            /* active[{id}] = bool */
                            {
                                SVar vBool;
                                SVAR_BOOL(vBool, (CBOOL)pMod->bActive);
                                _SVar_TableAdd(pActiveTbl, tc(pMod->sID), &vBool);
                            }

                            /* options[{id}] = <options table> */
                            if (pMod->pOptionsTable)
                            {
                                const char *sid = tc(pMod->sID);
                                SVar vTbl;

                                /*--- new collapse logic ---*/
                                if (   pMod->pOptionsTable->dCount == 1
                                    && pMod->pOptionsTable->pEntries[0].tKey.eType == CTYPE_CONST_CHAR
                                    && strcmp(SVAR_GET_CONSTCHAR(
                                                pMod->pOptionsTable->pEntries[0].tKey), sid) == 0
                                    && pMod->pOptionsTable->pEntries[0].tValue.eType == CTYPE_LUATABLE )
                                {
                                    /* use the inner table directly */
                                    SVarTable *inner =
                                        (SVarTable *)SVAR_GET_LUATABLE(
                                            pMod->pOptionsTable->pEntries[0].tValue);
                                    SVAR_LUATABLE(vTbl, inner);
                                }
                                else
                                {
                                    /* keep original structure (multi-group case) */
                                    SVAR_LUATABLE(vTbl, pMod->pOptionsTable);
                                }

                                _SVar_TableAdd(pOptionsTbl, sid, &vTbl);
                            }
                        }

                        /* Push lua tables into return variable */
                        SVARKEYB_LUATABLE(tRetVal, "active",  pActiveTbl);

                        if (pOptionsTbl->dCount > 0)
                        {
                            SVARKEYB_LUATABLE(tRetVal, "options", pOptionsTbl);
                        }
                    }
                    break;
                }
            }
            break;

        case UIEVENT_MODAL_TOOLS:
            {
                static const char *sKeyID       = "id";
                static const char *sKeyIcon     = "iconPath";
                static const char *sKeyIconDark = "iconDarkPath";
                static const char *sKeyTitle    = "title";
                static const char *sKeyDesc     = "description";
                static const char *sKeyOnClick  = "onClick";
                unsigned int dToolCount         = 0U;
                unsigned int m;

                cassert_no_null(pUserData);
                assert(dNumArgs == 1);
                assert(pUserData[0].eType == CTYPE_LUATABLE);

                for (m = 0; m < MAX_TOOL_ELEMS; ++m)
                {
                    GUIToolElement *pElem = &pApp->pElementSets->pToolElementArray[m];

                    str_destopt(&pElem->sIcon);
                    str_destopt(&pElem->sIconDark);
                    str_destopt(&pElem->sTitle);
                    str_destopt(&pElem->sDescription);
                }
                memset(pApp->pElementSets->pToolElementArray, 0, sizeof(GUIToolElement) * MAX_TOOL_ELEMS);

                /* parse Lua table */
                {
                    SVarTable *pRoot;
                    size_t     i;

                    pRoot = (SVarTable *)SVAR_GET_LUATABLE(pUserData[0]);
                    for (i = 0; i < pRoot->dCount && dToolCount < APP_MAX_ELEMENTS; ++i)
                    {
                        if (pRoot->pEntries[i].tValue.eType == CTYPE_LUATABLE)
                        {
                            SVarTable       *pEntry;
                            GUIToolElement  *pDst;
                            size_t           k;

                            pEntry = (SVarTable *)SVAR_GET_LUATABLE(pRoot->pEntries[i].tValue);
                            pDst   = &pApp->pElementSets->pToolElementArray[dToolCount];
                            memset(pDst, 0, sizeof(GUIToolElement));

                            for (k = 0; k < pEntry->dCount; ++k)
                            {
                                const char *sKey =
                                    SVAR_GET_CONSTCHAR(pEntry->pEntries[k].tKey);

                                if (strcmp(sKey, sKeyID) == 0)
                                {
                                    pDst->dID = (uint32_t)SVAR_GET_DOUBLE(pEntry->pEntries[k].tValue);
                                }
                                else if (strcmp(sKey, sKeyIcon) == 0)
                                {
                                    pDst->sIcon =
                                        str_c(SVAR_GET_CONSTCHAR(pEntry->pEntries[k].tValue));
                                }
                                else if (strcmp(sKey, sKeyIconDark) == 0)
                                {
                                    pDst->sIconDark =
                                        str_c(SVAR_GET_CONSTCHAR(pEntry->pEntries[k].tValue));
                                }
                                else if (strcmp(sKey, sKeyTitle) == 0)
                                {
                                    pDst->sTitle =
                                        str_c(SVAR_GET_CONSTCHAR(pEntry->pEntries[k].tValue));
                                }
                                else if (strcmp(sKey, sKeyDesc) == 0)
                                {
                                    pDst->sDescription =
                                        str_c(SVAR_GET_CONSTCHAR(pEntry->pEntries[k].tValue));
                                }
                                else if (strcmp(sKey, sKeyOnClick) == 0)
                                {
                                    if (SVAR_IS_LUAREF(pEntry->pEntries[k].tValue))
                                    {
                                        pDst->tLuaRef = pEntry->pEntries[k].tValue;
                                    }
                                }
                            }

#ifdef __DEBUG__
                            _al_printf(pApp,
                                "Tool %zu: id=%u title='%s' icon='%s' iconDark='%s' onClick='%d'\n",
                                dToolCount,
                                (unsigned)pDst->dID,
                                pDst->sTitle    ? tc(pDst->sTitle) : "(null)",
                                pDst->sIcon     ? tc(pDst->sIcon)  : "(null)",
                                pDst->sIconDark ? tc(pDst->sIconDark)  : "(null)",
                                SVAR_GET_LUAREF(pDst->tLuaRef));
#endif
                            ++dToolCount;
                        }
                    }
                }

                dModalRetVal = _Nappgui_ShowModal(
                    pApp,
                    Panel_GetModalTools(pApp),
                    TXT_TITLE_TOOLS
                );

                SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
            }
            break;

        case UIEVENT_MODAL_UPDATER:
            {
                dModalRetVal = _Nappgui_ShowModal(
                    pApp,
                    Panel_GetModalUpdater(pApp),
                    TXT_TITLE_UPDATER
                );
                switch (dModalRetVal)
                {
                    case MODAL_UPDATE_APP:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
                        break;
                    case ekGUI_CLOSE_BUTTON:
                    case ekGUI_CLOSE_ESC:
                    case MODAL_CANCEL:
                    default:
                        SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
                        break;
                }
            }
            break;

        case UIEVENT_MAX:
        default:
            SVARKEYB_NULL(tRetVal, sStatusKey);

            _al_printf(pApp,
                "Behavior for %s (%d) is not defined!\n",
                eEventType < UIEVENT_MAX ? EUIEventTypeStrings[eEventType] : "?", eEventType);
            break;
    }

    pApp->eCurrentUIEvent = UIEVENT_NULL;

    UNUSED(pUserData);

    return tRetVal;
}


/******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************/

#include <osapp/osmain.h>
osmain(_Nappgui_Start, _Nappgui_End, "", AppGUI)

/* #include <osmain.h> */
/* osmain(_Nappgui_Start, _Nappgui_End, "", AppGUI) */

/* int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;
    FUNC_CHECK_APP_CREATE(_Nappgui_Start, AppGUI);
    FUNC_CHECK_DESTROY(_Nappgui_End, AppGUI);
    osmain_imp(
        (uint32_t)argc, (char_t **)argv, NULL, 0.,
        (FPtr_app_create)_Nappgui_Start,
        (FPtr_app_update)NULL,
        (FPtr_destroy)_Nappgui_End,
        (char_t *)"");
        
    return 0;
}
 */
