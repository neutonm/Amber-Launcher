#ifndef __AMBER_LAUNCHER_GUI_H
#define __AMBER_LAUNCHER_GUI_H

#include "gui/gui.hxx"
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

#define APP_MAX_ELEMENTS 8
#define APP_MAX_LOCALES 8

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

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
    Window          *pWindow;
    Window          *pWindowModal;
    Panel           *pPanelModal;
    Edit            *pEdit;
    TextView        *pTextView;
    ImageView       *pImageView;
    Layout          *pLayoutMain;
    Layout          *pLayoutModalMain;
    Layout          *pLayoutExtra;
    String          *pString;

    GUITweaker      tGUITweaker[APP_MAX_ELEMENTS];
    unsigned int    dPage;
    unsigned int    dPageMax;

    LangInfo        tLocale[APP_MAX_LOCALES];
    unsigned int    dLocaleCount;
    unsigned int    dLocaleSelected[2];
    PopUp           *pPopupCore;
    PopUp           *pPopupMod;
    View            *pLocaleView;

    struct SObserver *pOnUserEvent;
} AppGUI;

/******************************************************************************
 * ENUMS
 ******************************************************************************/

typedef enum
{
    CPANEL_NULL,
    CPANEL_AUTOCONFIG,
    CPANEL_MAIN,
    CPANEL_IMAGEDEMO,
    CPANEL_MODAL,
    CPANEL_MAX
} EPanelType;

typedef enum
{
    UIEVENT_NULL = 0,
    UIEVENT_MODAL_MESSAGE,
    UIEVENT_MODAL_QUESTION,
    UIEVENT_MODAL_GAMENOTFOUND,
    UIEVENT_MODAL_TWEAKER,
    UIEVENT_MODAL_LOCALISATION,
    UIEVENT_MAX
} EUIEventType;

typedef enum { LOC_CORE, LOC_MOD } LocTier;

extern const char* EUserEventTypeStrings[];

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
Panel_Set(AppGUI* pApp, const EPanelType eType);

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
 * @return      Panel* 
 */
extern Panel* 
Panel_GetAutoConfigure(AppGUI* pApp);

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

__END_C

#endif
