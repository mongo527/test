/**********************************
    *** Created by: Mongo527 ***
**********************************/

#include <pebble.h>
#include "common.h"
#include "helloHome.h"

static Window *modesWindow;
static MenuLayer *modes_layer;

int NUMBER_MODES;
char modes[25][50];

uint16_t modes_num_sections_callback(MenuLayer *menu_layer, void *callback_context);
uint16_t modes_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void modes_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

void modes_load(Window *window) {
    menu_layer_reload_data(modes_layer);
}

void modes_unload(Window *window) {
    layer_destroy(window_get_root_layer(modesWindow));
    menu_layer_destroy(modes_layer);
}

void modes_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    int row = cell_index->row;
    Tuplet mode_set_tuple = TupletCString(AKEY_MODE_SET, modes[row]);
    
    DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
    
    dict_write_tuplet(iter, &mode_set_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
}

void modes_init() {
     modesWindow = window_create();
    
    window_set_window_handlers(modesWindow, (WindowHandlers) {
        .load = modes_load,
        .unload = modes_unload,
    });
    
    Layer *layer = window_get_root_layer(modesWindow);
    modes_layer = menu_layer_create(layer_get_bounds(layer));
    //menu_layer_set_highlight_colors(modes_layer, GColorBlue, GColorBlack);
    menu_layer_set_callbacks(modes_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) modes_num_sections_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) modes_num_rows_callback,
        .draw_row = (MenuLayerDrawRowCallback) modes_draw_row_callback,
        .select_click = (MenuLayerSelectCallback) modes_select_callback,
    });
    
    menu_layer_set_click_config_onto_window(modes_layer, modesWindow);
    layer_add_child(layer, menu_layer_get_layer(modes_layer));
    window_stack_push(modesWindow, true);
}

uint16_t modes_num_sections_callback(MenuLayer *menu_layer, void *callback_context) {
    return 1;
}
 
uint16_t modes_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return NUMBER_MODES;
}

void modes_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    graphics_context_set_text_color(ctx, GColorBlack);
    menu_cell_basic_draw(ctx, cell_layer, modes[cell_index->row], NULL, NULL);
    //graphics_draw_text(ctx, shows[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_24), (GRect) { .origin = { 8, 0 }, .size = { 144 - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

void modes_in_received_handler(DictionaryIterator *iter) {
    Tuple *mode_name_tuple = dict_find(iter, AKEY_MODE_NAME);
    Tuple *mode_index_tuple = dict_find(iter, AKEY_MODE_INDEX);
    Tuple *mode_final_tuple = dict_find(iter, AKEY_MODE_FINAL);
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d --- Final %d", mode_index_tuple->value->int16, mode_final_tuple->value->int16);
    NUMBER_MODES = mode_final_tuple->value->int16;
    
    if(mode_name_tuple) {
        /*APP_LOG(APP_LOG_LEVEL_DEBUG, "Mode Name Length = %d", mode_name_tuple->length);
        for(int i=0; i <= mode_name_tuple->length; i++) {
            APP_LOG(APP_LOG_LEVEL_DEBUG, "%c", mode_name_tuple->value->data[i]);
        }*/
        strcpy(modes[mode_index_tuple->value->int16], mode_name_tuple->value->cstring);
        
        if(mode_index_tuple->value->int16 == NUMBER_MODES-1) {
            modes_load(modesWindow);
        }
    }
}