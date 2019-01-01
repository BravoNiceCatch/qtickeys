#include "x11keyhook.h"

KeyboardHooker::KeyboardHooker() {
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        printf("unable to connect x server\n");
        exit(1);
    }
    if (!XQueryExtension(display, "XInputExtension", &xi_opcode, &event, &error)) {
        printf("X Input extension not available.\n");
        exit(1);
    }

    if (!xinput_version(display)) {
        printf("%s extension not available.\n", INAME);
    }

    sprintf(deviceId, "9");
}

int KeyboardHooker::xinput_version(Display *display) {
    XExtensionVersion *version;
    static int vers = -1;
    if (vers != -1) {
        return vers;
    }

    version = XGetExtensionVersion(display, INAME);
    if (version && (version != (XExtensionVersion *)NoSuchExtension)) {
        vers = version->major_version;
        XFree(version);
    }

#if HAVE_XI2
    if (vers >= XI_2_Major) {
        const char *forced_version;
        int maj = 2;
        int min = 0;
#if HAVE_XI22
        min = 2;
#elif HAVE_XI21
        min = 1;
#endif
        forced_version = getenv("XINPUT_XI2_VERSION");
        if (forced_version) {
            if (scanf(forced_version, "%d.%d", &maj, &min) != 2) {
                fprintf(stderr,
                        "Invalid format of XIPUT_XI2_VERSION "
                        "envirment variable. None major, minor\n");
                exit(1);
            }
            printf("overriding XI2 version to: %d.%d \n", maj, min);
        }

        XIQueryVersion(display, &maj, &min);
    }
#endif

    return vers;
}

KeyboardHooker::~KeyboardHooker() {
    XSync(display, False);
    XCloseDisplay(display);
}

void KeyboardHooker::Run(std::function<void(unsigned int)> callback) {
    isStop = false;
    XDeviceInfo *info;

    bool handle_proximity = True;
    info                  = find_device_info(display, deviceId, True);
    if (!info) {
        printf("unable to find device '%s'\n", deviceId);
        exit(1);
    } else {
        if (RegisterEvent(display, info, deviceId, handle_proximity)) {
            PrintEvent(display, callback);
        } else {
            fprintf(stderr, "no event registered...\n");
            exit(1);
        }
    }
}

void KeyboardHooker::Stop() { isStop = true; }

int KeyboardHooker::RegisterEvent(Display *dpy, XDeviceInfo *info, char *dev_name, bool handle_proximity) {
    int number = 0;
    XEventClass event_list[7];
    int i;
    XDevice *device;
    Window root_win;
    unsigned long screen;
    XInputClassInfo *ip;

    screen   = DefaultScreen(dpy);
    root_win = RootWindow(dpy, screen);
    device   = XOpenDevice(dpy, info->id);

    if (!device) {
        printf("unable to open device '%s'\n", dev_name);
        return 0;
    }

    if (device->num_classes > 0) {
        for (ip = device->classes, i = 0; i < info->num_classes; ip++, i++) {
            switch (ip->input_class) {
                case KeyClass:
                    DeviceKeyPress(device, key_press_type, event_list[number]);
                    number++;
                    DeviceKeyRelease(device, key_release_type, event_list[number]);
                    number++;
                    break;

                case ButtonClass:
                    DeviceButtonPress(device, button_press_type, event_list[number]);
                    number++;
                    DeviceButtonRelease(device, button_release_type, event_list[number]);
                    number++;
                    break;

                case ValuatorClass:
                    DeviceMotionNotify(device, motion_type, event_list[number]);
                    number++;
                    if (handle_proximity) {
                        ProximityIn(device, proximity_in_type, event_list[number]);
                        number++;
                        ProximityOut(device, proximity_out_type, event_list[number]);
                        number++;
                    }
                    break;

                default:
                    printf("unknown class\n");
                    break;
            }
        }

        if (XSelectExtensionEvent(dpy, root_win, event_list, number)) {
            printf("error selecting extended events\n");
            return 0;
        }
    }
    return number;
}

XDeviceInfo *KeyboardHooker::find_device_info(Display *display, char *name, bool only_extended) {
    XDeviceInfo *devices;
    XDeviceInfo *found = NULL;
    int loop;
    int num_devices;
    int len    = strlen(name);
    bool is_id = True;
    XID id     = (XID)-1;
    for (loop = 0; loop < len; loop++) {
        if (!isdigit(name[loop])) {
            is_id = False;
            break;
        }
    }

    if (is_id) {
        id = atoi(name);
    }

    devices = XListInputDevices(display, &num_devices);
    for (loop = 0; loop < num_devices; loop++) {
        if ((!only_extended || (devices[loop].use >= IsXExtensionDevice)) &&
            ((!is_id && strcmp(devices[loop].name, name) == 0) || (is_id && devices[loop].id == id))) {
            if (found) {
                fprintf(stderr,
                        "Warning: There are multiple devices named '%s'.\n"
                        "To ensure the correct one is selected, please use "
                        "the device ID instead.\n\n",
                        name);
                return NULL;
            } else {
                found = &devices[loop];
            }
        }
    }

    return found;
}

void KeyboardHooker::PrintEvent(Display *dpy, std::function<void(unsigned int)> callback) {
    XEvent Event;
    setvbuf(stdout, NULL, _IOLBF, 0);

    while (!isStop) {
        XNextEvent(dpy, &Event);

        if (Event.type == motion_type) {
            int loop;
            XDeviceMotionEvent *motion = (XDeviceMotionEvent *)&Event;

            printf("motion ");

            for (loop = 0; loop < motion->axes_count; loop++) {
                printf("a[%d]=%d ", motion->first_axis + loop, motion->axis_data[loop]);
            }
            printf("\n");
        } else if ((Event.type == button_press_type) || (Event.type == button_release_type)) {
            int loop;
            XDeviceButtonEvent *button = (XDeviceButtonEvent *)&Event;

            printf("button %s %d ", (Event.type == button_release_type) ? "release" : "press  ", button->button);

            for (loop = 0; loop < button->axes_count; loop++) {
                printf("a[%d]=%d ", button->first_axis + loop, button->axis_data[loop]);
            }
            printf("\n");
        } else if ((Event.type == key_press_type) || (Event.type == key_release_type)) {
            int loop;
            XDeviceKeyEvent *key = (XDeviceKeyEvent *)&Event;
            if (Event.type != key_release_type) {
                callback(key->keycode);
            }

            printf("key %s %d ", (Event.type == key_release_type) ? "release" : "press  ", key->keycode);

            for (loop = 0; loop < key->axes_count; loop++) {
                printf("a[%d]=%d ", key->first_axis + loop, key->axis_data[loop]);
            }
            printf("\n");
        } else if ((Event.type == proximity_out_type) || (Event.type == proximity_in_type)) {
            int loop;
            XProximityNotifyEvent *prox = (XProximityNotifyEvent *)&Event;

            printf("proximity %s ", (Event.type == proximity_in_type) ? "in " : "out");

            for (loop = 0; loop < prox->axes_count; loop++) {
                printf("a[%d]=%d ", prox->first_axis + loop, prox->axis_data[loop]);
            }
            printf("\n");
        } else {
            printf("what's that %d\n", Event.type);
        }
    }
}
