#!/bin/bash
name_upper=$(echo $1 | tr a-z A-Z)
cp -r TEMPLATE_table $1
mv $1/main/TEMPLATE_table.c $1/main/$1.c
mv $1/main/TEMPLATE_table.h $1/main/$1.h
sed -i -e "s/TEMPLATE_table.c/$1.c/g" $1/main/CMakeLists.txt 
sed -i -e "s/TEMPLATE_table.h/$1.h/g" $1/main/$1.c
sed -i -e "s/TEMPLATE_TABLE/$name_upper/g" $1/main/$1.c
sed -i -e "s/flag_TEMPLATE_table_value/flag_$1_value/g" $1/main/$1.c
sed -i -e "s/TEMPLATE_TABLE/$name_upper/g" $1/main/$1.h
