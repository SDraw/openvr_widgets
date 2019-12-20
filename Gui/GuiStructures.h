#pragma once

enum GuiMouseClick : unsigned char
{
    GuiMouseClick_Left = 0U,
    GuiMouseClick_Right,
    GuiMouseClick_Middle
};
enum GuiMouseClickState : unsigned char
{
    GuiClickState_Press = 0U,
    GuiClickState_Release
};
