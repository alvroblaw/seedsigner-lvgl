#include <string.h>

#include "py/runtime.h"
#include "py/obj.h"
#include "py/mphal.h"

#include "seedsigner_lvgl/bindings/c_api.h"

typedef struct _uiseedsigner_runtime_obj_t {
    mp_obj_base_t base;
    ss_runtime_t* rt;
} uiseedsigner_runtime_obj_t;

typedef struct _uiseedsigner_event_obj_t {
    mp_obj_base_t base;
    mp_obj_t type;
    mp_obj_t action_id;
    mp_obj_t component_id;
    mp_obj_t value;
    mp_obj_t meta_key;
    mp_obj_t meta_value;
} uiseedsigner_event_obj_t;

extern const mp_obj_type_t uiseedsigner_event_type;

static mp_obj_t none_if_null(const char* s) {
    return s ? mp_obj_new_str(s, strlen(s)) : mp_const_none;
}

static mp_obj_t value_from_event(const ss_event* ev) {
    switch (ev->value_tag) {
        case 1: return ev->value.bool_val ? mp_const_true : mp_const_false;
        case 2: return mp_obj_new_int_from_ll(ev->value.int_val);
        case 3: return ev->value.str_val ? mp_obj_new_str(ev->value.str_val, strlen(ev->value.str_val)) : mp_const_none;
        default: return mp_const_none;
    }
}

static mp_obj_t meta_value_from_event(const ss_event* ev) {
    switch (ev->meta_value_tag) {
        case 1: return ev->meta_value_str ? mp_obj_new_str(ev->meta_value_str, strlen(ev->meta_value_str)) : mp_const_none;
        case 2: return mp_obj_new_int_from_ll(ev->meta_value_int);
        default: return mp_const_none;
    }
}

static void event_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)kind;
    uiseedsigner_event_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "UiEvent(type=%d)", mp_obj_get_int(self->type));
}

static mp_obj_t make_event_obj(ss_event ev) {
    uiseedsigner_event_obj_t* obj = mp_obj_malloc(uiseedsigner_event_obj_t, &uiseedsigner_event_type);
    obj->type = mp_obj_new_int(ev.type);
    obj->action_id = none_if_null(ev.action_id);
    obj->component_id = none_if_null(ev.component_id);
    obj->value = value_from_event(&ev);
    obj->meta_key = none_if_null(ev.meta_key);
    obj->meta_value = meta_value_from_event(&ev);
    return MP_OBJ_FROM_PTR(obj);
}

static mp_obj_t event_attr(mp_obj_t self_in, qstr attr) {
    uiseedsigner_event_obj_t* self = MP_OBJ_TO_PTR(self_in);
    switch (attr) {
        case MP_QSTR_type: return self->type;
        case MP_QSTR_action_id: return self->action_id;
        case MP_QSTR_component_id: return self->component_id;
        case MP_QSTR_value: return self->value;
        case MP_QSTR_meta_key: return self->meta_key;
        case MP_QSTR_meta_value: return self->meta_value;
        default: return MP_OBJ_NULL;
    }
}

static void event_attr_handler(mp_obj_t self_in, qstr attr, mp_obj_t* dest) {
    if (dest[0] == MP_OBJ_NULL) {
        dest[0] = event_attr(self_in, attr);
    }
}

MP_DEFINE_CONST_OBJ_TYPE(
    uiseedsigner_event_type,
    MP_QSTR_UiEvent,
    MP_TYPE_FLAG_NONE,
    print, event_print,
    attr, event_attr_handler
    );

static void runtime_print(const mp_print_t* print, mp_obj_t self_in, mp_print_kind_t kind) {
    (void)self_in;
    (void)kind;
    mp_printf(print, "UiRuntime()");
}

