#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := gatt_server_service_table_demo

COMPONENT_ADD_INCLUDEDIRS := components/include

include $(IDF_PATH)/make/project.mk

codegen:
	rm -f $(PROJECT_PATH)/main/*c
	rm -f $(PROJECT_PATH)/main/*h
	rm -rf $(PROJECT_PATH)/build
	python code_gen/code_gen.py $(PROJECT_PATH)
