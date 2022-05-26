#!/bin/bash

APP_NAME="modeditor"
APP_DIRECTORY="$HOME/.local/share/applications/modengine/modeditor"
DESKTOP_FILE_PATH="$APP_DIRECTORY/modeditor.desktop"
RUN_FILE_PATH="$APP_DIRECTORY/run.sh"

# This value needs to be defined in the main game packaging code
MODENGINE_BINARY="$HOME/gamedev/mosttrusted/ModEngine"

if [ ! -d "$APP_DIRECTORY" ]; then 
  mkdir -p "$APP_DIRECTORY"
fi 

DESKTOP_TEMPLATE_VALUE=$(cat "./modeditor/app.desktop")
DESKTOP_FILE=$(echo -e "$DESKTOP_TEMPLATE_VALUE" | sed "s|{{APP_DIRECTORY}}|$APP_DIRECTORY|g")

RUN_TEMPLATE_FILE=$(cat "./modeditor/run.sh")
RUN_FILE=$(echo -e "$RUN_TEMPLATE_FILE" | sed "s|{{MODENGINE_BINARY}}|$MODENGINE_BINARY|g")

cp "./modeditor/icon.png" "$APP_DIRECTORY"
echo -e "$DESKTOP_FILE" >  "$DESKTOP_FILE_PATH" 
echo -e "$RUN_FILE" > "$RUN_FILE_PATH"
chmod 777 "$RUN_FILE_PATH"

# xdg-desktop-menu forceupdate --> do I need this?1