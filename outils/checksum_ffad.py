#recréation du code trouvé à l'addresse 0xFFAD
#checksum de la rom

checksum=0

for i in range(4):
    fi=open("../m12/dump/dump_Bz6_p"+str(i+1)+".bin","rb")
    content=fi.read()
    fi.close()
    for c in content:
        checksum+=c

checksum&=0xFFFF
checksum=hex(checksum)[2:].upper()
print(checksum)
