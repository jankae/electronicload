#ifndef GUI_SIGNALS_H_
#define GUI_SIGNALS_H_

typedef enum  {GUI_OK, GUI_ERROR, GUI_UNABLE} GUIResult_t;

typedef struct signal GUISignal_t;

struct signal {
    uint32_t clicked;
    int32_t encoder;
    struct {
        uint8_t lostFocus :1;
        uint8_t gotFocus :1;
    };
};

#endif
