[![Build status](https://ci.appveyor.com/api/projects/status/ctb2t0bawyus9x90?svg=true)](https://ci.appveyor.com/project/SDraw/openvr-widgets)
# OpenVR Widgets
Set of simple overlay widgets for usage in SteamVR environment.  
Application supports Windows 7, 8, 8.1 and 10.

# Installation
* Grab latest [release](../../releases/latest) for your platform.
* Extract archive to your preferable folder.
* Launch 'openvr_widgets' binary.
* Optional: Enable auto-launch in SteamVR settings.

# Settings
Editing 'settings.xml' allows to make few own changes to widgets.  
List of global settings:
* **update_rate** - update rate in milliseconds. Default value is 11, that corresponds to 90 FPS.
* **language** - applied language to widgets. Available values: en, ru. Default value is 'en'.
* **gui_font** - font that will be used for text. Default value is 'fonts/Hack-Regular.ttf'. Refer to [Hack font repository](../../../../source-foundry/Hack).

# Stats overlay widget
![](./.github/img_01.png)
* **Activation:** Double press on grip button of right hand controller.
* **Behaviour:** Follows right hand controller, transparency changes according to view angle on controller.
* **Features:** Shows current system time, CPU load, RAM usage and current FPS in game and VR compositor. Cycle through stats with trigger button while overlay is visible.
* **Deactivation:** Release grip button of right hand controller.
  
# Windows capture overlay widget
![](./.github/img_02.png)
* **Activation:** Press 'Add window capture widget' in additional dashboard menu 'OpenVR Widgets - Settings'.
* **Behaviour:** Displays your current visible system windows.
* **Features:**
  * **Movement:** Double press on trigger button of left hand at the center of overlay to attach it to your hand. Double press again to detach.
  * **Resize:** While overlay is attached to hand, bring right hand controller closely to overlay and double press trigger button. Move your right hand controller to adjust preferable size. Let go trigger button on your right hand controller to stop resizing.
  * **Controls:** Bring SteamVR dashboard to make control buttons appear.
    * **Pin:** Pin widget to prevent interactions outside of SteamVR dashboard.
    * **Close:** Close current capture widget.
    * **Left:** Cycle window to left.
    * **Right:** Cycle window to right.
    * **Update:** Update list of windows. Helpful when new window has appeared or old has been destroyed.
    * **FPS:** Changes captured FPS rate.
  * **Interaction:** You can interact with captured window using trigger as click and touchpad as scroll. Note: Not all active and non-active windows accept click and scroll messages sent via Windows API.
* **Notes:** Widget is highly CPU dependent. Consider to be in a range of 5 windows in resolution 1920x1080 and 15 FPS.
