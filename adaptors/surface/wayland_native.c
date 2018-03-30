#include "wayland_native.h"
//#include "gles_common.h"

const struct wl_registry_listener registry_listener = {
	global_registry_handler,
	global_registry_remover
};

struct wl_shm_listener shm_listener = {
	shm_format
};

const struct wl_seat_listener seat_listener = {
	seat_handle_capabilities,
};

const struct wl_touch_listener touch_listener = {
	touch_handle_down,
	touch_handle_up,
	touch_handle_motion,
	touch_handle_frame,
	touch_handle_cancel,
};

const struct wl_shell_surface_listener shell_surface_listener = {
	handle_ping,
	handle_configure,
	handle_popup_done
};

struct seat {
	Wayland_data *touch;
	struct wl_seat *seat;
	struct wl_touch *wl_touch;
};

/* Init wayland native data tyep */
void init_wayland_display(Wayland_data *data){
	data->display = wl_display_connect(NULL);
	if(data->display == NULL)
		printf("%s(%d) Fail to connect wayland display!\n", __func__, __LINE__);
	else
		printf("Init Wayland display 0x%x \n", (int)data->display);
}

void deinit_wayland_display(Wayland_data *data){
	if(data->display)
		wl_display_disconnect(data->display);
}

void init_wayland_surface(Wayland_data *data){
	struct wl_region *region;

	data->registry = wl_display_get_registry(data->display);
	wl_registry_add_listener(data->registry , &registry_listener, data);

	wl_display_dispatch(data->display);
    wl_display_roundtrip(data->display);

	data->surface = wl_compositor_create_surface(data->compositor);

	data->shell_surface = wl_shell_get_shell_surface(data->shell, data->surface);
	wl_shell_surface_add_listener (data->shell_surface, &shell_surface_listener, data);
	wl_shell_surface_set_toplevel (data->shell_surface);

	//Default window is 32bit, to run on NoCOMP mode window should be set 24bit
	region = wl_compositor_create_region(data->compositor);
	wl_region_add(region, 0, 0, data->width, data->height);
	wl_surface_set_opaque_region(data->surface, region);
}

void deinit_wayland_surface(Wayland_data *data){
	if(data->compositor)
		wl_compositor_destroy(data->compositor);
	if(data->shell_surface)
		wl_shell_surface_destroy(data->shell_surface);
	if(data->surface)
		wl_surface_destroy(data->surface);

}

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

void shm_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
	Wayland_data *touch = data;

	if (format == WL_SHM_FORMAT_ARGB8888)
		touch->has_argb = 1;
}

void add_seat(Wayland_data *touch, uint32_t name, uint32_t version)
{
	struct seat *seat;

	seat = malloc(sizeof *seat);

	seat->touch = touch;
	seat->wl_touch = NULL;
	seat->seat = wl_registry_bind(touch->registry, name,
				      &wl_seat_interface, 1);
	wl_seat_add_listener(seat->seat, &seat_listener, seat);
}

void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
			 enum wl_seat_capability caps)
{
	struct seat *seat = data;
	Wayland_data *touch = seat->touch;

	if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !seat->wl_touch) {
		seat->wl_touch = wl_seat_get_touch(wl_seat);
		wl_touch_set_user_data(seat->wl_touch, touch);
		wl_touch_add_listener(seat->wl_touch, &touch_listener, touch);
	} else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && seat->wl_touch) {
		wl_touch_destroy(seat->wl_touch);
		seat->wl_touch = NULL;
	}
}

void touch_handle_down(void *data, struct wl_touch *wl_touch,
		  uint32_t serial, uint32_t time, struct wl_surface *surface,
		  int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
	Wayland_data *touch = data;
	float x = wl_fixed_to_double(x_w);
	float y = wl_fixed_to_double(y_w);

	touch->init = 1;
}

void touch_handle_up(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, int32_t id)
{
	Wayland_data *touch = data;
}

void touch_handle_motion(void *data, struct wl_touch *wl_touch,
		    uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
	Wayland_data *touch = data;
	float x = wl_fixed_to_double(x_w);
	float y = wl_fixed_to_double(y_w);

	touch->x = x;
	touch->y = y;
}

void touch_handle_frame(void *data, struct wl_touch *wl_touch)
{
}

void touch_handle_cancel(void *data, struct wl_touch *wl_touch)
{
}

void handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

void handle_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height)
{
}

void handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

