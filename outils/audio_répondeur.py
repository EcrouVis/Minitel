from scipy import signal,io
import numpy as np

#paramètres d'entrée
rom="../m12/dump/dump_Bz6_p3.bin"
start=0x01D1
stop=0x9542
fosc=14745600
cpu_ppcycle=12

#prendre en compte les erreurs de timming ajoute une fondamentale à 5585Hz et des harmoniques
cycle_unique=True#pour ne pas prendre en compte les erreurs de timming
cycle_1=30#quand lecture de l'octet
cycle_2=27#quand on ne vient pas lire d'octet / cas général
cycle_3=28#quand on atteint le 6ème bit, envoi d'un signal / erreur de timming dans le code?


#bande audio téléphonie
fc=[300,3400]

#fréquence d'échantillonage
fech=fosc/cpu_ppcycle
if cycle_unique:
    fech=fech*8/(cycle_1+cycle_3+6*cycle_2)
fech_out=44100


f=open(rom,"rb")
f.seek(start)
audio=[]
for i in range(stop-start):
    v=int.from_bytes(f.read(1))
    for j in range(8):
        
        if cycle_unique:
            audio.append((v>>7)*2-1)
        else:
            n=cycle_2
            if j==7:
                n=cycle_1
            if j==5:
                n=cycle_3
            audio+=[(v>>7)*2-1]*cycle_2
        
        v=(v<<1)&0xFF
f.close()

#audio non filtré
audio_nf=signal.resample(audio,int(len(audio)*fech_out/fech))
audio_nf=audio_nf/np.max(np.abs(audio_nf))
io.wavfile.write("../m12/dump/répondeur_non_filtré.wav",fech_out,audio_nf)

#audio filtré émulant la bande passante de la ligne téléphonique
sos=signal.cheby2(16,60,fc,btype="bandpass",output="sos",fs=fech)
audio=signal.sosfiltfilt(sos,audio)
audio=signal.resample(audio,int(len(audio)*fech_out/fech))
audio=audio/np.max(np.abs(audio))
io.wavfile.write("../m12/dump/répondeur.wav",fech_out,audio)
