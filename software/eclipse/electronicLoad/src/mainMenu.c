#include "mainMenu.h"

static container_t c;
static valueLabel_t lBig, lSmall1, lSmall2;
static label_t lMode, lOn, lOverride1, lOverride2;
static entry_t eSetting;

static uint32_t null = 0;

void mainMenu_Init() {
    /* create main screen */
    label_createWithLength(&lMode, 3, FONT_MEDIUM);
    label_createWithLength(&lOn, 3, FONT_BIG);
    label_createWithLength(&lOverride1, 9, FONT_SMALL);
    label_createWithLength(&lOverride2, 9, FONT_SMALL);

    valueLabel_create(&lBig, &load.state.current, FONT_BIG, 4, UNIT_CURRENT);
    valueLabel_create(&lSmall1, &load.state.voltage, FONT_MEDIUM, 4,
            UNIT_VOLTAGE);
    valueLabel_create(&lSmall2, &load.state.power, FONT_MEDIUM, 4, UNIT_POWER);

    entry_create(&eSetting, &load.current, &settings.maxCurrent, &null,
            FONT_MEDIUM, 5, UNIT_CURRENT, NULL);

    container_create(&c, 128, 55);

    container_attach(&c, (widget_t *) &lBig, 0, 1);
    container_attach(&c, (widget_t *) &lSmall1, 0, 21);
    container_attach(&c, (widget_t *) &lSmall2, 46, 21);

    container_attach(&c, (widget_t *) &lOn, 92, 3);
    container_attach(&c, (widget_t *) &lOverride1, 92, 21);
    container_attach(&c, (widget_t *) &lOverride2, 92, 27);

    container_attach(&c, (widget_t *) &lMode, 0, 45);

    container_attach(&c, (widget_t *) &eSetting, 41, 43);
}

widget_t* mainMenu_getWidget(void) {
    return (widget_t*) &c;
}

GUISignal_t mainMenu_Input(GUISignal_t signal) {
    if (signal.clicked & HAL_BUTTON_ONOFF) {
        /* switch load on/off */
        load.powerOn = !load.powerOn;
        signal.clicked &= ~HAL_BUTTON_ONOFF;
    }
    if (signal.clicked
            & (HAL_BUTTON_CC | HAL_BUTTON_CP | HAL_BUTTON_CR | HAL_BUTTON_CV)) {
        /* switch to main screen */
        if(c.base.parent) {
            widget_deselectAll(c.base.parent->firstChild);
            widget_lostFocus(c.base.parent);
            notebook_t *n = (notebook_t*) c.base.parent;
            /* TODO this should not be hard-coded */
            n->selectedPage = 0;
        }
        c.base.flags.selected = 1;
        c.flags.editing = 1;
        c.flags.focussed = 1;
        widget_deselectAll(c.base.firstChild);
        eSetting.base.flags.selected = 1;
    }
    /* handle load mode change */
    loadMode_t newMode = load.mode;
    if (signal.clicked & HAL_BUTTON_CC) {
        newMode = FUNCTION_CC;
        signal.clicked &= ~HAL_BUTTON_CC;
    } else if (signal.clicked & HAL_BUTTON_CV) {
        newMode = FUNCTION_CV;
        signal.clicked &= ~HAL_BUTTON_CV;
    } else if (signal.clicked & HAL_BUTTON_CR) {
        newMode = FUNCTION_CR;
        signal.clicked &= ~HAL_BUTTON_CR;
    } else if (signal.clicked & HAL_BUTTON_CP) {
        newMode = FUNCTION_CP;
        signal.clicked &= ~HAL_BUTTON_CP;
    }
    if (newMode != load.mode) {
        /* switched to different mode */
        load.mode = newMode;
        /* safety: switch load off */
        load.powerOn = 0;
    }
    return signal;
}

void mainMenu_updateWidgets(void) {
    /* set on/off label */
    if (load.powerOn) {
        label_SetText(&lOn, "ON");
    } else {
        label_SetText(&lOn, "OFF");
    }
    /* set mode label */
    switch (load.mode) {
    case FUNCTION_CC:
        label_SetText(&lMode, "CC:");
        lBig.unit = UNIT_CURRENT;
        lBig.value = &load.state.current;
        lSmall1.unit = UNIT_VOLTAGE;
        lSmall1.value = &load.state.voltage;
        lSmall2.unit = UNIT_POWER;
        lSmall2.value = &load.state.power;
        eSetting.value = &load.current;
        eSetting.unit = UNIT_CURRENT;
        eSetting.max = &settings.maxCurrent;
        eSetting.min = &null;
        break;
    case FUNCTION_CV:
        label_SetText(&lMode, "CV:");
        lBig.unit = UNIT_VOLTAGE;
        lBig.value = &load.state.voltage;
        lSmall1.unit = UNIT_CURRENT;
        lSmall1.value = &load.state.current;
        lSmall2.unit = UNIT_POWER;
        lSmall2.value = &load.state.power;
        eSetting.value = &load.voltage;
        eSetting.unit = UNIT_VOLTAGE;
        eSetting.max = &settings.maxVoltage;
        eSetting.min = &settings.minVoltage;
        break;
    case FUNCTION_CR:
        label_SetText(&lMode, "CR:");
        lBig.unit = UNIT_CURRENT;
        lBig.value = &load.state.current;
        lSmall1.unit = UNIT_VOLTAGE;
        lSmall1.value = &load.state.voltage;
        lSmall2.unit = UNIT_POWER;
        lSmall2.value = &load.state.power;
        eSetting.value = &load.resistance;
        eSetting.unit = UNIT_RESISTANCE;
        eSetting.max = &settings.maxResistance;
        eSetting.min = &settings.minResistance;
        break;
    case FUNCTION_CP:
        label_SetText(&lMode, "CP:");
        lBig.unit = UNIT_POWER;
        lBig.value = &load.state.power;
        lSmall1.unit = UNIT_CURRENT;
        lSmall1.value = &load.state.current;
        lSmall2.unit = UNIT_VOLTAGE;
        lSmall2.value = &load.state.voltage;
        eSetting.value = &load.power;
        eSetting.unit = UNIT_POWER;
        eSetting.max = &settings.maxPower;
        eSetting.min = &null;
        break;
    }
    /* set override label */
    if(waveform.switchedOn) {
        label_SetText(&lOverride1, "WAVEFORM");
        label_SetText(&lOverride2, "RUNNING");
    } else if(arbitrary.state == ARB_RUNNING) {
        label_SetText(&lOverride1, "ARBITRARY");
        label_SetText(&lOverride2, "RUNNING");
    } else if(arbitrary.state== ARB_ARMED) {
        label_SetText(&lOverride1, "ARBITRARY");
        label_SetText(&lOverride2, "ARMED");
    } else {
        label_SetText(&lOverride1, "");
        label_SetText(&lOverride2, "");
    }
}
