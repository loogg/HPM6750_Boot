#ifndef __IAP_SLAVE_H
#define __IAP_SLAVE_H

#include <agile_modbus.h>

uint8_t compute_meta_length_after_function_callback(agile_modbus_t *ctx, int function,
                                                    agile_modbus_msg_type_t msg_type);
int compute_data_length_after_meta_callback(agile_modbus_t *ctx, uint8_t *msg, int msg_length,
                                            agile_modbus_msg_type_t msg_type);
int slave_callback(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info);

#endif
