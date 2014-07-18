#!/bin/bash


arduinoPreferences="${HOME}/.arduino/preferences.txt"

if [ ! -e "$arduinoPreferences" ]
then
    echo "Arduino preferences file not found: $arduinoPreferences"
    exit
fi

sketchbookPath=`grep sketchbook.path $arduinoPreferences`
sketchbookPath=${sketchbookPath##*=}

librariesDirectory="$sketchbookPath/libraries"
read -e -i "$librariesDirectory" -p "Arduino Libraries Directory: " input
librariesDirectory="${input:-$librariesDirectory}"

function main() {
    for liburl in `cat required_libraries.txt`
    do
        if [[ $liburl == *github* ]]
        then
            downloadGithubLibrary $liburl
        elif [[ $liburl == *zip ]]
        then
            downloadZipLibrary $liburl
        fi
    done
}

function downloadGithubLibrary() {
    libname=${1##*/}
    libname=${libname%.git}
    if [ -e "$librariesDirectory/$libname" ]
    then
        echo "Library exists, skipping: $libname"
    else
        cd $librariesDirectory
        git clone $1
    fi
}

function downloadZipLibrary() {
    filename=${1##*/}
    libname=${filename%.zip}
    if [ -e "$librariesDirectory/$libname" ]
    then
        echo "Library exists, skipping: $libname"
    else
        cd /tmp
        wget $1
        unzip $filename -d $librariesDirectory/
    fi
}

main
