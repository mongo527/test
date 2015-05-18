#include <pebble.h>
#include "switches.h"
#include "common.h"

static Window *switchesWindow;
static MenuLayer *switches_layer;

int NUMBER_SWITCHES;
char switches[50][50];
char switchesValue[5][5];
char switchesID[25][25];

uint16_t switches_num_sections_callback(MenuLayer *menu_layer, void *callback_context);
uint16_t switches_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void switches_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

void switches_load(Window *window) {
    menu_layer_reload_data(switches_layer);
}

void switches_unload(Window *window) {
    layer_destroy(window_get_root_layer(switchesWindow));
    menu_layer_destroy(switches_layer);
}

void switches_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    int row = cell_index->row;
    Tuplet switch_update_tuple = TupletCString(AKEY_SWITCH_UPDATE, switchesID[row]);
    
    DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
    
    dict_write_tuplet(iter, &switch_update_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
}

void switches_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    switches_load(switchesWindow);
}

void switches_init() {
     switchesWindow = window_create();
    
    window_set_window_handlers(switchesWindow, (WindowHandlers) {
        .load = switches_load,
        .unload = switches_unload,
    });
    
    Layer *layer = window_get_root_layer(switchesWindow);
    switches_layer = menu_layer_create(layer_get_bounds(layer));
    menu_layer_set_callbacks(switches_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) switches_num_sections_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) switches_num_rows_callback,
        .draw_row = (MenuLayerDrawRowCallback) switches_draw_row_callback,
        .select_click = (MenuLayerSelectCallback) switches_select_callback,
        .select_long_click = (MenuLayerSelectCallback) switches_select_long_callback,
    });
    
    menu_layer_set_click_config_onto_window(switches_layer, switchesWindow);
    layer_add_child(layer, menu_layer_get_layer(switches_layer));
    window_stack_push(switchesWindow, true);
}

uint16_t switches_num_sections_callback(MenuLayer *menu_layer, void *callback_context) {
    return 1;
}
 
uint16_t switches_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return NUMBER_SWITCHES;
}

void switches_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    graphics_context_set_text_color(ctx, GColorBlack);
    /*if(strcmp(switchesValue[cell_index->row], "on")) {
        menu_layer_set_highlight_colors(switches_layer, GColorGreen, GColorBlack);
    }
    else {
        menu_layer_set_highlight_colors(switches_layer, GColorRed, GColorBlack);
    }*/
    menu_cell_basic_draw(ctx, cell_layer, switches[cell_index->row], switchesValue[cell_index->row], NULL);
    //graphics_draw_text(ctx, shows[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_24), (GRect) { .origin = { 8, 0 }, .size = { 144 - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

void switches_in_received_handler(DictionaryIterator *iter) {
    Tuple *switch_name_tuple = dict_find(iter, AKEY_SWITCH_NAME);
    Tuple *switch_id_tuple = dict_find(iter, AKEY_SWITCH_ID);
    Tuple *switch_value_tuple = dict_find(iter, AKEY_SWITCH_VALUE);
    Tuple *switch_index_tuple = dict_find(iter, AKEY_SWITCH_INDEX);
    Tuple *switch_final_tuple = dict_find(iter, AKEY_SWITCH_FINAL);
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d --- Final %d", mode_index_tuple->value->int16, mode_final_tuple->value->int16);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Name: %s --- Value: %s", switch_name_tuple->value->cstring, switch_value_tuple->value->cstring);
    NUMBER_SWITCHES = switch_final_tuple->value->int16;
    
    if(switch_name_tuple) {
        strcpy(switches[switch_index_tuple->value->int16], switch_name_tuple->value->cstring);
        strcpy(switchesValue[switch_index_tuple->value->int16], switch_value_tuple->value->cstring);
        strcpy(switchesID[switch_index_tuple->value->int16], switch_id_tuple->value->cstring);
        
        if(switch_index_tuple->value->int16 == NUMBER_SWITCHES-1) {
            switches_load(switchesWindow);
        }
    }
}