static mp_obj_t runtime_make_new(const mp_obj_type_t* type, size_t n_args, size_t n_kw, const mp_obj_t* args) {
    mp_arg_check_num(n_args, n_kw, 0, 2, false);
    uiseedsigner_runtime_obj_t* self = mp_obj_malloc(uiseedsigner_runtime_obj_t, type);
    self->rt = ss_create(n_args >= 1 ? mp_obj_get_int(args[0]) : 240,
                         n_args >= 2 ? mp_obj_get_int(args[1]) : 320);
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t runtime_init(mp_obj_t self_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    if (!ss_init(self->rt)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("ss_init failed"));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(runtime_init_obj, runtime_init);

static mp_obj_t runtime_activate(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    const char* route_id = mp_obj_str_get_str(args[1]);
    const char* route_args = n_args >= 3 ? mp_obj_str_get_str(args[2]) : NULL;
    return mp_obj_new_int(ss_activate(self->rt, route_id, route_args));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_activate_obj, 2, 3, runtime_activate);

static mp_obj_t runtime_replace(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    const char* route_id = mp_obj_str_get_str(args[1]);
    const char* route_args = n_args >= 3 ? mp_obj_str_get_str(args[2]) : NULL;
    return mp_obj_new_int(ss_replace(self->rt, route_id, route_args));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_replace_obj, 2, 3, runtime_replace);

static mp_obj_t runtime_navigate(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    const char* action = mp_obj_str_get_str(args[1]);
    const char* route_id = n_args >= 3 && args[2] != mp_const_none ? mp_obj_str_get_str(args[2]) : NULL;
    const char* route_args = n_args >= 4 && args[3] != mp_const_none ? mp_obj_str_get_str(args[3]) : NULL;
    return mp_obj_new_int(ss_navigate(self->rt, action, route_id, route_args));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_navigate_obj, 2, 4, runtime_navigate);

static mp_obj_t runtime_get_active_route(mp_obj_t self_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    ss_active_route route = ss_get_active_route(self->rt);
    if (!route.valid) return mp_const_none;
    mp_obj_t tuple[3] = {
        mp_obj_new_str(route.route_id, strlen(route.route_id)),
        mp_obj_new_int(route.screen_token),
        mp_obj_new_int(route.stack_depth),
    };
    return mp_obj_new_tuple(3, tuple);
}
static MP_DEFINE_CONST_FUN_OBJ_1(runtime_get_active_route_obj, runtime_get_active_route);

static mp_obj_t runtime_send_input(mp_obj_t self_in, mp_obj_t key_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    ss_send_input(self->rt, (ss_input_key)mp_obj_get_int(key_in));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(runtime_send_input_obj, runtime_send_input);

static mp_obj_t runtime_set_screen_data(mp_obj_t self_in, mp_obj_t data_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(ss_set_screen_data(self->rt, mp_obj_str_get_str(data_in)));
}
static MP_DEFINE_CONST_FUN_OBJ_2(runtime_set_screen_data_obj, runtime_set_screen_data);

static mp_obj_t runtime_patch_screen_data(mp_obj_t self_in, mp_obj_t data_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_int(ss_patch_screen_data(self->rt, mp_obj_str_get_str(data_in)));
}
static MP_DEFINE_CONST_FUN_OBJ_2(runtime_patch_screen_data_obj, runtime_patch_screen_data);

static mp_obj_t runtime_push_modal(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    const char* modal_id = mp_obj_str_get_str(args[1]);
    const char* data = n_args >= 3 ? mp_obj_str_get_str(args[2]) : NULL;
    return mp_obj_new_int(ss_push_modal(self->rt, modal_id, data));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_push_modal_obj, 2, 3, runtime_push_modal);

static mp_obj_t runtime_dismiss_modal(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    uint32_t token = (uint32_t)mp_obj_get_int(args[1]);
    const char* result = n_args >= 3 ? mp_obj_str_get_str(args[2]) : NULL;
    return mp_obj_new_int(ss_dismiss_modal(self->rt, token, result));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_dismiss_modal_obj, 2, 3, runtime_dismiss_modal);

static mp_obj_t runtime_next_event(mp_obj_t self_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    ss_event ev = ss_next_event(self->rt);
    if (ev.type == SS_EVENT_NONE) return mp_const_none;
    return make_event_obj(ev);
}
static MP_DEFINE_CONST_FUN_OBJ_1(runtime_next_event_obj, runtime_next_event);

static mp_obj_t runtime_tick(mp_obj_t self_in, mp_obj_t ms_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    ss_tick(self->rt, (uint32_t)mp_obj_get_int(ms_in));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(runtime_tick_obj, runtime_tick);

static mp_obj_t runtime_refresh(mp_obj_t self_in) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(self_in);
    ss_refresh(self->rt);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(runtime_refresh_obj, runtime_refresh);

static mp_obj_t runtime_push_frame(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    mp_buffer_info_t buf;
    ss_camera_frame frame;
    mp_get_buffer_raise(args[1], &buf, MP_BUFFER_READ);
    frame.pixels = (const uint8_t*)buf.buf;
    frame.width = (uint32_t)mp_obj_get_int(args[2]);
    frame.height = (uint32_t)mp_obj_get_int(args[3]);
    frame.stride = frame.width;
    frame.sequence = 0;
    return mp_obj_new_int(ss_push_frame(self->rt, &frame));
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_push_frame_obj, 4, 4, runtime_push_frame);

static mp_obj_t runtime_enter(mp_obj_t self_in) { return self_in; }
static MP_DEFINE_CONST_FUN_OBJ_1(runtime_enter_obj, runtime_enter);

static mp_obj_t runtime_exit(size_t n_args, const mp_obj_t* args) {
    uiseedsigner_runtime_obj_t* self = MP_OBJ_TO_PTR(args[0]);
    (void)n_args;
    if (self->rt) {
        ss_destroy(self->rt);
        self->rt = NULL;
    }
    return mp_const_false;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(runtime_exit_obj, 4, 4, runtime_exit);

static const mp_rom_map_elem_t runtime_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&runtime_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_activate), MP_ROM_PTR(&runtime_activate_obj) },
    { MP_ROM_QSTR(MP_QSTR_replace), MP_ROM_PTR(&runtime_replace_obj) },
    { MP_ROM_QSTR(MP_QSTR_navigate), MP_ROM_PTR(&runtime_navigate_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_active_route), MP_ROM_PTR(&runtime_get_active_route_obj) },
    { MP_ROM_QSTR(MP_QSTR_send_input), MP_ROM_PTR(&runtime_send_input_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_screen_data), MP_ROM_PTR(&runtime_set_screen_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_patch_screen_data), MP_ROM_PTR(&runtime_patch_screen_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_push_modal), MP_ROM_PTR(&runtime_push_modal_obj) },
    { MP_ROM_QSTR(MP_QSTR_dismiss_modal), MP_ROM_PTR(&runtime_dismiss_modal_obj) },
    { MP_ROM_QSTR(MP_QSTR_next_event), MP_ROM_PTR(&runtime_next_event_obj) },
    { MP_ROM_QSTR(MP_QSTR_tick), MP_ROM_PTR(&runtime_tick_obj) },
    { MP_ROM_QSTR(MP_QSTR_refresh), MP_ROM_PTR(&runtime_refresh_obj) },
    { MP_ROM_QSTR(MP_QSTR_push_frame), MP_ROM_PTR(&runtime_push_frame_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&runtime_enter_obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&runtime_exit_obj) },
};
static MP_DEFINE_CONST_DICT(runtime_locals_dict, runtime_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    uiseedsigner_runtime_type,
    MP_QSTR_UiRuntime,
    MP_TYPE_FLAG_NONE,
    print, runtime_print,
    make_new, runtime_make_new,
    locals_dict, &runtime_locals_dict
    );

#define SS_CONST(name, value) { MP_ROM_QSTR(MP_QSTR_##name), MP_ROM_INT(value) }

static const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_uiseedsigner) },
    { MP_ROM_QSTR(MP_QSTR_UiRuntime), MP_ROM_PTR(&uiseedsigner_runtime_type) },
    SS_CONST(KEY_UP, 0),
    SS_CONST(KEY_DOWN, 1),
    SS_CONST(KEY_LEFT, 2),
    SS_CONST(KEY_RIGHT, 3),
    SS_CONST(KEY_PRESS, 4),
    SS_CONST(KEY_BACK, 5),
    SS_CONST(EVENT_ROUTE_ACTIVATED, 0),
    SS_CONST(EVENT_SCREEN_READY, 1),
    SS_CONST(EVENT_ACTION_INVOKED, 2),
    SS_CONST(EVENT_CANCEL_REQUESTED, 3),
    SS_CONST(EVENT_NEEDS_DATA, 4),
    SS_CONST(EVENT_ERROR, 5),
    SS_CONST(EVENT_NONE, 255),
};
static MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

const mp_obj_module_t mp_module_uiseedsigner = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_uiseedsigner, mp_module_uiseedsigner);
