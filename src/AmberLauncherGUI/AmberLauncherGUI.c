#include <AmberLauncherGUI.h>
#include <AmberLauncherCore.h>

#include <core/appcore.h>

#include <nappgui.h>
#include "res_app.h"

#include <assert.h>

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

const char* EUserEventTypeStrings[] = {
    "NULL",
    "MODAL_GAMENOTFOUND",
    NULL
};

static const int32_t TITLE_PNG_W = 672;
static const int32_t TITLE_PNG_H = 200;
/* static const uint32_t MODAL_CANCEL = 10; */
static const uint32_t MODAL_ACCEPT = 11;

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
 * @relatedalso Core
 * @brief       Initializes NappGUI instance
 * 
 * @return      App* 
 */
static AppGUI*
_Nappgui_Start(void);

/**
 * @relatedalso Core
 * @brief       Terminates and cleans up NappGUI instance
 * 
 * @param       pApp 
 */
static void 
_Nappgui_End(AppGUI **pApp);

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
_Callback_OnClose(AppGUI* pApp, Event* e);

/**
 * @relatedalso GUI
 * @brief       Related to various UI events called from (mostly) Lua
 *
 * @param       pApp
 * @param       dID         EUserEventType
 * @param       pUserData
 */
static SVar
_Callback_UIEvent(AppCore *pAppCore, uint32 dID, SVar *pUserData);

/******************************************************************************
 * HEADER DEFINITIONS
 ******************************************************************************/

