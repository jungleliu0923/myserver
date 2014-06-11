#!/bin/sh 
INSTALL_PATH="../"
MY_LOG_PATH="$INSTALL_PATH/mylog"
MY_CONF_PATH="$INSTALL_PATH/myconf"
MY_SERVER_PATH="$INSTALL_PATH/myserver"
MY_LOG_URL="https://github.com/jungleliu0923/mylog"
MY_CONF_URL="https://github.com/jungleliu0923/myconf"

if [ -d "$MY_LOG_PATH" ]; then
	rm -rf "$MY_LOG_PATH"
fi

if [ -d "$MY_CONF_PATH" ]; then
	rm -rf "$MY_CONF_PATH"
fi

env GIT_SSL_NO_VERIFY=true  git clone "$MY_LOG_URL"  "$MY_LOG_PATH"
env GIT_SSL_NO_VERIFY=true  git clone "$MY_CONF_URL" "$MY_CONF_PATH"

cd "$MY_LOG_PATH" && make
cd  "$MY_CONF_PATH" && make
make
make -C sample

