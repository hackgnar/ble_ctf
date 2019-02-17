/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Attributes State Machine */
enum
{
    FLAG_SCOREBOARD_IDX_SVC,

    FLAG_SCOREBOARD_IDX_CHAR_READ_DOCS,
    FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_DOCS,

    FLAG_SCOREBOARD_IDX_CHAR_READ_FLAGS_COMPLETE,
    FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_FLAGS_COMPLETE,

    FLAG_SCOREBOARD_IDX_CHAR_READ_WRITE_SUBMIT,
    FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_WRITE_SUBMIT,

    FLAG_SCOREBOARD_IDX_CHAR_READ_WRITE_WARP,
    FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_WRITE_WARP,
    
    FLAG_SCOREBOARD_IDX_CHAR_READ_WRITE_RESET,
    FLAG_SCOREBOARD_IDX_CHAR_VAL_READ_WRITE_RESET,
    
    //CODEGEN_HEADER_FLAG_IDX

    FLAG_SCOREBOARD_IDX_NB,
};
void app_main();
