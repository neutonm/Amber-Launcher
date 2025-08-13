#include "core/core.hxx"
#include "core/stdint.h"
#include "draw2d/draw2d.hxx"
#include "draw2d/guictx.hxx"
#include "ext/miniz.h"
#include "gui/gui.hxx"
#include "sewer/types.hxx"
#include <AmberLauncherGUI.h>
#include <AmberLauncherCore.h>

#include <bits/stdint-uintn.h>
#include <bits/types/error_t.h>
#include <core/common.h>
#include <core/appcore.h>

#include <nappgui.h>
#include <res_app.h>

#include <assert.h>

#include <stdio.h>
#include <string.h>

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

const char* EUIEventTypeStrings[] = {
    "NULL",
    "PRINT",
    "MAIN",
    "PLAY",
    "AUTOCONFIG",
    "MODAL_MESSAGE",
    "MODAL_QUESTION",
    "MODAL_GAMENOTFOUND",
    "MODAL_TWEAKER",
    "MODAL_LOCALISATION",
    "MODAL_OPTIONS",
    NULL
};

static const int32_t TITLE_PNG_W = 672;
static const int32_t TITLE_PNG_H = 200;
#define MODAL_OPT_NULL  1000
#define MODAL_OPT_A     1001
#define MODAL_OPT_B     1002
#define MODAL_OPT_MAX   1030
#define MODAL_CANCEL    1031
#define MODAL_ACCEPT    1032
#define MODAL_BROWSE    1033
#define MODAL_PREVIOUS  1034
#define MODAL_NEXT      1035

/******************************************************************************
 * STATIC DECLARATIONS
 ******************************************************************************/

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

static int
_RowForPage(int dPage, int dPageMax);

static Panel*
_Panel_TweakerDescription(AppGUI *pApp);

static Panel*
_Panel_TweakerOptionsSelector(AppGUI *pApp);

static Panel*
_Panel_TweakerButtons(AppGUI *pApp);

static void
_LocalisationFillPopup(AppGUI *pApp, PopUp *pPop, LocTier eTier);

static uint32_t
_GUIThread_Main_Null(GUIAsyncTaskData *pThreadData);

static void
_GUIThread_End_PanelSetMain(GUIAsyncTaskData *pThreadData, const uint32_t dRValue);

/******************************************************************************
 * STATIC CALLBACK DECLARATIONS
 ******************************************************************************/

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
    switch (eType)
    {
    case CPANEL_NULL:
        pPanel = Panel_GetNull(pApp);
        break;

    case CPANEL_AUTOCONFIG:
        pPanel = Panel_GetAutoConfigure(pApp,dFlags);
        break;

    case CPANEL_MAIN:
        pPanel = Panel_GetMain(pApp);
        break;

    case CPANEL_IMAGEDEMO:
        pPanel = Panel_GetImageDemo(pApp);
        break;

    default:
        pPanel = Panel_GetNull(pApp);
        break;
    }

    pApp->pPanelMain = pPanel;
    layout_panel_replace(pApp->pLayoutMain, pPanel, 0, 1);

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
        if (IS_VALID(pApp->pTextView))
        {
            const char_t *sPreviousLog = textview_get_text(pApp->pTextView);
            textview_printf(pConsole, "%s", sPreviousLog);
        }
    }
    
    pApp->pTextView = pConsole;

    /* Label: Greetings Text */
    label_text(pLabelGreet, bIsStateEnd ? TXT_CONFIGSUCCESS : TXT_GREETINGS);
    label_font(pLabelGreet, pFontGreet);

    /* Button: Configure */
    button_text(pButtonCfg, bIsStateEnd ? TXT_BTN_NEXT : TXT_BTN_CONFIGURE);
    button_font(pButtonCfg, pFontCfg);
    button_vpadding (pButtonCfg, 16);
    button_tag(pButtonCfg, dFlags);
    button_OnClick(pButtonCfg, listener(pApp, Callback_OnButtonConfigure, AppGUI));
    
    /* Layout: Text */
    layout_margin(pLayoutTxt, 4);
    layout_label(pLayoutTxt, pLabelGreet, 0, 0);

    /* Layout: Main */
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
    Layout  *pLayoutMain = layout_create(1, 2);
    Layout  *pLayoutTxt  = layout_create(1,1);
    Label   *pLabelTemp  = label_create();
    Button  *pButtonPlay = button_push();
    Font    *pFontPlay   = font_system(24, ekFNORMAL | ekFPIXELS);

    pApp->pTextView = NULL;

    /* Label: Temp */
    label_text(pLabelTemp, TXT_GREETINGS);
    label_multiline(pLabelTemp, TRUE);

    /* Button: Play */
    button_text(pButtonPlay, TXT_BTN_PLAY);
    button_font(pButtonPlay, pFontPlay);
    button_vpadding (pButtonPlay, 16);
    button_OnClick(pButtonPlay, listener(pApp, Callback_OnButtonPlay, AppGUI));
    
    /* Layout: Text */
    layout_margin(pLayoutTxt, 4);
    layout_label(pLayoutTxt, pLabelTemp, 0, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, TITLE_PNG_W);
    /* layout_halign(pLayout, 0, 0, ekCENTER); */
    layout_valign(pLayoutMain, 0, 0, ekTOP);
    layout_layout(pLayoutMain, pLayoutTxt, 0, 0);
    layout_button(pLayoutMain, pButtonPlay, 0, 1);

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
    Image       *pImage;
    ferror_t    eError;

    assert(pApp->pString);

    /* Image */
    pImage = image_from_file(tc(pApp->pString), &eError);
    assert(eError == ekFOK);

    /* Image View */
    imageview_scale(pImageView, ekGUI_SCALE_AUTO);
    imageview_size(pImageView, s2df(480,320));
    imageview_image(pImageView, pImage);

    /* Layout: Main */
    layout_imageview(pLayoutMain, pImageView, 0, 0);

    panel_layout(pPanelMain, pLayoutMain);

    image_destroy(&pImage);

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

