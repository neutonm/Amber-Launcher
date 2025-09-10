#ifndef __AMBER_LAUNCHER_GUI_H
#define __AMBER_LAUNCHER_GUI_H

#include "core/appcore.h"
#include "core/common.h"
#include "gui/gui.hxx"
#include <bits/stdint-intn.h>
#include <core/core.hxx>
#include <nappgui.h>

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

struct AppCore;
struct SObserver;

/******************************************************************************
 * MACROS
 ******************************************************************************/

#define APP_MAX_ELEMENTS    8
#define APP_MAX_LOCALES     8
#define MAX_OPT_ELEMS       32
#define MAX_TOOL_ELEMS      16
#define MAX_MOD_ELEMS       32

/******************************************************************************
 * ENUMS
 ******************************************************************************/
typedef enum
{
    UI_WIDGET_NULL,
    UI_WIDGET_RADIO,
    UI_WIDGET_POPUP,
    UI_WIDGET_EDIT,
    UI_WIDGET_LISTBOX,
    UI_WIDGET_MAX
} EUIWidgetType;

typedef enum
{
    CPANEL_NULL,
    CPANEL_AUTOCONFIG,
    CPANEL_MAIN,
    CPANEL_IMAGEDEMO,
    CPANEL_MODAL,
    CPANEL_MAX
} EPanelType;

typedef enum {
    CSIDEBUTTON_NULL        = 0,
    CSIDEBUTTON_MAIN,
    CSIDEBUTTON_SETTINGS,
    CSIDEBUTTON_MODS,
    CSIDEBUTTON_TOOLS,
    CSIDEBUTTON_UPDATE,
    CSIDEBUTTON_WEB_HOMEPAGE,
    CSIDEBUTTON_WEB_DISCORD,
    CBUTTON_MAX
} ESidebarButton;

typedef uint32_t FPanelFlags;
enum
{
    FLAG_PANEL_NONE         = 0U,
    FLAG_PANEL_STATE_END    = 1U << 0
};

typedef enum
{
    UIEVENT_NULL = 0,
    UIEVENT_DEBUG,
    UIEVENT_PRINT,
    UIEVENT_MAIN,
    UIEVENT_PLAY,
    UIEVENT_EXITAPP,
    UIEVENT_AUTOCONFIG,
    UIEVENT_MODAL_CLOSE,
    UIEVENT_MODAL_MESSAGE,
    UIEVENT_MODAL_QUESTION,
    UIEVENT_MODAL_GAMENOTFOUND,
    UIEVENT_MODAL_TWEAKER,
    UIEVENT_MODAL_LOCALISATION,
    UIEVENT_MODAL_OPTIONS,
    UIEVENT_MODAL_MODS,
    UIEVENT_MODAL_TOOLS,
    UIEVENT_MODAL_UPDATER,
    UIEVENT_MAX
} EUIEventType;

typedef enum { LOC_CORE, LOC_MOD } LocTier;

extern const char* EUserEventTypeStrings[];

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct GUIDebugData
{
    String          *pInputString;
    TextView        *pConsoleTextView;
    Edit            *pConsoleInput;
} GUIDebugData;

typedef struct GUIOptElement
{
    void            *pElement;
    String          *pKeyID;
    String          *pOutputString;
    String          *pTitle;
    String          *pOptTitle[APP_MAX_ELEMENTS];
    EUIWidgetType   eType;
    unsigned int    dNumOfOptions;
    unsigned int    dChoice;
} GUIOptElement;

typedef struct
{
    uint32_t dID;
    SVar    tLuaRef;
    String  *pIcon;
    String  *pIconDark;
    String  *pTitle;
    String  *pDescription;
    AppCore *pAppCore;
} GUIToolElement;

typedef struct
{
    String  *pID;
    String  *pName;
    String  *pAuthor;
    String  *pDescription;
    String  *pVersion;
    String  *pWebsite;
    String  *pRoot;
    String  *pGame;
    String  *pScreenshot;
    bool_t   bActive;
    const SVar *pOnInstall;
    const SVar *pOnUninstall;
    SVarTable *pOptions;
} GUIModElement;

typedef struct
{
    String          *pStringTweakerInfo;
    String          *pStringTag;
    String          *pStringImageTitle[APP_MAX_ELEMENTS];
    String          *pStringImagePath[APP_MAX_ELEMENTS];
    int             dSelectedOption;
    int             dMaxOptions;
} GUITweaker;

typedef struct
{
    const char_t *sCode;
    const char_t *sDisplay;

    const char_t *sCorePath;
    const char_t *sModPath;

    bool_t bCore;
    bool_t bMod;
} LangInfo;

