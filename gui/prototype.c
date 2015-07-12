/* shared memory area will contain:
 * The wallpaper.
 * The theme used.
 * The cursor shapes.
 * The fonts.
 * The frame buffer.
 * Resolution information.
 * winman PID.
 */

/* a shared library between programs contain the following code and structs:
 * DrawPixel() {takes RGB and converts it to the color depth of the h/w}.
 * DrawText() {draws text using some specific font.
 * CreateWindow():
 *    Create a window and register it at winman (see below).
 * CreateButton():
 *    blah blah..
 * EnterGUI():
 *    brings the program to GUI mode. the program enters a loop in which
 *    it waits for some event to happen on its windows (the event is
 *    driven by winman).
 * Every object has functions that respond to the events. for example,
 * Window object should be associated with PaintWindow() which is to
 * be executed when a paint event occurs.
 */

/* The winman program will be executed when the OS boots.
 * It does the following:
 * Creates the shared memory area.
 * Loads wallpapers, themes, cursors, ...
 * Creates the framebuffer. It should be
 * identical to the frame buffer of the hardware (same color depth).
 * Enters a loop:
 *    - Draw the wallpaper.
 *    - Ask every window to draw itself
 *    - Copy the frame buffer (The vga just does the copy quickly without
 *                             any processing.)
 * The program should listen to key presses and mouse events, and
 * send those events to children.
 * The program should provide a method to contact with other programs
 * which want to register their windows on screens.
 */

int dummy;
