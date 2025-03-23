#include "core/common.h"
#include "draw2d/draw2d.hxx"
#include "gui/gui.hxx"
#include "sewer/types.hxx"
#include <AmberLauncherGUI.h>
#include <AmberLauncherCore.h>

/******************************************************************************
 * STATIC DECLARATIONS
 ******************************************************************************/

static const int32_t TITLE_PNG_W = 672;
static const int32_t TITLE_PNG_H = 200;

/**
 * @relatedalso Panel
 * @brief       Returns root panel of the launcher
 * 
 * @param       pApp 
 * @return      Panel* 
 */
static Panel*
_Panel_GetRoot(App *pApp);

/**
 * @relatedalso Core
 * @brief       Initializes NappGUI instance
 * 
 * @return      App* 
 */
static App*
_Nappgui_Start(void);

/**
 * @relatedalso Core
 * @brief       Terminates and cleans up NappGUI instance
 * 
 * @param       pApp 
 */
static void 
_Nappgui_End(App **pApp);

/******************************************************************************
 * STATIC CALLBACK DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso Core
 * @brief       Invokes cleanup on manual closing of the main window
 * 
 * @param       pApp 
 * @param       e 
 */
static void
_Callback_OnClose(App* pApp, Event* e);

/******************************************************************************
 * HEADER DEFINITIONS
 ******************************************************************************/

Panel *
Panel_Set(App *pApp, const EPanelType eType)
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
Panel_GetNull(App* pApp)
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
Panel_GetAutoConfigure(App* pApp)
{
    Panel       *pPanelMain  = panel_create();
    Layout      *pLayoutMain = layout_create(1,3);
    Layout      *pLayoutTxt  = layout_create(1,1);
    Label       *pLabelGreet = label_create();
    Button      *pButtonCfg  = button_push();
    TextView    *pConsole    = textview_create();
    Font        *pFontGreet  = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFontCfg    = font_system(24, ekFNORMAL | ekFPIXELS);

    pApp->pText = pConsole;

    /* Label: Greetings Text */
    label_text(pLabelGreet, TXT_GREETINGS);
    label_font(pLabelGreet, pFontGreet);

    /* Button: Configure */
    button_text(pButtonCfg, TXT_BTN_CONFIGURE);
    button_font(pButtonCfg, pFontCfg);
    button_vpadding (pButtonCfg, 16);
    button_OnClick(pButtonCfg, listener(pApp, Callback_OnButtonConfigure, App));
    
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
Panel_GetMain(App* pApp)
{
    Panel   *pPanelMain  = panel_create();
    Layout  *pLayoutMain = layout_create(1, 2);
    Layout  *pLayoutTxt  = layout_create(1,1);
    Label   *pLabelTemp  = label_create();
    Button  *pButtonPlay = button_push();
    Font    *pFontPlay   = font_system(24, ekFNORMAL | ekFPIXELS);

    pApp->pText = NULL;

    /* Label: Temp */
    label_text(pLabelTemp, TXT_GREETINGS);
    label_multiline(pLabelTemp, TRUE);

    /* Button: Play */
    button_text(pButtonPlay, TXT_BTN_PLAY);
    button_font(pButtonPlay, pFontPlay);
    button_vpadding (pButtonPlay, 16);
    button_OnClick(pButtonPlay, listener(pApp, Callback_OnButtonPlay, App));
    
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

void 
Callback_OnButtonConfigure(App *pApp, Event *e)
{
    AmberLauncher_ConfigureStart(pApp->pAppCore);
    unref(e);
}

void 
Callback_OnButtonPlay(App *pApp, Event *e)
{
    AmberLauncher_Play(pApp->pAppCore);
    /* AmberLauncher_ProcessLaunch("mm7.exe", _argc, _argv, TRUE); */
    unref(e);
}

/******************************************************************************
 * STATIC DEFINITIONS
 ******************************************************************************/

static Panel*
_Panel_GetRoot(App *pApp)
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

static App*
_Nappgui_Start(void)
{
    App *pApp;
    Panel *pPanel;

    gui_respack(res_app_respack);
    gui_language("");

    pApp = heap_new0(App);
    pApp->pLayout       = NULL;
    pApp->pText         = NULL;
    pApp->pWindow       = NULL;
    
    pPanel              = _Panel_GetRoot(pApp);
    pApp->pWindow       = window_create(ekWINDOW_STDRES | ekWINDOW_ESC | ekWINDOW_RETURN);
    window_panel(pApp->pWindow, pPanel);
    window_title(pApp->pWindow, "Amber Island Launcher");
    /* window_origin(pApp->pWindow, v2df(800, 400)); */
    window_OnClose(pApp->pWindow, listener(pApp, _Callback_OnClose, App));
    window_show(pApp->pWindow);

    AmberLauncher_Start();
    
    return pApp;
}

static void 
_Nappgui_End(App **pApp)
{
    AmberLauncher_End();
    window_destroy(&(*pApp)->pWindow);
    heap_delete(pApp, App);
}

/******************************************************************************
 * STATIC CALLBACK DEFINITIONS
 ******************************************************************************/

static void 
_Callback_OnClose(App *pApp, Event *e)
{
    osapp_finish();
    unref(pApp);
    unref(e);
}

/******************************************************************************
 * MAIN FUNCTION
 ******************************************************************************/

 #include <osapp/osmain.h>
osmain(_Nappgui_Start, _Nappgui_End, "", App)

/* #include <osmain.h> */
/* osmain(_Nappgui_Start, _Nappgui_End, "", App) */

/* int main(int argc, char *argv[])
{
    _argc = argc;
    _argv = argv;
    FUNC_CHECK_APP_CREATE(_Nappgui_Start, App);
    FUNC_CHECK_DESTROY(_Nappgui_End, App);
    osmain_imp(
        (uint32_t)argc, (char_t **)argv, NULL, 0.,
        (FPtr_app_create)_Nappgui_Start,
        (FPtr_app_update)NULL,
        (FPtr_destroy)_Nappgui_End,
        (char_t *)"");
        
    return 0;
}
 */