typedef struct
{
    struct AppCore  *pAppCore;
    GUIDebugData    *pDebugData;

    Window          *pWindow;
    Window          *pWindowModal;
    Panel           *pPanelMain;
    Panel           *pPanelModal;

    /* GUI elements used either in main or modal window */
    Edit            *pEdit;
    TextView        *pTextView;
    ImageView       *pImageView;
    ListBox         *pListBox;
    Layout          *pLayoutMain;
    Layout          *pLayoutWindow;
    Layout          *pLayoutModalMain;
    Layout          *pLayoutExtra;
    String          *pString;
    Progress        *pProgressbar;

    GUITweaker      tGUITweaker[APP_MAX_ELEMENTS];
    unsigned int    dPage;
    unsigned int    dPageMax;

    GUIOptElement   *pOptElementArray;
    GUIToolElement  *pToolElementArray;
    GUIModElement   *pModElementArray;

    LangInfo        tLocale[APP_MAX_LOCALES];
    unsigned int    dLocaleCount;
    unsigned int    dLocaleSelected[2];
    PopUp           *pPopupCore;
    PopUp           *pPopupMod;
    View            *pLocaleView;

    EPanelType      eCurrentPanel;
    EUIEventType    eCurrentUIEvent;
} AppGUI;

typedef struct GUIAsyncTaskData
{
    AppGUI      *pApp;
    EPanelType  ePanelType;
    FPanelFlags dPanelFlags;
} GUIAsyncTaskData;

/* Json names must coincide with names of C struct */
typedef struct _InetUpdaterLauncherData
{
    int32_t version;
    int32_t build;
} InetUpdaterLauncherData;

typedef struct _InetUpdaterFile
{
    String *path;
    String *sha256;
    uint32_t size;
} InetUpdaterFile;

typedef struct _InetUpdaterJSONData
{
    String *schema;
    String *generated;
    InetUpdaterLauncherData *launcher;
    ArrSt(InetUpdaterFile) *files;
} InetUpdaterJSONData;

DeclSt(InetUpdaterFile);

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

__EXTERN_C

/**
 * @relatedalso Panel
 * @brief       Sets subpanel for core panel
 * 
 * @param       pApp 
 * @param       eType 
 * @return      Panel* 
 */
extern Panel*
Panel_Set(AppGUI* pApp, const EPanelType eType, FPanelFlags dFlags);

/*---------------------------------------------------------------------------*/

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel with zero elements (for testing)
 * 
 * @param       pApp 
 * @return      Panel* 
 */
extern Panel* 
Panel_GetNull(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel responsible for game auto configuration
 * 
 * @param       pApp
 * @param       dFlags (FPanelFlags)
 * @return      Panel*
 */
extern Panel* 
Panel_GetAutoConfigure(AppGUI* pApp, FPanelFlags dFlags);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel responsible for core launcher options
 * 
 * @param       pApp 
 * @return      Panel* 
 */
extern Panel*
Panel_GetMain(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel with single image view
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetImageDemo(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel for Modal Window related to Debug Console (Lua)
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalDebug(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel for Modal Window related to Message UI event
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalMessage(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel for Modal Window related to Yes/No style confirmation UI event.
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalQuestion(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel for Modal Window related to GameNotFound UI event
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalGameNotFound(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel for Modal Window related to Feature/Config tweaker
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalTweaker(AppGUI *pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel for Modal Window related to game/mod Localisation tweaks 
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalLocalisation(AppGUI *pApp);

/**
 * @relatedalso Panel
 * @brief       Shows options window (configure general, mods and tweaks in one place)
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalOptions(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Shows mod manager window (game related)
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalModManager(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Shows tools window (custom scripts, external apps, etc)
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalTools(AppGUI* pApp);

/**
 * @relatedalso Panel
 * @brief       Shows auto update window
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalUpdater(AppGUI* pApp);

/**
 * @relatedalso Internet
 * @brief       Initialises amber laucnher related inet data
 */
extern void
AutoUpdate_Init(void);

/**
 * @relatedalso Internet
 * @brief       Checks for update
 *
 * @return      bool_t TRUE if update is avaliable
 */
extern bool_t
AutoUpdate_CheckForUpdates(AppGUI *pApp);

/**
 * @relatedalso Internet
 * @brief       Executes update session
 *
 * @return      bool_t TRUE On success
 */
extern bool_t
AutoUpdate_Update(AppGUI *pApp);

/******************************************************************************
 * HEADER CALLBACK DECLARATIONS
 ******************************************************************************/

/**
 * @relatedalso Callback
 * @brief       Invokes full reconfiguration for the game
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnWindowHotkeyF6(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Invokes full reconfiguration for the game
 * 
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonConfigure(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Launches the game and closes AmberLauncher
 * 
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonPlay(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Callback for button for processing debug input strings
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalDebug(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for non-instrusive operations
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalMessage(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for aborting current operations
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalQuestion(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for acquring file paths via file browser
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalGameNotFound(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for processing tweaker wizard events
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalTweaker(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for processing localisation tweak window
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalLocalisation(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Draw callback for localisation window gui-view
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnDrawLocalisation(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for processing button signals from options window
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalOptions(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for processing external commands in tools modal window
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalTools(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for (auto) updater modal window
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonModalUpdater(AppGUI* pApp, Event *e);

/**
 * @relatedalso Async
 * @brief       Schedules task thread to replace main panel
 *
 * @param       pApp
 * @param       eType (EPanelType)
 * @param       dFlags (FPanelFlags)
 */
extern bool_t
GUIThread_SchedulePanelSet(AppGUI *pApp, EPanelType eType, FPanelFlags dFlags);

__END_C

#endif
