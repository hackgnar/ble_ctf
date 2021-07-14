FROM espressif/idf as dev
RUN mkdir /ble_ctf
COPY ./main /ble_ctf/main
COPY ./Makefile /ble_ctf/
COPY ./sdkconfig.example /ble_ctf/sdkconfig
ENV IDF_PATH=/opt/esp/idf
ENV IDF_TOOLS_PATH=/opt/esp
RUN . $IDF_PATH/export.sh && cd /ble_ctf && make
CMD ["bash"]
