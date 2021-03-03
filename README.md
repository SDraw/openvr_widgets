# OpenVR Widgets [![Build status](https://ci.appveyor.com/api/projects/status/ctb2t0bawyus9x90?svg=true)](https://ci.appveyor.com/project/SDraw/openvr-widgets) [![Build status](https://ci.appveyor.com/api/projects/status/x308bi2tnillpyjt?svg=true)](https://ci.appveyor.com/project/SDraw/openvr-widgets-linux) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) 
Set of simple overlay widgets for usage in SteamVR environment.  
Application supports Windows 7, 8, 8.1, 10 and Linux (in experimental stage).

# Installation
* Grab latest [release](../../releases/latest) for your platform.
* Extract archive to your preferable folder.
* Launch `openvr_widgets` binary.
* Optional: Enable auto-launch in SteamVR settings.

# Settings
Editing `settings.xml` allows to make few own changes to widgets.  
* **target_rate** - application FPS update rate. Available values: 60, 90, 120, 144. Default value is 60. Adviced to use corresponding menu item in `Settings` of dashboard overlay.
* **gui_font** - font that will be used for text. Default value is `fonts/Hack-Regular.ttf`. Refer to [Hack font repository](../../../../source-foundry/Hack).
* **keyboard_layout** - path to keyboard layout file for keyboard widgets.

# Stats overlay widget
* **Activation:** Double press on grip button of right hand controller.
* **Behaviour:** Follows right hand controller, transparency changes according to view angle on controller.
* **Features:** Shows current system time, CPU load, RAM usage, current FPS in game and VR compositor, controllers charge, trackers charge. Cycle through stats with trigger button while overlay is visible.
* **Deactivation:** Release grip button of right hand controller.
* **Notes:**
  * CPU load isn't always accurate.
  * FPS info is calcuated based on latest VR frame info that isn't always accurate.
  
# Windows capture overlay widget
* **Activation:** Press `OpenVR Widgets - Widgets - Window capture widget` menu item in dashboard.
* **Behaviour:** Captures selected visible system window.
* **Features:**
  * **Movement:** Double press on trigger button of left hand at the center of overlay to attach it to your hand. Double press again to detach.
  * **Resize:** While overlay is attached to hand, bring right hand controller closely to overlay and double press trigger button. Move your right hand controller to adjust preferable size. Let go trigger button on your right hand controller to stop resizing.
  * **Controls:** Bring SteamVR dashboard to make control buttons appear.
    * **Pin/unpin:** Pin/unpin widget to prevent interactions outside of SteamVR dashboard.
    * **Close:** Close current capture widget.
    * **Left:** Cycle window to left.
    * **Right:** Cycle window to right.
    * **Update:** Update list of windows. Helpful when new window has appeared or old has been destroyed.
    * **FPS:** Changes captured FPS rate.
  * **Interaction:** You can interact with captured window using trigger as click and touchpad as scroll.
  * **Notes:** 
    * Not all active and non-active windows accept click and scroll messages sent via Windows API.
    * Latest version of capture library has degraded performance, only single existing capture widget can provide 60 FPS update rate.
    * Input interaction on Linux isn't implemented yet
  
# Keyboard overlay widget
* **Activation:** Press `Add keyboard widget` in dashboard menu `OpenVR Widgets - Settings`.
* **Behaviour:** Emulates global keyboard input.
* **Interaction:** Point to desired key and press trigger button.
* **Notes:** Widget is in experimental state and can be changed in future. Input on Linux isn't implemented yet.
