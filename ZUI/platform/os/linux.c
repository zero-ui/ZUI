#include <ZUI.h>


#if (defined PLATFORM_OS_LINUX)


xcb_connection_t *connection;
const xcb_setup_t *setup;
xcb_screen_iterator_t iter;
xcb_screen_t *screen;


ZuiBool ZuiOsInitialize() {
	connection = xcb_connect(NULL, NULL);
	setup = xcb_get_setup(connection);
	iter = xcb_setup_roots_iterator(setup);
	screen = iter.data;
	return screen != NULL;
}

ZuiOsWindow ZuiOsCreateWindow(int Left, int Top, int Width, int Height, ZuiText Title, ZuiBool ShowInTaskbar, ZuiBool PosMiddle, ZuiWindow Window) {

	return NULL;
}
ZuiVoid ZuiOsDestroyWindow(ZuiOsWindow OsWindow) {
	if (OsWindow) {
		xcb_destroy_window(connection, OsWindow->xwindow);
		free(OsWindow);
	}
}
ZuiVoid ZuiOsSetWindowTitle(ZuiOsWindow OsWindow, ZuiText Title) {
	if (OsWindow) {
		xcb_change_property(connection,
			XCB_PROP_MODE_REPLACE,
			OsWindow->xwindow,
			XCB_ATOM_WM_NAME,
			XCB_ATOM_STRING,
			8,
			strlen(Title.c_str()),
			Title.c_str());
	}
}
int ZuiOsGetWindowTitle(ZuiOsWindow OsWindow, ZuiText out, int outlen) {
	if (OsWindow) {
		

	}
	return 0;
}
#endif