Panel*
Panel_GetModalMessage(AppGUI* pApp)
{
    Panel   *pPanelMain     = panel_create();
    Layout  *pLayoutMain    = layout_create(1,2);
    Label   *pLabelMessage  = label_create();
    Button  *pButtonOK      = button_push();

    /* Label: Message */
    label_text(pLabelMessage, TXT_NULL);
    if (pApp)
    {
        if (pApp->pString)
        {
            label_text(pLabelMessage, tc(pApp->pString));
        }
    }

    /* Button: OK */
    button_text(pButtonOK, TXT_BTN_OK);
    button_tag(pButtonOK, MODAL_ACCEPT);
    button_OnClick(pButtonOK, listener(pApp, Callback_OnButtonModalMessage, AppGUI));

    /* Layout: Main */
    layout_label(pLayoutMain, pLabelMessage,0,0);
    layout_button(pLayoutMain, pButtonOK, 0, 1);

    panel_layout(pPanelMain, pLayoutMain);

    unref(pApp);

    return pPanelMain;
}

Panel*
Panel_GetModalQuestion(AppGUI* pApp)
{
    Panel   *pPanelMain     = panel_create();
    Layout  *pLayoutMain    = layout_create(1,2);
    Layout  *pLayoutButtons = layout_create(2,1);
    Label   *pLabelMessage  = label_create();
    Button  *pButtonContinue= button_push();
    Button  *pButtonCancel  = button_push();

    /* Label: Message */
    label_text(pLabelMessage, TXT_NULL);
    if (pApp)
    {
        if (pApp->pString)
        {
            label_text(pLabelMessage, tc(pApp->pString));
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

    /* Layout: Buttons */
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonContinue, 1, 0);

    /* Layout: Main */
    layout_label(pLayoutMain, pLabelMessage,0,0);
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
    Layout      *pLayoutButtons = layout_create(2, 1);
    Edit        *pEditPath      = edit_create();
    Label       *pLabelInfo     = label_create();
    Button      *pButtonSubmit  = button_push();
    Button      *pButtonCancel  = button_push();
    Button      *pButtonBrowse  = button_push();
    Font        *pFontInfo      = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFontSubmit    = font_system(24, ekFNORMAL | ekFPIXELS);

    /* Label: Greetings Text */
    label_text(pLabelInfo, TXT_GAMENOTFOUND);
    label_font(pLabelInfo, pFontInfo);

    /* Edit: Game Path */
    pApp->pEdit = pEditPath;

    /* Button: Submit */
    button_text(pButtonSubmit, TXT_BTN_SUBMIT);
    button_font(pButtonSubmit, pFontSubmit);
    button_vpadding (pButtonSubmit, 16);
    button_tag(pButtonSubmit, MODAL_ACCEPT);
    button_OnClick(pButtonSubmit, listener(pApp, Callback_OnButtonModalGameNotFound, AppGUI));

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_font(pButtonCancel, pFontSubmit);
    button_vpadding (pButtonCancel, 16);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalGameNotFound, AppGUI));

    /* Button: Browse (file) */
    button_text(pButtonBrowse, TXT_BTN_BROWSE);
    button_font(pButtonBrowse, pFontInfo);
    button_tag(pButtonBrowse, MODAL_BROWSE);
    button_OnClick(pButtonBrowse, listener(pApp, Callback_OnButtonModalGameNotFound, AppGUI));

    /* Layout: Text */
    layout_margin(pLayoutTxt, 4);
    layout_label(pLayoutTxt, pLabelInfo, 0, 0);

    /* Layout: Game Path */
    layout_edit(pLayoutPath, pEditPath, 0,0);
    layout_button(pLayoutPath, pButtonBrowse, 1, 0);

    /* Layout: Buttons */
    layout_button(pLayoutButtons,pButtonCancel,0, 0);
    layout_button(pLayoutButtons,pButtonSubmit,1, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, 250);
    layout_layout(pLayoutMain, pLayoutTxt, 0, 0);
    layout_layout(pLayoutMain, pLayoutPath, 0, 1);
    layout_layout(pLayoutMain,pLayoutButtons,0, 2);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    if (pApp->pWindowModal != NULL)
    {
        window_defbutton(pApp->pWindowModal, pButtonSubmit);
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

    pTweaker            = &pApp->tGUITweaker[pApp->dPage];
    assert(IS_VALID(pTweaker));

    /* Label: Feature Description */
    label_multiline(pLabelDescription, TRUE);
    label_text(pLabelDescription, tc(pTweaker->pStringTweakerInfo));

    layout_label(pLayoutMain, pLabelDescription, 0, 0);

    /* Panel: Main */
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
    int i;

    pTweaker = &pApp->tGUITweaker[pApp->dPage];
    assert(IS_VALID(pTweaker));

    /* Buttons: Tweak Options */
    for (i = 0; i < pTweaker->dMaxOptions; i++)
    {
        if (!str_empty(pTweaker->pStringImageTitle[i]))
        {
            pButtonOpt[i] = button_radio();
            button_text(pButtonOpt[i], tc(pTweaker->pStringImageTitle[i]));
            button_tag(pButtonOpt[i], (MODAL_OPT_A + i) );
            button_OnClick(pButtonOpt[i], listener(pApp, Callback_OnButtonModalTweaker, AppGUI));
        }
    }

    /* Set active button */
    if (pTweaker->pStringImageTitle[pTweaker->dSelectedOption])
    {
        button_state(pButtonOpt[pTweaker->dSelectedOption], ekGUI_ON);
    }
    else if (pTweaker->pStringImageTitle[0])
    {
        button_state(pButtonOpt[0], ekGUI_ON);
    }

    /* Layout: Selector */
    for (i = 0; i < pTweaker->dMaxOptions; i++)
    {
        if (!str_empty(pTweaker->pStringImageTitle[i]))
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
    if (str_empty(pTweaker->pStringImageTitle[0]))
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
static int
_RowForPage(int dPage, int dPageMax)
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
    const int dRowForPage = _RowForPage(pApp->dPage, pApp->dPageMax);
    int dRow;

    pApp->pLayoutExtra  = pLayoutDynamicButtons;

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

    /* Button: Confirm */
    button_text(pButtonConfirm, TXT_BTN_ACCEPT);
    button_tag(pButtonConfirm, MODAL_ACCEPT);
    button_OnClick(pButtonConfirm, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

    button_text(pButtonFirstConfirm, TXT_BTN_ACCEPT);
    button_tag(pButtonFirstConfirm, MODAL_ACCEPT);
    button_OnClick(pButtonFirstConfirm, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

    /* Button: Previous */
    button_text(pButtonPrev, TXT_BTN_PREVIOUS);
    button_tag(pButtonPrev, MODAL_PREVIOUS);
    button_OnClick(pButtonPrev, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

    button_text(pButtonLastPrev, TXT_BTN_PREVIOUS);
    button_tag(pButtonLastPrev, MODAL_PREVIOUS);
    button_OnClick(pButtonLastPrev, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

    /* Button: Next */
    button_text(pButtonFirstNext, TXT_BTN_NEXT);
    button_tag(pButtonFirstNext, MODAL_NEXT);
    button_OnClick(pButtonFirstNext, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

    button_text(pButtonNext, TXT_BTN_NEXT);
    button_tag(pButtonNext, MODAL_NEXT);
    button_OnClick(pButtonNext, listener(pApp, Callback_OnButtonModalTweaker, AppGUI));

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
    layout_layout(pLayoutDynamicButtons, pLayoutSinglePage, 0, 0);
    layout_layout(pLayoutDynamicButtons, pLayoutFirstPage, 0, 1);
    layout_layout(pLayoutDynamicButtons, pLayoutNPage, 0, 2);
    layout_layout(pLayoutDynamicButtons, pLayoutLastPage, 0, 3);

    /* Layout: Main */
    layout_button(pLayoutMain, pButtonCancel, 0, 0);
    layout_layout(pLayoutMain, pLayoutDynamicButtons, 1, 0);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    /* Show appropriate button layout */
    for (dRow = 0; dRow < 4; ++dRow)
    {
        layout_show_row(pApp->pLayoutExtra, dRow, dRow == dRowForPage);
    }

    return pPanelMain;
}

Panel*
Panel_GetModalTweaker(AppGUI *pApp)
{
    const GUITweaker* pTweaker;
    Panel       *pPanelMain         = panel_create();
    Panel       *pPanelDescription  = _Panel_TweakerDescription(pApp);
    Panel       *pPanelSelector     = _Panel_TweakerOptionsSelector(pApp);
    Panel       *pPanelImage        = Panel_GetImageDemo(pApp);
    Panel       *pPanelButtons      = _Panel_TweakerButtons(pApp);
    Layout      *pLayoutMain        = layout_create(1,5);

    pTweaker                = &pApp->tGUITweaker[pApp->dPage];
    pApp->pLayoutModalMain  = pLayoutMain;
    assert(IS_VALID(pTweaker));

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
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
}

static void
_Callback_LocalisationPopupSelector(AppGUI *pApp, Event *e)
{
    /* Since there's no way to "tag" popups, just acquire data from all widgets */
    pApp->dLocaleSelected[LOC_CORE] = popup_get_selected(pApp->pPopupCore);
    pApp->dLocaleSelected[LOC_MOD]  = popup_get_selected(pApp->pPopupMod);

    view_update(pApp->pLocaleView);

    unref(e);

    /* gui_language(lang); */
}

static void 
_LocalisationFillPopup(AppGUI *pApp, PopUp *pPop, LocTier eTier)
{
    size_t i;

    popup_clear(pPop);
    for (i = 0; i < pApp->dLocaleCount; ++i)
    {
        const LangInfo *li = &pApp->tLocale[i];
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
    Layout      *pLayoutPopupLabels = layout_create(3,1);
    Layout      *pLayoutPopups      = layout_create(3,1);
    Layout      *pLayoutButtons     = layout_create(4,1);
    Label       *pLabelDescription  = label_create();
    Label       *pLabelLocCore      = label_create();
    Label       *pLabelLocMod       = label_create();
    View        *pViewPreview       = view_create();
    Button      *pButtonAccept      = button_push();
    Button      *pButtonCancel      = button_push();
    PopUp       *pPopUpCore         = popup_create();
    PopUp       *pPopUpMod          = popup_create();

    pApp->pPopupCore    = pPopUpCore;
    pApp->pPopupMod     = pPopUpMod;
    pApp->pLocaleView   = pViewPreview;

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
    button_OnClick(pButtonAccept, listener(pApp, Callback_OnButtonModalLocalisation, AppGUI));

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalLocalisation, AppGUI));

    /* Layout: Popup Labels */
    layout_hsize(pLayoutPopupLabels, 0, 200);
    layout_hsize(pLayoutPopupLabels, 1, 200);
    layout_hsize(pLayoutPopupLabels, 2, 200);
    layout_label(pLayoutPopupLabels, pLabelLocCore, 0, 0);
    layout_label(pLayoutPopupLabels, pLabelLocMod, 1, 0);

    /* Layout: Popups */
    layout_hsize(pLayoutPopups, 0, 200);
    layout_hsize(pLayoutPopups, 1, 200);
    layout_hsize(pLayoutPopups, 2, 200);
    layout_popup(pLayoutPopups, pPopUpCore, 0, 0);
    layout_popup(pLayoutPopups, pPopUpMod, 1, 0);

    /* Layout: Buttons */
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonAccept, 3, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, 640);

    layout_label(pLayoutMain, pLabelDescription, 0, 0);
    layout_layout(pLayoutMain, pLayoutPopupLabels, 0,1);
    layout_layout(pLayoutMain, pLayoutPopups, 0,2);
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
    Panel       *pPanelMain         = panel_scroll(FALSE,TRUE);
    Layout      *pLayoutMain        = layout_create(8,16);

    unsigned int i      = 0;
    unsigned int j      = 0;
    unsigned int dRow   = 0;

    GUIOptElement *pElem;

    cassert_no_null(pApp->pOptElementArray);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, 640);

    for(i = 0; i < MAX_OPT_ELEMS; i ++)
    {
        pElem = &pApp->pOptElementArray[i];
        assert(IS_VALID(pElem));

        switch(pElem->eType)
        {
            case UI_WIDGET_RADIO:
                {
                    Label *pLabel           = label_create();
                    Layout *pLayoutButton   = layout_create(pElem->dNumOfOptions*2, 1);

                    /* Store layout instead of button because there are multiple buttons present there */
                    pElem->pElement = pLayoutButton;

                    label_text(pLabel, tc(pElem->pTitle));

                    for(j = 0; j < pElem->dNumOfOptions; j++)
                    {
                        Label *pLabelOpt    = label_create();
                        Button *pButton     = button_radio();
                        unsigned int pos    = j * 2;

                        label_text(pLabelOpt, tc(pElem->pOptTitle[j]));
                        label_align(pLabelOpt, ekLEFT);

                        button_tag(pButton, (MODAL_OPT_A + i) );
                        if (pElem->dChoice == j)
                        {
                            button_state(pButton, ekGUI_ON);
                        }

                        layout_label(pLayoutButton, pLabelOpt, pos, 0);
                        layout_button(pLayoutButton, pButton, pos + 1, 0);
                        layout_halign(pLayoutButton, pos, 0, ekJUSTIFY);
                        layout_hsize(pLayoutButton, pos, 128);
                        layout_hsize(pLayoutButton, pos+1, 64);
                    }

                    layout_label(pLayoutMain, pLabel, 0, dRow);
                    layout_layout(pLayoutMain, pLayoutButton, 0, dRow+1);

                    dRow += 2;
                }
                break;
            case UI_WIDGET_POPUP:
                {
                    Label *pLabel = label_create();
                    PopUp *pPopup = popup_create();

                    pElem->pElement = pPopup;

                    label_text(pLabel, tc(pElem->pTitle));

                    for(j = 0; j < pElem->dNumOfOptions; j++)
                    {
                        popup_add_elem(pPopup, tc(pElem->pOptTitle[j]), NULL);
                    }

                    popup_selected(pPopup, pElem->dChoice);
                    layout_label(pLayoutMain, pLabel, 0, dRow);
                    layout_popup(pLayoutMain, pPopup, 7, dRow);
                    layout_hsize(pLayoutMain, 0, 256);

                    dRow++;
                }
                break;
            case UI_WIDGET_EDIT:
                {
                    Label *pLabel   = label_create();
                    Edit  *pEdit    = edit_create();

                    pElem->pElement = pEdit;

                    label_text(pLabel, tc(pElem->pTitle));
                    edit_text(pEdit, tc(pElem->pOptTitle[0]));

                    layout_label(pLayoutMain, pLabel, 0, dRow);
                    layout_edit(pLayoutMain, pEdit, 7, dRow);
                    layout_hsize(pLayoutMain, 0, 256);

                    dRow++;
                }
                break;
            case UI_WIDGET_LISTBOX:
                {
                    Label       *pLabel   = label_create();
                    ListBox     *pListbox = listbox_create();

                    pElem->pElement = pListbox;

                    label_text(pLabel, tc(pElem->pTitle));

                    listbox_checkbox(pListbox, TRUE);
                    for(j = 0; j < pElem->dNumOfOptions; j++)
                    {
                        const bool_t bCheckbox = (pElem->dChoice & (1U << j)) != 0;

                        listbox_add_elem(pListbox, tc(pElem->pOptTitle[j]), NULL);
                        listbox_check(pListbox, j, bCheckbox);
                    }

                    layout_label(pLayoutMain, pLabel, 0, dRow);
                    layout_listbox(pLayoutMain, pListbox, 0, dRow+1);
                    layout_hsize(pLayoutMain, 0, 256);

                    dRow += 2;
                }
                break;
            default:
                break;
        }
    }

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;

    unref(pApp);
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

    /* Button: Cancel */
    button_text(pButtonCancel, TXT_BTN_CANCEL);
    button_tag(pButtonCancel, MODAL_CANCEL);
    button_OnClick(pButtonCancel, listener(pApp, Callback_OnButtonModalOptions, AppGUI));

    /* Layout: Buttons */
    layout_button(pLayoutButtons, pButtonCancel, 0, 0);
    layout_button(pLayoutButtons, pButtonAccept, 3, 0);

    /* Layout: Main */
    layout_panel(pLayoutMain, pPanelSubpanel, 0,0);
    layout_layout(pLayoutMain, pLayoutButtons, 0, 1);

    /* Panel: Main */
    panel_layout(pPanelMain, pLayoutMain);

    return pPanelMain;
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

    textview_printf(pApp->pDebugData->pConsoleTextView,
        "> %s\n",sEditText);

    AmberLauncher_ExecuteLua(pApp->pAppCore, sEditText);

    edit_text(pApp->pDebugData->pConsoleInput,"");
    unref(e);
}

void
Callback_OnButtonModalMessage(AppGUI *pApp, Event *e)
{
    Button *pButton = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);
    window_stop_modal(pApp->pWindowModal, dButtonTag);
    unref(pButton);
    unref(e);
}

void
Callback_OnButtonModalQuestion(AppGUI *pApp, Event *e)
{
    Button *pButton = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);
    window_stop_modal(pApp->pWindowModal, dButtonTag);
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
        window_stop_modal(pApp->pWindowModal, dButtonTag);
        return;
    }

    /* Button clicked: Browse */
    if (pApp->pString)
    {
        str_destroy(&pApp->pString);
    }

    pApp->pString = str_c(comwin_open_file(pApp->pWindowModal, sFileFormat, 1, NULL));

    if (pApp->pEdit)
    {
        edit_text(pApp->pEdit, tc(pApp->pString));
    }

    if (pApp->pTextView)
    {
        textview_printf(pApp->pTextView,"Game path: %s\n", tc(pApp->pString));
    }
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
            window_stop_modal(pApp->pWindowModal, dButtonTag);
            return;
        case MODAL_PREVIOUS:
        case MODAL_NEXT:
            {
                const int dStep = (dButtonTag == MODAL_NEXT) ? 1 : -1;
                pApp->dPage     = bmath_clampd(pApp->dPage + dStep, 0, pApp->dPageMax - 1);
                bPageSwitch     = TRUE;
                break;
            }
        default:
            break;
    }

    if (bPageSwitch == TRUE)
    {
        const int dRowForPage    = _RowForPage(pApp->dPage, pApp->dPageMax);
        int dRow;

        pTweaker = &pApp->tGUITweaker[pApp->dPage];
        dImageIndex = pApp->tGUITweaker[pApp->dPage].dSelectedOption;
        layout_panel_replace(pApp->pLayoutModalMain, _Panel_TweakerDescription(pApp), 0, 0);
        layout_panel_replace(pApp->pLayoutModalMain, _Panel_TweakerOptionsSelector(pApp), 0, 1);
        /* layout_panel_replace(pApp->pLayoutModalMain, _Panel_TweakerButtons(pApp), 0, 4); */

        /* Show appropriate button layout */
        for (dRow = 0; dRow < 4; ++dRow)
        {
            layout_show_row(pApp->pLayoutExtra, dRow, dRow == dRowForPage);
        }
    }

    str_destopt(&pApp->pString);
    pApp->pString = str_copy(pTweaker->pStringImagePath[dImageIndex]);
    layout_panel_replace(pApp->pLayoutModalMain, Panel_GetImageDemo(pApp), 0, 2);

    layout_update(pApp->pLayoutModalMain);
}

