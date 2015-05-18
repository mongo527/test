#include <pebble.h>
#include "momentary.h"
#include "common.h"

static Window *mSwitchesWindow;
static MenuLayer *mSwitches_layer;

int NUMBER_MSWITCHES;
char mSwitches[50][50];
char mSwitchesValue[5][5];
char mSwitchesID[25][25];

uint16_t mSwitches_num_sections_callback(MenuLayer *menu_layer, void *callback_context);
uint16_t mSwitches_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void mSwitches_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

void mSwitches_load(Window *window) {
    menu_layer_reload_data(mSwitches_layer);
}

void mSwitches_unload(Window *window) {
    layer_destroy(window_get_root_layer(mSwitchesWindow));
    menu_layer_destroy(mSwitches_layer);
}

void mSwitches_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    int row = cell_index->row;
    Tuplet mSwitch_update_tuple = TupletCString(AKEY_MSWITCH_UPDATE, mSwitchesID[row]);
    
    DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	
	if (iter == NULL) {
		return;
	}
    
    dict_write_tuplet(iter, &mSwitch_update_tuple);
    dict_write_end(iter);
    
    app_message_outbox_send();
}

void mSwitches_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    mSwitches_load(mSwitchesWindow);
}

void mSwitches_init() {
     mSwitchesWindow = window_create();
    
    window_set_window_handlers(mSwitchesWindow, (WindowHandlers) {
        .load = mSwitches_load,
        .unload = mSwitches_unload,
    });
    
    Layer *layer = window_get_root_layer(mSwitchesWindow);
    mSwitches_layer = menu_layer_create(layer_get_bounds(layer));
    menu_layer_set_callbacks(mSwitches_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) mSwitches_num_sections_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) mSwitches_num_rows_callback,
        .draw_row = (MenuLayerDrawRowCallback) mSwitches_draw_row_callback,
        .select_click = (MenuLayerSelectCallback) mSwitches_select_callback,
        .select_long_click = (MenuLayerSelectCallback) mSwitches_select_long_callback,
    });
    
    menu_layer_set_click_config_onto_window(mSwitches_layer, mSwitchesWindow);
    layer_add_child(layer, menu_layer_get_layer(mSwitches_layer));
    window_stack_push(mSwitchesWindow, true);
}

uint16_t mSwitches_num_sections_callback(MenuLayer *menu_layer, void *callback_context) {
    return 1;
}
 
uint16_t mSwitches_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return NUMBER_MSWITCHES;
}

void mSwitches_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    graphics_context_set_text_color(ctx, GColorBlack);
    /*if(strcmp(mSwitchesValue[cell_index->row], "on")) {
        menu_layer_set_highlight_colors(mSwitches_layer, GColorGreen, GColorBlack);
    }
    else {
        menu_layer_set_highlight_colors(mSwitches_layer, GColorRed, GColorBlack);
    }*/
    menu_cell_basic_draw(ctx, cell_layer, mSwitches[cell_index->row], mSwitchesValue[cell_index->row], NULL);
    //graphics_draw_text(ctx, shows[cell_index->row], fonts_get_system_font(FONT_KEY_GOTHIC_24), (GRect) { .origin = { 8, 0 }, .size = { 144 - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

void mSwitches_in_received_handler(DictionaryIterator *iter) {
    Tuple *mSwitch_name_tuple = dict_find(iter, AKEY_MSWITCH_NAME);
    Tuple *mSwitch_id_tuple = dict_find(iter, AKEY_MSWITCH_ID);
    Tuple *mSwitch_value_tuple = dict_find(iter, AKEY_MSWITCH_VALUE);
    Tuple *mSwitch_index_tuple = dict_find(iter, AKEY_MSWITCH_INDEX);
    Tuple *mSwitch_final_tuple = dict_find(iter, AKEY_MSWITCH_FINAL);
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Index: %d --- Final %d", mSwitch_index_tuple->value->int16, mSwitch_final_tuple->value->int16);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Name: %s --- Value: %s", mSwitch_name_tuple->value->cstring, mSwitch_value_tuple->value->cstring);
    NUMBER_MSWITCHES = mSwitch_final_tuple->value->int16;
    
    if(mSwitch_name_tuple) {
        strcpy(mSwitches[mSwitch_index_tuple->value->int16], mSwitch_name_tuple->value->cstring);
        strcpy(mSwitchesValue[mSwitch_index_tuple->value->int16], mSwitch_value_tuple->value->cstring);
        strcpy(mSwitchesID[mSwitch_index_tuple->value->int16], mSwitch_id_tuple->value->cstring);
        
        if(mSwitch_index_tuple->value->int16 == NUMBER_MSWITCHES-1) {
            mSwitches_load(mSwitchesWindow);
        }
    }
}