#!/bin/bash

if [ "$FONT" = "" ]
then
    FONT=/usr/share/fonts/truetype/ubuntu-font-family/UbuntuMono-R.ttf
fi

echo "font : $FONT"

for i in *\.diag
do
    PNG=$(echo ${i} | sed "s/diag$/pdf/")

    if [ ! -f ${PNG} ] || [ $(stat -c "%X" ${PNG}) -lt  $(stat -c "%X" ${i}) ]
    then

        echo seqdiag ${i}
        seqdiag -Tpdf -a -f $FONT ${i}

        echo touch ${PNG}
        touch ${PNG}
    fi
done

PKTDIAG=pktdiag
for i in *\.$PKTDIAG
do
    PNG=$(echo ${i} | sed "s/$PKTDIAG$/pdf/")

    if [ ! -f ${PNG} ] || [ $(stat -c "%X" ${PNG}) -lt  $(stat -c "%X" ${i}) ]
    then

        echo seqdiag ${i}
        packetdiag -Tpdf -a -f $FONT ${i}

        echo touch ${PNG}
        touch ${PNG}
    fi
done

# GDOT="gviz-dot"
# for i in *\.$GDOT
# do
#     PDF=$(echo ${i} | sed "s/$GDOT$/pdf/")
# 
#     if [ ! -f ${PDF} ] || [ $(stat -c "%X" ${PDF}) -lt  $(stat -c "%X" ${i}) ]
#     then
# 
#         echo dot ${i}
#         dot -Tpdf ${i} > ${PDF}
# 
#         echo touch ${PDF}
#         touch ${PDF}
#     fi
# done
