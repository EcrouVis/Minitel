import ffmpeg
import numpy as np
from scipy import signal
import json
import sys
from os import path



def conv_filter(dt,fech,freq,window=lambda x:1):
    n=np.ceil(dt*fech)
    w=window(np.arange(n)/(n-1))
    w=w*np.exp(1j*2*np.pi*freq*np.arange(n)/fech)
    return w/n
flat_top=lambda x:0.21557895-0.41663158*np.cos(2*np.pi*x)+0.277263158*np.cos(4*np.pi*x)-0.083578947*np.cos(6*np.pi*x)+0.006947368*np.cos(8*np.pi*x)
hann=lambda x:np.sin(np.pi*x)**2
hamming=lambda x:0.53836-0.46164*np.cos(2*np.pi*x)#semble être la meilleure solution

def compute_transitions(data,bps,fmod,fech):
    cv1=conv_filter(1/bps,fech,fmod[0],window=hamming)
    cv2=conv_filter(1/bps,fech,fmod[1],window=hamming)
    s1=np.abs(signal.convolve(data,cv1,mode="same"))
    s2=np.abs(signal.convolve(data,cv2,mode="same"))

    n=int(15*fech/bps)#domaine de validité des transitions (distance de groupes de points>somme des écarts types des groupes)
    k=1#68%:1 95%:2 ! les transitions peuvent causer des faux négatifs si k trop grand
    g1,g2=np.sort([s1,s2],axis=0)
    cvm=np.ones(n)/n
    g1m=signal.convolve(g1,cvm,mode="same")#moyennes
    g2m=signal.convolve(g2,cvm,mode="same")
    g1s=signal.convolve(g1**2,cvm,mode="same")
    g2s=signal.convolve(g2**2,cvm,mode="same")
    g1s=np.sqrt(g1s-g1m**2)#écarts types
    g2s=np.sqrt(g2s-g2m**2)
    d=g2m-g1m
    l=(g1s+g2s)*k
    
    return s1>s2,d>l

def decode_transitions(tr,v,dt):
    i=1
    data=[]
    p=0b0110100110010110
    tr=np.concatenate([tr,np.ones(int(np.ceil(10*dt)))])#pour éviter un crash quand l'audio se termine avant la fin d'un message
    while i<len(tr):
        if tr[i-1] and not tr[i] and v[i]:
            i2=i+dt/2#!dt non entier
            if not tr[int(i2)]:
                a=0
                for j in range(8):
                    i2+=dt
                    if tr[int(i2)]:
                        a+=(1<<j)
                i2+=dt
                d={"time":i}
                d["byte"]=a&0x7F
                i=int(i2)
                d["framing error"]=not tr[i]
                d["parity error"]=bool(((p>>(a&0x0F))^(p>>(a>>4)))&0x01)
                data.append(d)
        i+=1
    return data



for i in range(1,len(sys.argv)):
    
    audio_file=sys.argv[i]
    
    if not path.isfile(audio_file):
        print(audio_file,"n'existe pas")
    else:
        probe = ffmpeg.probe(audio_file)
        audio_info=next((stream for stream in probe['streams'] if stream['codec_type'] == 'audio'), None)
        freq=audio_info["sample_rate"]

        audio, _ = (ffmpeg
            .input(audio_file)
            .output('-', format='f64le', acodec='pcm_f64le', ac=1, ar=freq)
            .overwrite_output()
            .run(capture_stdout=True)
        )

        freq=float(freq)
        audio=np.frombuffer(audio,np.float64)



        ch1bps=75
        ch1f=[390,450]
        dt1=freq/ch1bps

        ch2bps=1200
        ch2f=[1300,2100]
        dt2=freq/ch2bps

        tr1,v1=compute_transitions(audio,ch1bps,ch1f,freq)
        data1=decode_transitions(tr1,v1,dt1)
        tr2,v2=compute_transitions(audio,ch2bps,ch2f,freq)
        data2=decode_transitions(tr2,v2,dt2)

        fo=audio_file[::-1].split(".",1)[-1][::-1]+".json"
        f=open(fo,"w")
        f.write(json.dumps({"sample rate":freq,"75bps":data1,"1200bps":data2}))
        f.close()
        print("Résultat enregistré dans",fo)
