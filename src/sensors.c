#include <pebble.h>
#include "sensors.h"
#include "common.h"
    
static Window *sensorsWindow;
static MenuLayer *sensors_layer;

int NUMBER_CONTACTS=0;
char contacts[50][50];
char contactsValue[8][8];
//char contactsID[25][25];

int NUMBER_MOTIONS=0;
char motions[50][50];
char motionsValue[10][10];
//char motionsID[25][25];

uint16_t sensors_num_sections_callback(MenuLayer *menu_layer, void *callback_context);
uint16_t sensors_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
void sensors_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context);

uint16_t sensors_num_sections_callback(MenuLayer *menu_layer, void *callback_context) {
    return 2;
}

uint16_t sensors_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    switch(section_index) {
        case 0:
            return NUMBER_CONTACTS;
        case 1:
            return NUMBER_MOTIONS;
        default:
            return 0;
    }
}

static int16_t sensors_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void sensors_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
    switch(section_index) {
        case 0:
            menu_cell_basic_header_draw(ctx, cell_layer, "Contact Sensors");
            break;
        case 1:
            menu_cell_basic_header_draw(ctx, cell_layer, "Motion Sensors");
            break;
    }
}

void sensors_draw_row_callback(GContext *ctx, Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    switch(cell_index->section) {
        case 0:
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Name: %s --- Value: %s", contacts[cell_index->row], contactsValue[cell_index->row]);
            menu_cell_basic_draw(ctx, cell_layer, contacts[cell_index->row], contactsValue[cell_index->row], NULL);
            break;
        case 1:
            APP_LOG(APP_LOG_LEVEL_DEBUG, "Name: %s --- Value: %s", motions[cell_index->row], motionsValue[cell_index->row]);
            menu_cell_basic_draw(ctx, cell_layer, motions[cell_index->row], motionsValue[cell_index->row], NULL);
            break;
    }
}

void sensors_load(Window *window) {
    menu_layer_reload_data(sensors_layer);
}

void sensors_unload(Window *window) {
    layer_destroy(window_get_root_layer(sensorsWindow));
    menu_layer_destroy(sensors_layer);
}

void sensors_init() {
    sensorsWindow = window_create();
    window_set_window_handlers(sensorsWindow, (WindowHandlers) {
        .load = sensors_load,
        .unload = sensors_unload,
    });
    
    Layer *layer = window_get_root_layer(sensorsWindow);
    sensors_layer = menu_layer_create(layer_get_bounds(layer));
    menu_layer_set_callbacks(sensors_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = (MenuLayerGetNumberOfSectionsCallback) sensors_num_sections_callback,
        .get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) sensors_num_rows_callback,
        .get_header_height = (MenuLayerGetHeaderHeightCallback) sensors_get_header_height_callback,
        .draw_header = (MenuLayerDrawHeaderCallback) sensors_draw_header_callback,
        .draw_row = (MenuLayerDrawRowCallback) sensors_draw_row_callback,
    });
    
    menu_layer_set_click_config_onto_window(sensors_layer, sensorsWindow);
    layer_add_child(layer, menu_layer_get_layer(sensors_layer));
    window_stack_push(sensorsWindow, true);
}

void sensors_in_received_handler(DictionaryIterator *iter) {
    Tuple *sensor_name_tuple = dict_find(iter, AKEY_SENSOR_NAME);
    Tuple *sensor_value_tuple = dict_find(iter, AKEY_SENSOR_VALUE);
    Tuple *sensor_type_tuple = dict_find(iter, AKEY_SENSOR_TYPE);
    Tuple *sensor_index_tuple = dict_find(iter, AKEY_SENSOR_INDEX);
    Tuple *sensor_final_tuple = dict_find(iter, AKEY_SENSOR_FINAL);
    
    if(sensor_type_tuple) {
        if(strcmp(sensor_type_tuple->value->cstring, "contactSensor") == 0) {
            strcpy(contacts[sensor_index_tuple->value->int16], sensor_name_tuple->value->cstring);
            strcpy(contactsValue[sensor_index_tuple->value->int16], sensor_value_tuple->value->cstring);
            NUMBER_CONTACTS++;
            //APP_LOG(APP_LOG_LEVEL_DEBUG, "NUMBER_CONTACTS: %d", NUMBER_CONTACTS);
        }
        else if(strcmp(sensor_type_tuple->value->cstring, "motionSensor") == 0) {
            strcpy(motions[sensor_index_tuple->value->int16], sensor_name_tuple->value->cstring);
            strcpy(motionsValue[sensor_index_tuple->value->int16], sensor_value_tuple->value->cstring);
            NUMBER_MOTIONS++;
        }
    }
    if(sensor_index_tuple->value->int16 == (sensor_final_tuple->value->int16)-1) {
        sensors_load(sensorsWindow);
    }
    
    
/*
    Tuple *contact_name_tuple = dict_find(iter, AKEY_CONTACT_NAME);
//    Tuple *contact_id_tuple = dict_find(iter, AKEY_CONTACT_ID);
    Tuple *contact_value_tuple = dict_find(iter, AKEY_CONTACT_VALUE);
    Tuple *contact_index_tuple = dict_find(iter, AKEY_CONTACT_INDEX);
    Tuple *contact_final_tuple = dict_find(iter, AKEY_CONTACT_FINAL);
    
    Tuple *motion_name_tuple = dict_find(iter, AKEY_MOTION_NAME);
//    Tuple *motion_id_tuple = dict_find(iter, AKEY_MOTION_ID);
    Tuple *motion_value_tuple = dict_find(iter, AKEY_MOTION_VALUE);
    Tuple *motion_index_tuple = dict_find(iter, AKEY_MOTION_INDEX);
    Tuple *motion_final_tuple = dict_find(iter, AKEY_MOTION_FINAL);
    
    if(contact_name_tuple) {
        NUMBER_CONTACTS = contact_final_tuple->value->int16;
        strcpy(contacts[contact_index_tuple->value->int16], contact_name_tuple->value->cstring);
        strcpy(contactsValue[contact_index_tuple->value->int16], contact_value_tuple->value->cstring);
        
        if(contact_index_tuple->value->int16 == NUMBER_CONTACTS-1) {
            sensors_load(sensorsWindow);
        }
    }
    else if(motion_name_tuple) {
        NUMBER_MOTIONS = motion_final_tuple->value->int16;
        strcpy(motions[motion_index_tuple->value->int16], motion_name_tuple->value->cstring);
        strcpy(motionsValue[motion_index_tuple->value->int16], motion_value_tuple->value->cstring);
        
        if(motion_index_tuple->value->int16 == NUMBER_MOTIONS-1) {
            sensors_load(sensorsWindow);
        }
    }
*/
}