#ifndef __PATTERN_H_
#define __PATTERN_H_

enum StorageType
{
    EXTERNAL_SD = 1, /* external sdcard */
    INTERNAL_SD = 2, /* internal sdcard */
};

enum ComparePattern
{
    POSITION_PATTERN = 1,
    DIRTY_WHITE_PATTERN = 2,
    DIRTY_BLACK_PATTERN = 3,
    COLOR_WHITE_PATTERN = 4,
    COLOR_BLACK_PATTERN = 5,
};

enum DisplayPattern
{
    RED_PATTERN = 1,
    BLUE_PATTERN = 2,
    GREEN_PATTERN = 3,
    BLACK_PATTERN = 4,
    WHITE_PATTERN = 5,
    BLACK_WHITE_PATTERN = 6,
    FRAME_PATTERN = 7,
};

#endif
