#!/bin/bash -l 


geomlist(){ cat << EOL
nmskTailOuter
nmskTailInner
nmskSolidMask
EOL
}

for geom in $(geomlist) ; do  
   echo $BASH_SOURCE $geom 
   GEOM=$geom ./gxt.sh $*
   [ $? -ne 0 ] && echo $BASH_SOURCE gxt error for geom $geom && exit 1
done 

exit 0


