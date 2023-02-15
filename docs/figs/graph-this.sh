#!/bin/bash

for i in *.fig
do
	# latin1 accents
	grep -E "[ôéèàîÉê]" $i 2>/dev/null 1>/dev/null
	if [ $? -eq 0 ]; then
		echo $i matches a latin1 accent
		sed -r -i "s/É/\\\311/g" $i
		sed -r -i "s/à/\\\340/g" $i
		sed -r -i "s/è/\\\350/g" $i
		sed -r -i "s/é/\\\351/g" $i
		sed -r -i "s/ê/\\\352/g" $i
		sed -r -i "s/î/\\\356/g" $i
		sed -r -i "s/ô/\\\364/g" $i
		# sed -r -i "s/°/\\\176/g" $i
	fi

    PDF=$(echo ${i} | sed "s/fig$/pdf/")

    if [ ! -f ${PDF} ] || [ $(stat -c "%X" ${PDF}) -lt  $(stat -c "%X" ${i}) ]
    then

        echo "fig2ps ${i}"
        fig2dev -L pdf ${i} > ${PDF}

        echo "touch ${PDF}"
        touch ${PDF}

        # echo "make and touch ${PDF}"
		# pdf2ps ${PDF}
        # touch ${PDF}
    fi
done
