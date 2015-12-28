#!/bin/bash

if [ "x$1" = "xdebug" ]; then 
    CONFIGURE_OPTS="--enable-jam-debug"
fi

export CFLAGS="-Wall -g"

phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd storage/files && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../snmp && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../tokyo && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../stomp && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../email && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../spread && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../zeromq2 && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install && \
cd ../skeleton && \
phpize && ./configure ${CONFIGURE_OPTS} && make clean && make && make install
