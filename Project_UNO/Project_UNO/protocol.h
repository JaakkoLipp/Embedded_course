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

/* Emergency – play buzzer melody once */
#define CMD_BUZZER_PLAY_ONESHOT 0x20

/* additional feature ding */
#define CMD_DING 0x25


#endif /* PROTOCOL_H */
