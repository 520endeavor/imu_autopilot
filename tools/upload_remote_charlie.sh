echo ***COPY build/obj/main.elf
scp ./build/obj/main.elf pixhawk@192.168.1.82:/home/pixhawk/imu_firmware/


echo ***RESET IMU mavconn-messenger -r
ssh pixhawk@192.168.1.82 mavconn-messenger -r

echo ***UPLOAD imu_firmware/main.elf 
ssh pixhawk@192.168.1.82 imu_firmware/lpc21iap imu_firmware/main.elf

# 	TO Copy your rsa key to the onboard computer execute the following
#	ssh-copy-id -i ~/.ssh/id_rsa.pub pixhawk@192.168.1.82
#	from now on you can upload whithout needing a password

