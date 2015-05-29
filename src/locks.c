#include <pebble.h>
#include "locks.h"
#include "common.h"

static Window *locksWindow;
static MenuLayer *locks_layer;

int NUMBER_LOCKS;
char locks[50][50];
char locksValue[5][5];
char locksID[25][25];

uint16_t locks_num_sections_callback(MenuLayer *menu_layer, void *callback_context);
uint16_t locks_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void locks_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

void locks_load(Window *window) {
    menu_layer_reload_data(locks_layer);
}

void locks_unload(Window *window) {
    layer_destroy(window_get_root_layer(locksWindow));
    menu_layer_destroy(locks_layer);
}

void locks_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    int row = cell_index->row;
    Tuplet lock_update_tuple = TupletCString(AKEY_LOCK_UPDATE, locksID[row]);
    
    DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
    
    dict_write_tuplet(iter, &lock_update_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
}

void locks_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    locks_load(locksWindow);
}

void locks_init() {
     locksWindow = window_create();
    
    window_set_window_handlers(locksWindow, (WindowHandlers) {
        .load = locks_load,
        .unload = locks_unload,
    });
    
    Layer *layer = window_get_root_layer(locksWindow);
    locks_layer = menu_layer_create(layer_get_bounds(layer));
    menu_layer_set_callbacks(locks_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) locks_num_sections_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) locks_num_rows_callback,
        .draw_row = (MenuLayerDrawRowCallback) locks_draw_row_callback,
        .select_click = (MenuLayerSelectCallback) locks_select_callback,
        .select_long_click = (MenuLayerSelectCallback) locks_select_long_callback,
    });
    
    menu_layer_set_click_config_onto_window(locks_layer, locksWindow);
    layer_add_child(layer, menu_layer_get_layer(locks_layer));
    window_stack_push(locksWindow, true);
}

uint16_t locks_num_sections_callback(MenuLayer *menu_layer, void *callback_context) {
    return 1;
}
 
uint16_t locks_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return NUMBER_LOCKS;
}

void locks_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    //graphics_context_set_text_color(ctx, GColorBlack);
    /*if(strcmp(locksValue[cell_index->row], "on")) {
        menu_layer_set_highlight_colors(locks_layer, GColorGreen, GColorBlack);
    }
    else {
        menu_layer_set_highlight_colors(locks_layer, GColorRed, GColorBlack);
    }*/
    menu_cell_basic_draw(ctx, cell_layer, locks[cell_index->row], locksValue[cell_index->row], NULL);
    //graphics_draw_text(ctx, shows[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_24), (GRect) { .origin = { 8, 0 }, .size = { 144 - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

void locks_in_received_handler(DictionaryIterator *iter) {
    Tuple *lock_name_tuple = dict_find(iter, AKEY_LOCK_NAME);
    Tuple *lock_id_tuple = dict_find(iter, AKEY_LOCK_ID);
    Tuple *lock_value_tuple = dict_find(iter, AKEY_LOCK_VALUE);
    Tuple *lock_index_tuple = dict_find(iter, AKEY_LOCK_INDEX);
    Tuple *lock_final_tuple = dict_find(iter, AKEY_LOCK_FINAL);
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d --- Final %d", lock_index_tuple->value->int16, lock_final_tuple->value->int16);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Name: %s --- Value: %s", lock_name_tuple->value->cstring, lock_value_tuple->value->cstring);
    NUMBER_LOCKS = lock_final_tuple->value->int16;
    
    if(lock_name_tuple) {
        strcpy(locks[lock_index_tuple->value->int16], lock_name_tuple->value->cstring);
        strcpy(locksValue[lock_index_tuple->value->int16], lock_value_tuple->value->cstring);
        strcpy(locksID[lock_index_tuple->value->int16], lock_id_tuple->value->cstring);
        
        if(lock_index_tuple->value->int16 == NUMBER_LOCKS-1) {
            locks_load(locksWindow);
        }
    }
}