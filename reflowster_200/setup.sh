#!/bin/bash

librariesDirectory="${HOME}/Arduino/libraries"
read -e -i "$librariesDirectory" -p "Arduino Libraries Directory: " input
librariesDirectory="${input:-$librariesDirectory}"

function main() {
    for liburl in `cat required_libraries.txt`
    do
        echo "Library URL: $liburl"
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
        cd $libraries
        git clone $1
    fi
}

function downloadZipLibrary() {
    libname=${1##*/}
    libname=${libname%.zip}
    echo "libname: $libname"
}

main
