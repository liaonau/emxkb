/* Thanks to Evgeny Chukreev © 2005, GNU GPL */
/* See http://akshaal.livejournal.com/58473.html */
/* Modified a little by Bzek .) */
/* Modified by Roman Leonov */

/* Usage:
 *      emxkb 0  # For first  group
 *      emxkb 1  # For second group
 */

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EMACS_CLASS "Emacs"

int    lock_group(int window_id, int group);
int    send_key_to_emacs(Display *display, int window_id, int group);
Window get_client(Display *display, Window w);
int    fprintf_return_error(int return_code, const char *message);

int fprintf_return_error(int return_code, const char *message)
{
    fprintf(stderr, "%s\n", message);
    return return_code;
}

int main(int argc, char *argv[])
{
    Display    *display;
    XClassHint  class_hint;
    Window      focus;
    Window      focus_parent;
    int         unused_rtr;
    int         group;

    if (argc < 2)
        return fprintf_return_error(1, "Usage: emxkb <group>");
    group   = strtol(argv[1], NULL, 0);
    display = XOpenDisplay(NULL);
    if (!display)
        return fprintf_return_error(2, "Can't open display");

    XGetInputFocus(display, &focus, &unused_rtr);
    if (!focus)
        return fprintf_return_error(3, "Can't get focus");

    focus_parent = get_client(display, focus);
    if (!focus_parent)
        focus_parent = focus; /* root Window? */
        /*return fprintf_return_error(3, "Can't get focus parent");*/

    XGetClassHint(display, focus_parent, &class_hint);
    if (class_hint.res_class != NULL
        && !strncmp(class_hint.res_class, EMACS_CLASS, strlen(EMACS_CLASS)))
    {
        /* Yeah, Emacs! Send a key. */
        if (!send_key_to_emacs(display, focus, group))
            return fprintf_return_error(4, "Failed during sending a key to Emacs");
        /* Change group to 0 (it should be "us") for Emacs */
        group = 0;
    }
    if (!lock_group(focus, group))
        return fprintf_return_error(5, "Failed during locking group");
    XCloseDisplay(display);
    return EXIT_SUCCESS;
}

Window get_client(Display *display, Window w)
{
    int    done = False;
    Atom   atom = XInternAtom(display, "WM_STATE", False);
    while (!done)
    {
        if (w == 0) return 0;
        long unsigned int  items      = 0;
        Atom               unused_atr = 0;
        int                unused_afr = 0;
        unsigned long      unused_bar = 0;
        unsigned char     *unused_prop;
        if (Success != XGetWindowProperty(display, w, atom,
                                          0, (~0L), False,
                                          AnyPropertyType, &unused_atr, &unused_afr,
                                          &items,
                                          &unused_bar, &unused_prop))
        {
            return 0;
        }
        if (items == 0)
        {
            Window parent;
            Window *children = NULL;
            Window unused_root;
            unsigned int unused_nchildren_return;
            XQueryTree(display, w, &unused_root, &parent, &children, &unused_nchildren_return);
            if (children != NULL)
                XFree(children);
            w = parent;
        }
        else
        {
            done = True;
        }
    }
    return w;
}

int send_key_to_emacs(Display *display, int window_id, int group)
{
    XKeyEvent event;

    /* Init event */
    memset(&event, 0, sizeof(event));
    event.window  = window_id;
    event.display = display;
    event.root    = RootWindow(display, DefaultScreen(display));
    event.state   = 0;
    event.keycode = XKeysymToKeycode(display, XStringToKeysym("space"));

    switch (group)
    {
    case 0:
        event.state = Mod4Mask;
        break;
    case 1:
        event.state = Mod3Mask;
        break;
    case 2:
        event.state = Mod4Mask & Mod3Mask;
        break;
    default:
        event.state = Mod4Mask;
        break;
    }

    /* Send KeyPress event */
    event.type = KeyPress;
    if (!XSendEvent(display, window_id, False, 0, (XEvent*) &event))
        return fprintf_return_error(False, "send_key_to_emacs(): Can't send KeyPress event");
    /* Send KeyRelease event */
    event.type = KeyRelease;
    if (!XSendEvent(display, window_id, False, 0, (XEvent*) &event))
        return fprintf_return_error(False, "send_key_to_emacs(): Can't send KeyRelease event");

    XSync(display, False);
    return True;
}

int lock_group(int window_id, int group)
{
    Display *xkb_display = XkbOpenDisplay(NULL, NULL, NULL, NULL, NULL, NULL);
    if (!xkb_display)
        return fprintf_return_error(False, "lock_group(): Can't open display");
    /* Init XKB */
    if (!XkbQueryExtension(xkb_display, NULL, NULL, NULL, NULL, NULL))
        return fprintf_return_error(False, "lock_group(): Can't init XKB");
    /* Set Focus */
    if (window_id > 0)
    {
        XSetInputFocus(xkb_display, window_id, RevertToParent, CurrentTime);
        XSync(xkb_display, False);
    }
    if (!XkbLockGroup(xkb_display, XkbUseCoreKbd, abs(group)))
        return fprintf_return_error(False, "lock_group(): Can't lock group");

    XSync(xkb_display, False);
    XCloseDisplay(xkb_display);
    return True;
}
