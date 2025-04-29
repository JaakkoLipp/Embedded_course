/*************************************************************
 * 1.  protocol.h
 *************************************************************/
#ifndef PROTOCOL_H
#define PROTOCOL_H

/* LED commands (1 byte, no parameter) */
#define CMD_MOVEMENT_LED_ON   0x10
#define CMD_MOVEMENT_LED_OFF  0x11
#define CMD_DOOR_LED_ON       0x12
#define CMD_DOOR_LED_OFF      0x13
#define CMD_EMERGENCY         0x14

#endif /* PROTOCOL_H */
