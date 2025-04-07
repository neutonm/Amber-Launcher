#ifndef __AMBER_LAUNCHER_GUI_H
#define __AMBER_LAUNCHER_GUI_H

#include <nappgui.h>

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

struct AppCore;
struct SObserver;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct
{
    struct AppCore  *pAppCore;
    Window          *pWindow;
    Window          *pWindowModal;
    Edit            *pEdit;
    TextView        *pTextView;
    ImageView       *pImageView;
    Layout          *pLayout;
    String          *pString;

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
    CPANEL_MODAL,
    CPANEL_MAX
} EPanelType;

typedef enum
{
    USEREVENT_NULL = 0,
    USEREVENT_MODAL_GAMENOTFOUND,
    USEREVENT_MAX
} EUserEventType;

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
 * @brief       Returns predefined panel for Modal Window related to GameNotFound UI event
 *
 * @param       pApp
 * @return      Panel*
 */
extern Panel*
Panel_GetModalGameNotFound(AppGUI* pApp);

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
 * @brief       Generic callback for confirming operations
 *
 * @param       pApp
 * @param       e
 */
 extern void
 Callback_OnButtonSubmit(AppGUI* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Generic callback for acquring file paths via file browser
 *
 * @param       pApp
 * @param       e
 */
extern void
Callback_OnButtonBrowseFile(AppGUI* pApp, Event *e);

__END_C

#endif
