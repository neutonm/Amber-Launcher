/* NAppGUI Hello World */

#include <AmberLauncherGUI.h>
#include <AmberLauncherCore.h>

/******************************************************************************
 * STATIC DECLARATIONS
 ******************************************************************************/

static const char_t *_SLABEL = 
    "Greetings adventurer, before you can play Amber Island, you have to configure the game. "
    "Make sure you have Might and Magic 7 installed on your PC.";

static const int32_t TITLE_PNG_W = 672;
static const int32_t TITLE_PNG_H = 200;

/* static int _argc    = 0;
static char** _argv = NULL; */

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
        pPanel = Panel_GetConfigure(pApp);
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
    Panel   *pPanel      = panel_create();
    Layout  *pLayout     = layout_create(1, 1);
    
    panel_layout(pPanel, pLayout);
    unref(pApp);

    return pPanel;
}

Panel* 
Panel_GetConfigure(App* pApp)
{
    Panel       *pPanel1     = panel_create();
    Panel       *pPanel2     = panel_create();
    Layout      *pLayout1    = layout_create(1, 3);
    Layout      *pLayout2    = layout_create(1, 1);
    Label       *pLabel      = label_create();
    Button      *pButton     = button_push();
    TextView    *pText       = textview_create();
    Font        *pFont       = font_system(18, ekFNORMAL | ekFPIXELS);
    Font        *pFont2      = font_system(24, ekFNORMAL | ekFPIXELS);

    /* Label */
    label_text(pLabel, _SLABEL);
    label_font(pLabel, pFont);

    /* Button */
    button_text(pButton, "Configure");
    button_font(pButton, pFont2);
    button_vpadding (pButton, 16);
    button_OnClick(pButton, listener(pApp, Callback_OnButtonConfigure, App));
    
    /* Layout Settings */
    layout_hsize(pLayout2, 0, TITLE_PNG_W);
    layout_vsize(pLayout2, 0, TITLE_PNG_H);
    layout_margin(pLayout2, 8);
    layout_halign(pLayout2, 0, 0, ekJUSTIFY);

    /* Layout placement */
    layout_label(pLayout2, pLabel, 0, 0);
    panel_layout(pPanel2, pLayout2);

    layout_layout(pLayout1, pLayout2, 0, 0);
    layout_panel(pLayout1, pPanel2, 0, 0);
    layout_textview(pLayout1, pText, 0, 1);
    layout_button(pLayout1, pButton, 0, 2);
    panel_layout(pPanel1, pLayout1);

    pApp->pText = pText;

    font_destroy(&pFont);
    font_destroy(&pFont2);

    return pPanel1;
}

Panel* 
Panel_GetMain(App* pApp)
{
    Panel   *pPanel1     = panel_create();
    Layout  *pLayout1    = layout_create(1, 2);
    Label   *pLabel      = label_create();
    Button  *pButton     = button_push();
    Font    *pFont       = font_system(18, ekFNORMAL | ekFPIXELS);
    Font    *pFont2      = font_system(24, ekFNORMAL | ekFPIXELS);

    pApp->pText = NULL;

    /* Label */
    label_text(pLabel, _SLABEL);

    /* Button */
    button_text(pButton, "Play");
    button_font(pButton, pFont2);
    button_vpadding (pButton, 16);
    button_OnClick(pButton, listener(pApp, Callback_OnButtonPlay, App));
    
    /* Layout Settings */
    layout_halign(pLayout1, 0, 0, ekJUSTIFY);
    
    /* Layout placement */
    layout_label(pLayout1, pLabel, 0, 0);
    layout_button(pLayout1, pButton, 0, 1);
    panel_layout(pPanel1, pLayout1);

    font_destroy(&pFont);
    font_destroy(&pFont2);
    unref(pApp);

    return pPanel1;
}

void 
Callback_OnButtonConfigure(App *pApp, Event *e)
{
    AmberLauncher_ConfigureStart(pApp);
    textview_printf(pApp->pText, "Button configure\n");
    Panel_Set(pApp, CPANEL_MAIN);
    unref(e);
}

void 
Callback_OnButtonPlay(App *pApp, Event *e)
{
    if (pApp->pText)
    {
        textview_printf(pApp->pText, "Button play\n");
    }
    AmberLauncher_Play(pApp);
    /* AmberLauncher_ProcessLaunch("mm7.exe", _argc, _argv, TRUE); */
    unref(e);
}

/******************************************************************************
 * STATIC DEFINITIONS
 ******************************************************************************/

static Panel*
_Panel_GetRoot(App *pApp)
{
    Panel       *pPanel1    = panel_create();
    Panel       *pPanel2    = panel_create();
    Layout      *pLayout1   = layout_create(1, 2);
    Layout      *pLayout2   = layout_create(1, 2);
    Layout      *pLayout3   = layout_create(1, 1);
    
    ImageView   *pImageView = imageview_create();

    pApp->pImageView        = pImageView;
    pApp->pLayout           = pLayout1;
    
    /* Panel2 */
    layout_hsize(pLayout2, 0, TITLE_PNG_W);
    layout_vsize(pLayout2, 0, TITLE_PNG_H);
    /* layout_vmargin(pLayout2, 0, 4); */
    /* imageview_scale(pImageView, ekGUI_SCALE_AUTO); */
    imageview_size(pImageView, s2df(TITLE_PNG_W,TITLE_PNG_H));
    imageview_image(pImageView, (const Image*)TITLE_JPG);
    layout_imageview(pLayout2, pImageView, 0, 0);
    panel_layout(pPanel2, pLayout3);
    
    /* Panel1 */
    layout_halign(pLayout1, 0, 0, ekTOP);
    layout_layout(pLayout1, pLayout2, 0, 0);
    layout_panel(pLayout1, pPanel2, 0, 1);
    layout_margin(pLayout1, 8);
    layout_halign(pLayout1, 0, 1, ekJUSTIFY);
    panel_layout(pPanel1, pLayout1);

    /* Default pPanel */
    Panel_Set(pApp, CPANEL_AUTOCONFIG);

    return pPanel1;
}

static App*
_Nappgui_Start(void)
{
    App *pApp;
    Panel *pPanel;

    gui_respack(res_app_respack);
    gui_language("");

    pApp = heap_new0(App);
    pApp->pImageView    = NULL;
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