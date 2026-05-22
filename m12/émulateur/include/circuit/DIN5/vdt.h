#ifndef VDT_H
#define VDT_H

static const unsigned char clear_bulk[]={0x0C};//does not clear the service row / move cursor to 00 01
static const unsigned char bell[]={0x07};
static const unsigned char enable_cursor[]={0x11};
static const unsigned char disable_cursor[]={0x14};
static const unsigned char use_G0[]={0x0F};
static const unsigned char use_G1[]={0x0E};
static const unsigned char line_separator_40c[]={0x60,0x12,0x67};
static const unsigned char foreground_color_magenta[]={0x1B,0x45};
static const unsigned char background_color_magenta[]={0x1B,0x55};
static const unsigned char foreground_color_black[]={0x1B,0x40};
static const unsigned char background_color_black[]={0x1B,0x50};
static const unsigned char foreground_color_white[]={0x1B,0x47};
static const unsigned char background_color_white[]={0x1B,0x57};
static const unsigned char swap_color[]={0x1B,0x5D};
static const unsigned char revert_swap_color[]={0x1B,0x5C};

static const unsigned char disable_local_echo[]={0x1B,0x3B,0x60,0x5A,0x51};//automatically reenabled when PT 0->1
static const unsigned char enable_local_echo[]={0x1B,0x3B,0x61,0x5A,0x51};
static const unsigned char reset_minitel[]={0x1B,0x39,0x7F};
static const unsigned char enable_mixed_mode[]={0x1B,0x3A,0x32,0x7D};
static const unsigned char disable_mixed_mode[]={0x1B,0x3A,0x32,0x7E};
static const unsigned char request_working_status[]={0x1B,0x39,0x72};
static const unsigned char screen_roll[]={0x1B,0x3A,0x69,0x43};
static const unsigned char screen_page[]={0x1B,0x3A,0x6A,0x43};
static const unsigned char default_lower_case[]={0x1B,0x3A,0x69,0x45};
static const unsigned char default_upper_case[]={0x1B,0x3A,0x6A,0x45};
static const unsigned char request_keyboard_status[]={0x1B,0x3A,0x72,0x59};
static const unsigned char extended_keyboard[]={0x1B,0x3B,0x69,0x59,0x41};
static const unsigned char simple_keyboard[]={0x1B,0x3B,0x6A,0x59,0x41};

#endif /* CONNEXION_WEBSOCKET_VDT_H */