void
Callback_OnButtonModalLocalisation(AppGUI *pApp, Event *e)
{
    Button *pButton     = event_sender(e, Button);
    uint32_t dButtonTag = button_get_tag(pButton);

    pApp->dLocaleSelected[LOC_CORE] = popup_get_selected(pApp->pPopupCore);
    pApp->dLocaleSelected[LOC_MOD]  = popup_get_selected(pApp->pPopupMod);

    window_stop_modal(pApp->pWindowModal, dButtonTag);

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
                GUIOptElement *pElem = &pApp->pOptElementArray[i];

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
                            str_upd(&pElem->pOutputString, pValue);
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
                                /* str_upd(&pElem->pOutputString, listbox_get_text(pListbox, j)); */
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        window_stop_modal(pApp->pWindowModal, dButtonTag);
        return;
    }
}

void 
Callback_OnButtonPlay(AppGUI *pApp, Event *e)
{
    AmberLauncher_Play(pApp->pAppCore);
}

void
Callback_OnDrawLocalisation(AppGUI *pApp, Event *e)
{
    ferror_t        eError1, eError2;
    Image           *pImageBG = NULL;
    Image           *pImageFG = NULL;
    const EvDraw    *pEvDraw  = event_params(e, EvDraw);
    DCtx            *pCtx = pEvDraw ? pEvDraw->ctx : NULL;

    assert(pApp->tLocale[pApp->dLocaleSelected[LOC_CORE]].bCore);
    assert(pApp->tLocale[pApp->dLocaleSelected[LOC_MOD]].bMod);

    pImageBG = image_from_file(pApp->tLocale[pApp->dLocaleSelected[LOC_CORE]].sCorePath,&eError1);
    pImageFG = image_from_file(pApp->tLocale[pApp->dLocaleSelected[LOC_MOD]].sModPath,&eError2);

    assert(eError1 == ekFOK);
    assert(eError2 == ekFOK);

    draw_clear(pCtx, color_rgb(0, 0, 0));

    if (IS_VALID(pImageBG))
        draw_image(pCtx, pImageBG, 0, 0);

    if (IS_VALID(pImageFG))
        draw_image(pCtx, pImageFG, 0, 0);

    image_destroy(&pImageBG);
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

static Panel*
_Panel_GetRoot(AppGUI *pApp)
{
    Panel       *pPanelMain = panel_create();
    Panel       *pPanelExt  = panel_create();
    Layout      *pLayoutMain= layout_create(1,2);
    Layout      *pLayoutExt = layout_create(1,1);
    ImageView   *pImageView = imageview_create();

    Label *label1 = label_create();
    label_text(label1, TXT_GREETINGS);
    label_multiline(label1, TRUE);

    pApp->pLayoutMain = pLayoutMain;

    /* Global Margin */
    layout_margin(pLayoutMain, 4);
    
    /* Top Image */
    /* layout_hsize(pLayoutMain, 0, TITLE_PNG_W);
    layout_vsize(pLayoutMain, 0, TITLE_PNG_H);
    layout_halign(pLayoutMain, 0, 0, ekJUSTIFY);
    layout_valign(pLayoutMain, 0, 0, ekJUSTIFY);*/
    layout_halign(pLayoutMain, 0, 1, ekJUSTIFY);
    layout_valign(pLayoutMain, 0, 1, ekJUSTIFY); 
    imageview_scale(pImageView, ekGUI_SCALE_AUTO);
    imageview_size(pImageView, s2df(TITLE_PNG_W,TITLE_PNG_H));
    imageview_image(pImageView, (const Image*)TITLE_JPG);
    layout_imageview(pLayoutMain, pImageView, 0, 0);
    
    /* External Panel */
    layout_hsize(pLayoutExt, 0, TITLE_PNG_W);
    layout_halign(pLayoutExt, 0, 0, ekCENTER);
    layout_valign(pLayoutExt, 0, 0, ekTOP);
    layout_label(pLayoutExt, label1, 0, 0);
    panel_layout(pPanelExt, pLayoutExt);

    /* Global Panel */
    layout_panel(pLayoutMain, pPanelExt, 0, 1);
    panel_layout(pPanelMain, pLayoutMain);

    /* Starting panel */
    Panel_Set(pApp, CPANEL_MAIN, FLAG_PANEL_NONE);

    return pPanelMain;
}

static AppGUI*
_Nappgui_Start(void)
{
    AppGUI *pApp;
    Panel *pPanel;

    gui_respack(res_app_respack);
    gui_language("");

    /* Core initialization */
    pApp = heap_new0(AppGUI);
    cassert_no_null(pApp);

    pApp->pAppCore            = AppCore_create();
    AppCore_init(pApp->pAppCore);
    pApp->pAppCore->pOwner    = (void*)pApp;
    pApp->pAppCore->cbUIEvent = _Callback_UIEvent;

    /* AppGUI  */
    pApp->pDebugData          = heap_new(GUIDebugData);
    cassert_no_null(pApp->pDebugData);
    pApp->pOptElementArray    = heap_new_n0(MAX_OPT_ELEMS, GUIOptElement);
    cassert_no_null(pApp->pOptElementArray);

    pPanel           = _Panel_GetRoot(pApp);
    pApp->pWindow    = window_create(ekWINDOW_STDRES | ekWINDOW_CLOSE | ekWINDOW_ESC | ekWINDOW_RETURN);

    window_hotkey(pApp->pWindow, ekKEY_F6, 0, listener(pApp, Callback_OnWindowHotkeyF6, AppGUI));
    window_panel(pApp->pWindow, pPanel);
    window_title(pApp->pWindow, TXT_TITLE_LAUNCHER);
    /* window_origin(pApp->pWindow, v2df(800, 400)); */
    window_OnClose(pApp->pWindow, listener(pApp, _Callback_OnCloseMainWindow, AppGUI));
    window_show(pApp->pWindow);

    AmberLauncher_Start(pApp->pAppCore);
    
    return pApp;
}

static void 
_Nappgui_End(AppGUI **pApp)
{
    size_t i;
    size_t j;

    /* Debug Data */
    str_destopt(&(*pApp)->pDebugData->pInputString);
    heap_delete(&(*pApp)->pDebugData, GUIDebugData);
    (*pApp)->pDebugData = NULL;

    /* APPGUI */
    for(i = 0; i < APP_MAX_ELEMENTS; i++)
    {
        GUITweaker *pTweaker = &(*pApp)->tGUITweaker[i];
        str_destopt(&pTweaker->pStringTag);
        str_destopt(&pTweaker->pStringTweakerInfo);

        for(j = 0; j < APP_MAX_ELEMENTS; j++)
        {
            str_destopt(&pTweaker->pStringImageTitle[j]);
            str_destopt(&pTweaker->pStringImagePath[j]);
        }
    }

    /* Options */
    {
        unsigned int i, j;

        for (i = 0; i < MAX_OPT_ELEMS; ++i)
        {
            GUIOptElement *pElem = &(*pApp)->pOptElementArray[i];

            str_destopt(&pElem->pKeyID);
            str_destopt(&pElem->pOutputString);
            str_destopt(&pElem->pTitle);

            for (j = 0; j < APP_MAX_ELEMENTS; ++j)
                str_destopt(&pElem->pOptTitle[j]);
        }
    }

    heap_delete_n(&(*pApp)->pOptElementArray, MAX_OPT_ELEMS, GUIOptElement);
    (*pApp)->pOptElementArray = NULL;

    str_destopt(&(*pApp)->pString);

    AmberLauncher_End((*pApp)->pAppCore);
    if ((*pApp)->pWindowModal)
    {
        window_destroy(&(*pApp)->pWindowModal);
    }
    window_destroy(&(*pApp)->pWindow);
    AppCore_free(&(*pApp)->pAppCore);
    heap_delete(pApp, AppGUI);
}

static uint32_t
_Nappgui_ShowModal( AppGUI *pApp, Panel *pPanel, const char* sTitle)
{
    uint32_t dRet;

    pApp->pWindowModal  = window_create(ekWINDOW_STD | ekWINDOW_CLOSE | ekWINDOW_ESC | ekWINDOW_RETURN);
    pApp->pPanelModal   = pPanel;
    window_panel (pApp->pWindowModal, pPanel);
    window_title (pApp->pWindowModal, sTitle);
    window_OnClose(pApp->pWindowModal, listener(pApp, _Callback_OnCloseModalWindow, AppGUI));
    dRet = window_modal(pApp->pWindowModal, pApp->pWindow);
    window_destroy(&pApp->pWindowModal);

    return dRet;
}

/******************************************************************************
 * STATIC CALLBACK DEFINITIONS
 ******************************************************************************/

static void 
_Callback_OnCloseMainWindow(AppGUI *pApp, Event *e)
{
    const EvWinClose *p = event_params(e, EvWinClose);
    cassert_no_null(pApp);

    switch (p->origin)
    {
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
        case ekGUI_CLOSE_INTRO:

            if (pApp->eCurrentUIEvent == UIEVENT_DEBUG)
            {
                *pResult = FALSE; /* prevents from closing window */

                Callback_OnButtonModalDebug(pApp, e);
                return;
            }
            break;

        case ekGUI_CLOSE_ESC:
        case ekGUI_CLOSE_BUTTON:
            break;

        default:
            *pResult = FALSE; /* prevents from closing window */
            break;
    }
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
    if (pApp->pTextView && eEventType < UIEVENT_MAX)
    {
        textview_printf(pApp->pTextView,"UI EVENT: %s\n", EUIEventTypeStrings[eEventType]);
    }
    #endif

    if (pApp->pString)
    {
        str_destroy(&pApp->pString);
    }

    pApp->eCurrentUIEvent = eEventType;

    switch(eEventType)
    {
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

                if (pApp->pTextView)
                {
                    textview_printf(pApp->pTextView,"%s\n", sMessage);
                    SVARKEYB_BOOL(tRetVal, sStatusKey, CTRUE);
                    break;
                }

                SVARKEYB_BOOL(tRetVal, sStatusKey, CFALSE);
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

        case UIEVENT_AUTOCONFIG:
            {
                FPanelFlags dFlags = FLAG_PANEL_NONE;

                /* Button state - "Configure" or "Next"? */
                if (IS_VALID(pUserData) && dNumArgs >= 1)
                {
                    bool_t bButtonNext = SVAR_GET_BOOL(pUserData[0]);
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

                pApp->pString = str_c(SVAR_GET_CONSTCHAR(pUserData[0]));

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

                pApp->pString = str_c(SVAR_GET_CONSTCHAR(pUserData[0]));

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
                        if (pApp->pString)
                        {
                            const char* pGamePath = tc(pApp->pString);
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

                textview_printf(pApp->pTextView, "Processing %zu groups\n", pTable->dCount);

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
                    if (pApp->tGUITweaker[i].pStringTweakerInfo)
                    {
                        str_destroy(&pApp->tGUITweaker[i].pStringTweakerInfo);
                        pApp->tGUITweaker[i].pStringTweakerInfo = NULL;
                    }
                    if (pApp->tGUITweaker[i].pStringTag)
                    {
                        str_destroy(&pApp->tGUITweaker[i].pStringTag);
                        pApp->tGUITweaker[i].pStringTag = NULL;
                    }

                    for (k = 0; k < APP_MAX_ELEMENTS; ++k)
                    {
                        if (pApp->tGUITweaker[i].pStringImageTitle[k])
                        {
                            str_destroy(&pApp->tGUITweaker[i].pStringImageTitle[k]);
                            pApp->tGUITweaker[i].pStringImageTitle[k] = NULL;
                        }
                        if (pApp->tGUITweaker[i].pStringImagePath[k])
                        {
                            str_destroy(&pApp->tGUITweaker[i].pStringImagePath[k]);
                            pApp->tGUITweaker[i].pStringImagePath[k] = NULL;
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
                                pApp->tGUITweaker[i].pStringTweakerInfo = str_c(sGroupInfo);
                            }
                            else if (strcmp(sKey, "tag") == 0)
                            {
                                const char *sTagName = SVAR_GET_CONSTCHAR(pGroupTable->pEntries[k].tValue);
                                pApp->tGUITweaker[i].pStringTag = str_c(sTagName);
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
                                pApp->tGUITweaker[i].pStringImageTitle[dOptionIndex] = str_c(sOptionName);
                            }
                            if (sImagePath)
                            {
                                pApp->tGUITweaker[i].pStringImagePath[dOptionIndex] = str_c(sImagePath);
                            }

                            textview_printf(pApp->pTextView, "Group %zu - Option %zu: %s, Asset: %s\n",
                                        i + 1, dOptionIndex + 1,
                                        sOptionName ? sOptionName : "Unknown",
                                        sImagePath ? sImagePath : "No image");

                            dOptionIndex++;
                        }
                    }

                    pApp->tGUITweaker[i].dMaxOptions = dOptionIndex;

                    textview_printf(pApp->pTextView, "Group %zu Info: %s (%zu options)\n",
                                i + 1,
                                sGroupInfo ? sGroupInfo : "No description",
                                dOptionIndex);

                    dGroupCount++;
                }

                /* Set the current page to first group */
                pApp->dPage     = 0;
                pApp->dPageMax  = dGroupCount;
                for(i = 0; i < APP_MAX_ELEMENTS; i++)
                {
                    pApp->tGUITweaker[i].dSelectedOption    = 0;
                }

                /* Set initial image to first option of first group if available */
                if (dGroupCount > 0 && pApp->tGUITweaker[0].pStringImagePath[0])
                {
                    str_upd(&pApp->pString, tc(pApp->tGUITweaker[0].pStringImagePath[0]));
                }

                textview_printf(pApp->pTextView, "Total groups processed: %zu\n", dGroupCount);

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
                            unsigned int i;
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
                                const bool_t bIsEmpty = !str_empty(pApp->tGUITweaker[i].pStringTag);
                                SVARKEYB_INT(tRetVal, bIsEmpty ? tc(pApp->tGUITweaker[i].pStringTag) : sOptName[i], pApp->tGUITweaker[i].dSelectedOption);
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
                pApp->dLocaleCount = 0;
                memset(pApp->dLocaleSelected, 0, sizeof(pApp->dLocaleSelected));

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
                        LangInfo *dst   = &pApp->tLocale[pApp->dLocaleCount++];
                        dst->sCode      = sCode;
                        dst->sCorePath  = sCorePath;
                        dst->sModPath   = sModPath;
                        dst->bCore      = !str_empty_c(sCorePath);
                        dst->bMod       = !str_empty_c(sModPath);

                        textview_printf(pApp->pTextView,
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
                        uint32_t dCore  = pApp->dLocaleSelected[LOC_CORE];
                        uint32_t dMod   = pApp->dLocaleSelected[LOC_MOD];
                        SVARKEYB_BOOL(tRetVal,sStatusKey, CTRUE);
                        SVARKEYB_CONSTCHAR(tRetVal,"core",  pApp->tLocale[dCore].sCode);
                        SVARKEYB_CONSTCHAR(tRetVal,"mod",   pApp->tLocale[dMod].sCode);
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
                int            dModalRetVal;

                /* pre-checks + init */
                cassert_no_null(pUserData);
                assert(dNumArgs == 1);
                assert(pUserData[0].eType == CTYPE_LUATABLE);

                pRootTable  = (SVarTable *)SVAR_GET_LUATABLE(pUserData[0]);
                dElemCount  = 0U;
                memset(pApp->pOptElementArray, 0, sizeof(GUIOptElement) * MAX_OPT_ELEMS);

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
                        pDst                = &pApp->pOptElementArray[dElemCount];

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
                                pDst->pTitle =
                                    str_c(SVAR_GET_CONSTCHAR(pElem->pEntries[iField].tValue));
                            }
                            else if (strcmp(sValKey, sKeyID) == 0)
                            {
                                pDst->pKeyID =
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
                                        pDst->pOptTitle[dOptIndex++] =
                                            str_c(SVAR_GET_CONSTCHAR(
                                                    pElem->pEntries[iField].tValue));
                                        break;

                                    case CTYPE_DOUBLE:
                                        pDst->pOptTitle[dOptIndex++] =
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
                                                pDst->pOptTitle[dOptIndex++] =
                                                    str_c(SVAR_GET_CONSTCHAR(
                                                            pOpts->pEntries[q].tValue));
                                            }
                                        }
                                    }
                                    break;

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
                                    pDst->pOptTitle[0] =
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

                        textview_printf(pApp->pTextView,
                                        "Elem %zu: title='%s' type=%d opts=%u\n",
                                        dElemCount,
                                        (pDst->pTitle ? tc(pDst->pTitle) : "(null)"),
                                        (int)pDst->eType,
                                        pDst->dNumOfOptions);

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
                            const GUIOptElement *pElem   = &pApp->pOptElementArray[u];
                            const char          *sOutKey =
                                (pElem->pKeyID != NULL) ? tc(pElem->pKeyID) : tc(pElem->pTitle);

                            switch (pElem->eType)
                            {
                                case UI_WIDGET_EDIT:
                                {
                                    const char *sVal =
                                        (pElem->pOutputString != NULL) ? tc(pElem->pOutputString) :
                                        (pElem->pOptTitle[0]  != NULL) ? tc(pElem->pOptTitle[0]) : "";
                                    SVARKEYB_CONSTCHAR(tRetVal, sOutKey, sVal);
                                }
                                break;

                                case UI_WIDGET_LISTBOX:
                                case UI_WIDGET_POPUP:
                                case UI_WIDGET_RADIO:
                                    SVARKEYB_INT(tRetVal, sOutKey, (int)pElem->dChoice);
                                    break;

                                default:
                                    break;
                            }
                        }
                    }
                    break;
                }
            }
            break;

        default:
            SVARKEYB_NULL(tRetVal, sStatusKey);

            if (pApp->pTextView)
            {
                textview_printf(
                    pApp->pTextView,
                    "Behavior for %s (%d) is not defined!\n",
                    eEventType < UIEVENT_MAX ? EUIEventTypeStrings[eEventType] : "?", eEventType);
            }
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
