rpi: gpsupdate.c file_loc.c
	arm-linux-gnueabi-gcc gpsupdate.c -o gpsupdate && arm-linux-gnueabi-gcc file_loc.c -o file_loc
user: gpsupdate.c file_loc.c
	gcc gpsupdate.c -o gpsupdate_user && gcc file_loc.c -o file_loc_user
all: gpsupdate.c file_loc.c
	arm-linux-gnueabi-gcc gpsupdate.c -o gpsupdate && arm-linux-gnueabi-gcc file_loc.c -o file_loc && gcc gpsupdate.c -o gpsupdate_user && gcc file_loc.c -o file_loc_user
