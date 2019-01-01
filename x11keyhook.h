#ifndef X11KEYHOOK_H
#define X11KEYHOOK_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#if HAVE_XI2
#include <X11/extensions/XInput2.h>
#endif
#include <ctype.h>
#include <string.h>
#include <functional>

#undef Bool
#undef CursorShape
#undef Expose
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef None
#undef Status
#undef Unsorted

using namespace std;

#define INVALID_EVENT_TYPE -1

static int motion_type         = INVALID_EVENT_TYPE;
static int button_press_type   = INVALID_EVENT_TYPE;
static int button_release_type = INVALID_EVENT_TYPE;
static int key_press_type      = INVALID_EVENT_TYPE;
static int key_release_type    = INVALID_EVENT_TYPE;
static int proximity_in_type   = INVALID_EVENT_TYPE;
static int proximity_out_type  = INVALID_EVENT_TYPE;

typedef void (*CallbackType)(unsigned int keycode);

class KeyboardHooker {
 private:
    bool isStop;
    Display *display;
    int xi_opcode;
    int event;
    int error;
    char deviceId[10];
    int RegisterEvent(Display *dpy, XDeviceInfo *info, char *dev_name, bool handle_proximity);
    void PrintEvent(Display *dpy, std::function<void(unsigned int)> callback);
    int xinput_version(Display *display);
    XDeviceInfo *find_device_info(Display *display, char *name, bool only_extended);

 public:
    KeyboardHooker();
    ~KeyboardHooker();
    void Run(std::function<void(unsigned int)> f);
    void Stop();
};

#endif  // X11KEYHOOK_H
