ret=$(ping www.google.com)
if [[ $ret -lt 1 ]]; then
   echo "Internet not available, please use iwctl to connect to the network." 
else
    pacman -S gcc
    gcc -o base-install base-install.c
    ./base-install
fi
