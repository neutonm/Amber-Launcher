#ifndef __AMBER_LAUNCHER_GUI_H
#define __AMBER_LAUNCHER_GUI_H

#include <core/common.h>
#include <nappgui.h>

#include "res_app.h"
#include <gui/guiall.h>

/******************************************************************************
 * STRUCTS
 ******************************************************************************/

typedef struct _app_t App;
struct _app_t
{
    AppCore      *pAppCore;
    Window       *pWindow;
    TextView     *pText;
    ImageView    *pImageView;
    Layout       *pLayout;
};

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
Panel_Set(App* pApp, const EPanelType eType);

/*---------------------------------------------------------------------------*/

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel with zero elements (for testing)
 * 
 * @param       pApp 
 * @return      Panel* 
 */
extern Panel* 
Panel_GetNull(App* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel responsible for game auto configuration
 * 
 * @param       pApp 
 * @return      Panel* 
 */
extern Panel* 
Panel_GetAutoConfigure(App* pApp);

/**
 * @relatedalso Panel
 * @brief       Returns predefined panel responsbile for core launcher options
 * 
 * @param       pApp 
 * @return      Panel* 
 */
extern Panel* 
Panel_GetMain(App* pApp);

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
Callback_OnButtonConfigure(App* pApp, Event *e);

/**
 * @relatedalso Callback
 * @brief       Launches the game and closes AmberLauncher
 * 
 * @param       pApp 
 * @param       e 
 */
extern void
Callback_OnButtonPlay(App* pApp, Event *e);

__END_C

#endif
