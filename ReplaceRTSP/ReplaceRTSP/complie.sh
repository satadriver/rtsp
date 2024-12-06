local_dir=${PWD}

tempname="monolith_ljg"

rm -f ${tempname}
rm -f monolith.h
rm -f tgtstart

cd rtsp
make clean
make
# arm-linux-gnueabihf-g++ -mthumb -std=c++11 --static -o send_hex_data doRTSP.cpp -lpthread

cd ${local_dir}



# tempname="send_hex_data"
cp ./rtsp/${tempname} .
# cp ../pcap/${tempname} .
xxd -i ${tempname} > monolith.h

sed -i "s/${tempname}_len/monolith_size/g" monolith.h
sed -i "s/${tempname}/monolith_file/g" monolith.h

arm-linux-gnueabihf-gcc -mthumb --static -o tgtstart replce.c
