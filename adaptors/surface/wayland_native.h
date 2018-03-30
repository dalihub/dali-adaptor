#include <stdio.h>
#include <stdlib.h>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <wayland-egl-core.h>

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>
//#include <ttrace.h>

// #ifdef ???
extern "C" {
// #endif
typedef struct WAYLAND_DATA{
	/* Data for wayland */
	struct wl_display *display;
	struct wl_surface *surface;

	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shell *shell;
	struct wl_shm *shm;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
	struct wl_shell_surface *shell_surface;
	struct wl_buffer *buffer;

	/* Data for touch input */
	int has_argb;
	int width, height;
	void *data;
	float x,y;
	int init;
}Wayland_data;


/****************************
void init_wayland_display(Wayland_data *data);
void deinit_wayland_display(Wayland_data *data);

void init_wayland_surface(Wayland_data *data);
void deinit_wayland_surface(Wayland_data *data);

void global_registry_handler(void *data, struct wl_registry *registry, unsigned int id, const char *interface, unsigned int version)
{
	Wayland_data *wl_data = (Wayland_data*)data;
		if (strcmp(interface, "wl_compositor") == 0)
		{
			wl_data->compositor = wl_registry_bind(registry, id, &wl_compositor_interface, 1);
		}
		else if (strcmp(interface, "wl_shell") == 0)
		{
			wl_data->shell = wl_registry_bind(registry, id, &wl_shell_interface, 1);
		}
		else if(strcmp(interface, "wl_shm") == 0)
		{
			wl_data->shm = wl_registry_bind(registry, id, &wl_shm_interface, 1);
			wl_shm_add_listener(wl_data->shm, &shm_listener, wl_data);
		}
		else if(strcmp(interface, "wl_seat") == 0)
		{
			add_seat(wl_data, id, version);
		}

}
void global_registry_remover(void *data, struct wl_registry *registry, unsigned int id)
{
	printf("Got a registry losing event for %d\n", id);

}
void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format);
void add_seat(Wayland_data *touch, uint32_t name, uint32_t version);
void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, enum wl_seat_capability caps);
void touch_handle_down(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, struct wl_surface *surface,
		  int32_t id, wl_fixed_t x_w, wl_fixed_t y_w);

void touch_handle_up(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, int32_t id);
void touch_handle_motion(void *data, struct wl_touch *wl_touch, uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w);
void touch_handle_frame(void *data, struct wl_touch *wl_touch);
void touch_handle_cancel(void *data, struct wl_touch *wl_touch);

void handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);
void handle_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height);
void handle_popup_done(void *data, struct wl_shell_surface *shell_surface);


const struct wl_registry_listener registry_listener = {
	global_registry_handler,
	global_registry_remover
};

*/
// #ifdef ???
}
// #endif
