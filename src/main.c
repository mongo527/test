/**********************************
    *** Created by: Mongo527 ***
**********************************/

#include <pebble.h>
#include "common.h"
#include "helloHome.h"
#include "switches.h"
#include "locks.h"
#include "momentary.h"
    
static Window *window;
static SimpleMenuLayer *main_layer;
static SimpleMenuSection main_sections[1];
static SimpleMenuItem main_items[4];

static void modes_callback (int index, void *ctx) {
    Tuplet modes_tuple = TupletCString(AKEY_FETCH_MODES, "modes");
    
    DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
    
    dict_write_tuplet(iter, &modes_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
    
    modes_init();
}

static void switches_callback (int index, void *ctx) {
    Tuplet fetch_switches_tuple = TupletCString(AKEY_FETCH_SWITCHES, "switches");
    
    DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
    
    dict_write_tuplet(iter, &fetch_switches_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
    
    switches_init();
}

static void locks_callback (int index, void *ctx) {
    Tuplet fetch_locks_tuple = TupletCString(AKEY_FETCH_LOCKS, "locks");
    
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    if(iter == NULL) {
        return;
    }
    
    dict_write_tuplet(iter, &fetch_locks_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
    
    locks_init();
}

static void mSwitches_callback (int index, void *ctx) {
    Tuplet fetch_mSwitches_tuple = TupletCString(AKEY_FETCH_MSWITCHES, "momentary");
    
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    
    if(iter == NULL) {
        return;
    }
    
    dict_write_tuplet(iter, &fetch_mSwitches_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
    
    mSwitches_init();
}

static void window_load(Window *window) {	
	main_items[0] = (SimpleMenuItem) {
        .title = "Modes",
        .callback = modes_callback,
    };
    
    main_items[1] = (SimpleMenuItem) {
		.title = "Switches",
		.callback = switches_callback,
	};
    
    main_items[2] = (SimpleMenuItem) {
        .title = "Locks",
        .callback = locks_callback,
    };
    
    main_items[3] = (SimpleMenuItem) {
        .title = "Momentary",
        .callback = mSwitches_callback,
    };
    
    main_sections[0] = (SimpleMenuSection) {
		.num_items = 4,
		.items = main_items,
	};
    
    Layer *layer = window_get_root_layer(window);
	main_layer = simple_menu_layer_create(layer_get_frame(layer), window, main_sections, 1, NULL);
	layer_add_child(layer, simple_menu_layer_get_layer(main_layer));
}

void window_unload(Window *window) {
    layer_destroy(window_get_root_layer(window));
	simple_menu_layer_destroy(main_layer);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    if(dict_find(iter, AKEY_MODE_NAME)) {
        modes_in_received_handler(iter);
    }
    if(dict_find(iter, AKEY_SWITCH_ID)) {
        switches_in_received_handler(iter);
    }
    if(dict_find(iter, AKEY_LOCK_ID)) {
        locks_in_received_handler(iter);
    }
    if(dict_find(iter, AKEY_MSWITCH_ID)) {
        mSwitches_in_received_handler(iter);
    }
}

static void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Inbound Message was Dropped: %d", reason);
}

static void app_message_init(void) {
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	//app_message_register_outbox_failed(out_failed_handler);
	
	app_message_open(256, 256);
}

void handle_init(void) {
    window = window_create();
    app_message_init();
    
    window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
    
    window_stack_push(window, true);
}

void handle_deinit(void) {
    window_destroy(window);
}

int main(void) {
    handle_init();
    app_event_loop();
    //handle_deinit();
}