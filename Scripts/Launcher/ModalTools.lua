-- DESCRIPTION: Provides options for launching external apps and tools

local TOOLENUM = {
    NULL            = 0,
    AUTOCONFIG      = 1,
    MAX
}

local _ToolsTable = {
    {
        id          = TOOLENUM.AUTOCONFIG,
        iconPath    = "Data/Launcher/tool-autoconfig.png",
        title       = "Autoconfig",
        description = "Re-launches the automatic configuration wizard, letting you fine-tune performance, graphics, and gameplay settings in one guided flow.",
        onClick     = function()
            AL.UICall(UIEVENT.MODAL_CLOSE)
            AL.UICall(UIEVENT.AUTOCONFIG)
        end
    }
}

function ModalShowTools()

    local uiResponse = AL.UICall(UIEVENT.MODAL_TOOLS, _ToolsTable)
    if uiResponse and uiResponse.status == true then
        print(dump(uiResponse))
    end
end
