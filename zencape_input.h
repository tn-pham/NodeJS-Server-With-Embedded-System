// Zen Cape control inputs with accelerometer and joystick
#ifndef ZENCAPE_INPUT_H
#define ZENCAPE_INPUT_H

// init must be called before anything else
// to start taking inputs
void ZenCape_input_init(void);

// cleanup must be called at the end
void ZenCape_input_cleanup(void);

#endif