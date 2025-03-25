#ifndef __AMBER_LAUNCHER_GUI_H
#define __AMBER_LAUNCHER_GUI_H

#include <core/common.h>
#include <nappgui.h>

#include "core/appcore.h"
#include "res_app.h"
#include <gui/guiall.h>

/******************************************************************************
 * FORWARD DECLARATION
 ******************************************************************************/

struct AppCore;

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct
{
    struct AppCore  *pAppCore;
    Window          *pWindow;
    TextView        *pText;
    ImageView       *pImageView;
    Layout          *pLayout;
} AppGUI;

typedef enum
{
    CPANEL_NULL,
    CPANEL_AUTOCONFIG,
    CPANEL_MAIN,
    CPANEL_MAX
} EPanelType;

__EXTERN_C

/******************************************************************************
 * HEADER DECLARATIONS
 ******************************************************************************/

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
 * @brief       Returns predefined panel responsbile for core launcher options
 * 
 * @param       pApp 
 * @return      Panel* 
 */
extern Panel* 
Panel_GetMain(AppGUI* pApp);

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

__END_C

#endif