Panel *
Panel_Set(AppGUI *pApp, const EPanelType eType)
{
    Panel *pPanel = NULL;
    Button *defbutton = NULL;
    switch ((uint32_t)eType)
    {
    case CPANEL_NULL:
        pPanel = Panel_GetNull(pApp);
        break;

    case CPANEL_AUTOCONFIG:
        pPanel = Panel_GetAutoConfigure(pApp);
        break;

    case CPANEL_MAIN:
        pPanel = Panel_GetMain(pApp);
        break;

    default:
        pPanel = Panel_GetNull(pApp);
        break;
    }

    layout_panel_replace(pApp->pLayout, pPanel, 0, 1);

    if (pApp->pWindow != NULL)
    {
        window_defbutton(pApp->pWindow, defbutton);
    }

    return NULL;
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
Panel_GetAutoConfigure(AppGUI* pApp)
{
    Panel       *pPanelMain  = panel_create();
    Layout      *pLayoutMain = layout_create(1,3);
    Layout      *pLayoutTxt  = layout_create(1,1);
    Label       *pLabelGreet = label_create();
    Button      *pButtonCfg  = button_push();
    TextView    *pConsole    = textview_create();
    Font        *pFontGreet  = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFontCfg    = font_system(24, ekFNORMAL | ekFPIXELS);

    pApp->pTextView = pConsole;

    /* Label: Greetings Text */
    label_text(pLabelGreet, TXT_GREETINGS);
    label_font(pLabelGreet, pFontGreet);

    /* Button: Configure */
    button_text(pButtonCfg, TXT_BTN_CONFIGURE);
    button_font(pButtonCfg, pFontCfg);
    button_vpadding (pButtonCfg, 16);
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
Panel_GetModalGameNotFound(AppGUI* pApp)
{
    Panel       *pPanelMain     = panel_create();
    Layout      *pLayoutMain    = layout_create(1,3);
    Layout      *pLayoutPath    = layout_create(2,1);
    Layout      *pLayoutTxt     = layout_create(1,1);
    Edit        *pEditPath      = edit_create();
    Label       *pLabelInfo     = label_create();
    Button      *pButtonSubmit  = button_push();
    Button      *pButtonBrowse  = button_push();
    Font        *pFontInfo      = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFontSubmit    = font_system(24, ekFNORMAL | ekFPIXELS);

    /* Label: Greetings Text */
    label_text(pLabelInfo, "Launcher couldn\'t locate the Might and Magic 7 installation. Please select the game\'s folder.");
    label_font(pLabelInfo, pFontInfo);

    /* Edit: Game Path */
    pApp->pEdit = pEditPath;

    /* Button: Browse (file) */
    button_text(pButtonBrowse, TXT_BTN_BROWSE);
    button_font(pButtonBrowse, pFontInfo);
    button_OnClick(pButtonBrowse, listener(pApp, Callback_OnButtonBrowseFile, AppGUI));

    /* Button: Submit */
    button_text(pButtonSubmit, TXT_BTN_SUBMIT);
    button_font(pButtonSubmit, pFontSubmit);
    button_vpadding (pButtonSubmit, 16);
    button_OnClick(pButtonSubmit, listener(pApp, Callback_OnButtonSubmit, AppGUI));

    /* Layout: Text */
    layout_margin(pLayoutTxt, 4);
    layout_label(pLayoutTxt, pLabelInfo, 0, 0);

    /* Layout: Game Path */
    layout_edit(pLayoutPath, pEditPath, 0,0);
    layout_button(pLayoutPath, pButtonBrowse, 1, 0);

    /* Layout: Main */
    layout_hsize(pLayoutMain, 0, 250);
    layout_layout(pLayoutMain, pLayoutTxt, 0, 0);
    layout_layout(pLayoutMain, pLayoutPath, 0, 1);
    layout_button(pLayoutMain,pButtonSubmit,0, 2);

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

void 
Callback_OnButtonConfigure(AppGUI *pApp, Event *e)
{
    AmberLauncher_ConfigureStart(pApp->pAppCore);
    unref(e);
}

void
Callback_OnButtonSubmit(AppGUI *pApp, Event *e)
{
    window_stop_modal(pApp->pWindowModal, MODAL_ACCEPT);
    unref(e);
}

void
Callback_OnButtonBrowseFile(AppGUI *pApp, Event *e)
{
    const char *sFileFormat[] = {
        "exe",
    };

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
    unref(e);
}

void 
Callback_OnButtonPlay(AppGUI *pApp, Event *e)
{
    AmberLauncher_Play(pApp->pAppCore);
    /* AmberLauncher_ProcessLaunch("mm7.exe", _argc, _argv, TRUE); */
    unref(e);
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
    label_text(label1, "Hey");
    label_multiline(label1, TRUE);

    pApp->pLayout           = pLayoutMain;

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
    Panel_Set(pApp, CPANEL_AUTOCONFIG);

    return pPanelMain;
}

static AppGUI*
_Nappgui_Start(void)
{
    AppGUI *pApp;
    Panel *pPanel;

    gui_respack(res_app_respack);
    gui_language("");

    /* GUI Specific Application variables */
    pApp             = heap_new0(AppGUI);
    pApp->pAppCore   = AppCore_create();
    pApp->pWindow    = NULL;
    pApp->pTextView  = NULL;
    pApp->pImageView = NULL;
    pApp->pLayout    = NULL;
    pApp->pString    = NULL;

    /* Core Library Application variables */
    AppCore_init(pApp->pAppCore);
    pApp->pAppCore->pOwner      = (void*)pApp;
    pApp->pAppCore->cbUIEvent   = _Callback_UIEvent;

    pPanel           = _Panel_GetRoot(pApp);
    pApp->pWindow    = window_create(ekWINDOW_STDRES | ekWINDOW_ESC | ekWINDOW_RETURN);

    window_panel(pApp->pWindow, pPanel);
    window_title(pApp->pWindow, TXT_TITLE_LAUNCHER);
    /* window_origin(pApp->pWindow, v2df(800, 400)); */
    window_OnClose(pApp->pWindow, listener(pApp, _Callback_OnClose, AppGUI));
    window_show(pApp->pWindow);

    AmberLauncher_Start(pApp->pAppCore);
    
    return pApp;
}

static void 
_Nappgui_End(AppGUI **pApp)
{
    if ((*pApp)->pString)
    {
        str_destroy(&(*pApp)->pString);
    }

    AmberLauncher_End((*pApp)->pAppCore);
    window_destroy(&(*pApp)->pWindow);
    AppCore_free(&(*pApp)->pAppCore);
    heap_delete(pApp, AppGUI);
}

/******************************************************************************
 * STATIC CALLBACK DEFINITIONS
 ******************************************************************************/

static void 
_Callback_OnClose(AppGUI *pApp, Event *e)
{
    osapp_finish();
    unref(pApp);
    unref(e);
}

static SVar
_Callback_UIEvent(AppCore *pAppCore, uint32 dID, SVar *pUserData)
{
    AppGUI *pApp;
    SVar tRetVal;

    SVAR_NULL(tRetVal);

    assert(IS_VALID(pAppCore));
    pApp = (AppGUI*)pAppCore->pOwner;

    if (pApp->pTextView && dID < USEREVENT_MAX)
    {
        textview_printf(pApp->pTextView,"USEREVENT: %s\n", EUserEventTypeStrings[dID]);
    }

    switch(dID)
    {
        case USEREVENT_MODAL_GAMENOTFOUND:
            {
                /** @todo wip. Maybe this whole _Callback_UserEvent should we completely removed */
                Panel  *pPanelModal;

                pApp->pWindowModal  = window_create(ekWINDOW_STD | ekWINDOW_ESC);
                pPanelModal         = Panel_GetModalGameNotFound(pApp);

                window_panel(pApp->pWindowModal, pPanelModal);
                window_title(pApp->pWindowModal, TXT_TITLE_GAMENOTFOUND);
                window_modal(pApp->pWindowModal, pApp->pWindow);
                window_destroy(&pApp->pWindowModal);

                /* Return value (file path) */
                if (pApp->pString)
                {
                    const char* pGamePath = tc(pApp->pString);
                    SVAR_CONSTCHAR(tRetVal, pGamePath);
                }
            }
            break;

        default:
            if (pApp->pTextView)
            {
                textview_printf(
                    pApp->pTextView,
                    "Behavior for %s (%d) is not defined!\n",
                    dID < USEREVENT_MAX ? EUserEventTypeStrings[dID] : "?", dID);
            }
            break;
    }

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
