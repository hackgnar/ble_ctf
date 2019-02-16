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
    TEMPLATE_TABLE_IDX_SVC,

    TEMPLATE_TABLE_IDX_CHAR_READ_SAMPLE,
    TEMPLATE_TABLE_IDX_CHAR_VAL_READ_SAMPLE,

    //todo: HRS in here is a cut/paste job... get rid of it 
    TEMPLATE_TABLE_IDX_NB,
};
//TODO generate flag name
void app_main();
