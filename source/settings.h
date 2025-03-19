#ifndef RRC_SETTINGS_H
#define RRC_SETTINGS_H

enum rrc_settings_result
{
    RRC_SETTINGS_LAUNCH = 0,
    RRC_SETTINGS_EXIT = 1
};

enum rrc_settings_result rrc_settings_display();

#endif
