# Project-TA-2025-Keamanan-Transmisi-Data-pada-Sistem-IoT
 <img width="3227" height="2802" alt="gambar-rancangan-sistem-SEQUENCE-DIAGRAM-keamanan-pada-perangkat-IoT drawio" src="https://github.com/user-attachments/assets/cddad807-cb08-41dc-a7b1-f7943751fd6c" />
Rancangan sistem pada gambar diatas menjelaskan, proses terjadinya transmisi data diantara publisher, broker, dan subscriber. Dimana publisher mengirimkan data temperature, humidity yang telah dienkripsi, serta data checksum SHA-256 ke broker, dan broker meneruskan data tersebut ke subscriber. Setiap data yang diterima oleh subscriber akan melalui proses validasi, metode Checksum SHA-256 akan berperan melakukan validasi dengan melakukan perbandingan diantara Data Checksum yang diterima oleh subscriber melalui publisher, dengan Data Checksum yang dihasilkan melalui proses perhitungan ulang di subscriber. Hasil dari proses validasi akan menghasilkan dua output, apabila terdapat perbedaan diantara kedua data checksum tersebut, maka hasil validasi dinyatakan Data Manipulated. Proses dekripsi untuk mendapatkan data plaintext dari data yang dienkripsi tersebut tidak dilakukan. Dan Subscriber akan melakukan request pengiriman ulang data ke publisher untuk mendapatkan data asli yang tidak dimanipulasi. Lalu jika hasil validasi menyatakan kedua data checksum tersebut memiliki nilai yang sama, maka proses dekripsi akan dilakukan melalui metode AES-128 untuk mendapatkan data plaintext temperature dan humidity serta menginformasikan ke subscriber bahwa data yang diterima valid.

# Instalasi Program Publisher dan Subscriber
1. untuk program publisher dapat menggunakan software arduino IDE, download file program_publisher_arduino_IDE dan jalankan di software arduino IDE. diperlukan konfigurasi tambahan seperti install board manager, library manager, serta menentukan port, example: COM3 .
2. untuk broker yang digunakan HiveMQ, ataupun dapat menggunakan broker lain sesuai keinginan.
3. untuk program Subscriber dapat menggunakan software node-red, download file flows.json dan import file tersebut pada software node-red.
 
# Tahapan proses simulasi MITM attack menggunakan tools MITM Proxy pada sistem keamanan yang dibuat
Sebelum memulai tahapan ini, perangkat manapun yang ditargetkan diharuskan telah terinstall certificate mitmproxy-ca-cert.pem .
- MITM attack memiliki beberapa teknik serangan, salah satunya yang digunakan pada simulasi serangan ini adalah ARP Spoofing melalui perintah berikut:
   1. sudo sysctl net.ipv4.ip_forward=1
   2. sudo sysctl -w net.ipv4.conf.all.send_redirects=0
   3. sudo arpspoof -i eth0 -r -t [isi-IP-Target] [isi-IP-Router]
      
- setelah proses ARP Spoofing berhasil dijalankan, selanjutnya dilakukan perintah berikut:
   1. sudo iptables -t nat -A PREROUTING -i eth0 -p tcp --dport [isi-Port-target] -j REDIRECT --to-port 8081
   2. mitmproxy --mode transparent --showhost -p 8081 -k --tcp-hosts ".*"
- Reference related to use MITM tools: https://github.com/nmatt0/mitmtools
  
- Setelahnya akan diarahkan ke flows panel pada tools MITM Proxy, didalam flows panel tersebut akan terdapat isi komunikasi yang dilakukan oleh target. dan untuk melakukan intercept pada salah satu data komunikasi diperlukan menggunakan perintah: ~b [isi-format-topik-data-target] .
<img width="462" height="101" alt="image" src="https://github.com/user-attachments/assets/614ecd35-73fc-4b74-b1c0-8834673a2fab" />


# Hasil Dokumentasi
# Dokumentasi proses simulasi serangan Data Tampering [Data Intercepted]
<img width="670" height="48" alt="image" src="https://github.com/user-attachments/assets/22488fbc-4126-49fd-834d-adb71e537d63" />

# Dokumentasi proses simulasi serangan Data Tampering [Data Manipulated]
<img width="677" height="59" alt="image" src="https://github.com/user-attachments/assets/5b8db702-6468-45e9-9408-62bb78835a91" />

# Dokumentasi proses yang terjadi setelah Data dimanipulasi [UI dashboard Subscriber]
<img width="702" height="339" alt="image" src="https://github.com/user-attachments/assets/9ecc8818-f55d-406b-9144-2fad84da8717" />

# Dokumentasi subscriber melakukan request pengiriman ulang data ke publisher [Log Data Subscriber]
<img width="214" height="478" alt="image" src="https://github.com/user-attachments/assets/9532a5b0-e02f-4d2d-8dd4-92c181834672" />

# Dokumentasi publisher mengerimkan ulang data yang direquest oleh subsrcriber [Log Serial Monitor Publisher]
<img width="699" height="161" alt="image" src="https://github.com/user-attachments/assets/248b5ff7-1115-47d2-9157-29b4c11a512f" />




