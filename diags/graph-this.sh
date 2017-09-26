#!/bin/bash

if [ "$FONT" = "" ]
then
    FONT=/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf
fi

echo "font : $FONT"

# $1 = program, $2 = filename extension
function graphit()
{
    PROG=$1
    FNEXT=$2
    ls *.$FNEXT 2>/dev/null 1>&2
    if [ $? -eq 0 ]; then
        for i in *\.$FNEXT
        do
            PDF=$(echo ${i} | sed "s/$FNEXT$/pdf/")
            if [ ! -f ${PDF} ] || [ $(stat -c "%X" ${PDF}) -lt  $(stat -c "%X" ${i}) ]
            then

                PROGOPT="-Tpdf"
                case $PROG in
                    "seqdiag" | "packetdiag" | "nwdiag")
                        PROGOPT="$PROGOPT -a -f $FONT"
                        echo ${PROG} ${PROGOPT} ${i}
                        ${PROG} ${PROGOPT} ${i}
                        ;;
                    "dot")
                        echo "${PROG} ${PROGOPT} ${i} > ${PDF}"
                        ${PROG} ${PROGOPT} ${i} > ${PDF}
                        ;;
                esac

                echo touch ${PDF}
                touch ${PDF}
            fi
        done
    fi
}

graphit "seqdiag" "diag"
graphit "packetdiag" "pktdiag"
graphit "nwdiag" "nwdiag"
graphit "dot" "gviz-dot"
