from scipy import signal,io

#paramètres d'entrée
rom="../m12/dump/dump_Bz6_p3.bin"
start=0x01D1
stop=0x9542
fech=14745600/324

#bande audio téléphonie
fc=[300,3400]

#fréquence d'échantillonage de sortie
fech2=44100


f=open(rom,"rb")
f.seek(start)
d=[]
for i in range(stop-start):
    v=int.from_bytes(f.read(1))
    for j in range(8):
        d.append((v>>7)*2-1)
        v=(v<<1)&0xFF
f.close()

sos=signal.cheby2(16,60,fc,btype="bandpass",output="sos",fs=fech)

d=signal.sosfiltfilt(sos,d)
d=signal.resample(d,int(len(d)*fech2/fech))

io.wavfile.write("../m12/dump/répondeur.wav",fech2,d)
