import wave
import scipy
import numpy as np

mi_5=1318.510
solD_5=1661.218
réD_4=622.253
mi_4=659.255
solD_4=830.609
si_4=987.766
fa_4=698.456
sol_4=783.991
laD_4=932.327
doD_5=1108.730
do_5=1046.502



fech=48000

ratio=3/4
t_exp=0.06
b_exp=t_exp*fech

t=1.5
n=int(t*fech)

def build_wave(freq,fech,n_opti,ratio):
    freq=fech/np.round(fech/freq)#fréquence la plus proche de ce qui est voulu
    n_f_step=fech/freq
    n_opti=np.round(n_f_step*np.round(n_opti/n_f_step))#temps le plus proche de ce qui est voulu
    w=(np.arange(n_opti)*freq)%fech
    w=w<fech*ratio
    return w.astype(float)*2-1


#1
fl=mi_5#1319.25
fh=solD_5#1656
rep=8
wl=build_wave(fl,fech,fech/12,ratio)
wh=build_wave(fh,fech,fech/12,ratio)
v=np.tile(np.concatenate([wl,wh]),rep)
v=(127*v*(1-np.exp(-np.arange(len(v))/b_exp))+127).astype(np.uint8)
v=np.concatenate([v,np.ones(n-len(v),np.uint8)*127])
with wave.open("sonnerie_1_recréation.wav", mode="wb") as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(1)
    wav_file.setframerate(fech)
    wav_file.writeframes(v)

#2
f=réD_4#622.685
v=build_wave(f,fech,16*fech/12,ratio)
v=(255*v*(1-np.exp(-np.arange(len(v))/b_exp))+127.5*np.exp(-np.arange(len(v))/b_exp)).astype(np.uint8)
v=np.concatenate([v,np.ones(n-len(v),np.uint8)*127])
with wave.open("sonnerie_2_recréation.wav", mode="wb") as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(1)
    wav_file.setframerate(fech)
    wav_file.writeframes(v)

#3
fl=mi_4#659.63
fh=solD_4#832.45
rep=8
wl=build_wave(fl,fech,fech/12,ratio)
wh=build_wave(fh,fech,fech/12,ratio)
v=np.tile(np.concatenate([wl,wh]),rep)
v=(255*v*(1-np.exp(-np.arange(len(v))/b_exp))+127.5*np.exp(-np.arange(len(v))/b_exp)).astype(np.uint8)
v=np.concatenate([v,np.ones(n-len(v),np.uint8)*127])
with wave.open("sonnerie_3_recréation.wav", mode="wb") as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(1)
    wav_file.setframerate(fech)
    wav_file.writeframes(v)

#4
fh=si_4#985
fm=solD_4#832.45
fl=fa_4#698.05
rep=4
wh=build_wave(fh,fech,fech/12,ratio)
wm=build_wave(fm,fech,2*fech/12,ratio)
wl=build_wave(fl,fech,fech/12,ratio)
v=np.tile(np.concatenate([wh,wm,wl]),rep)
v=(255*v*(1-np.exp(-np.arange(len(v))/b_exp))+127.5*np.exp(-np.arange(len(v))/b_exp)).astype(np.uint8)
v=np.concatenate([v,np.ones(n-len(v),np.uint8)*127])
with wave.open("sonnerie_4_recréation.wav", mode="wb") as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(1)
    wav_file.setframerate(fech)
    wav_file.writeframes(v)

#5
f1=réD_4#622.77
f2=sol_4#782.238
f3=laD_4#932.15
f4=doD_5#1111.93
f5=do_5#1044.86
f6=fa_4#698.126
f7=solD_4#832.485
w1=build_wave(f1,fech,2*fech/12,ratio)
w2=build_wave(f2,fech,fech/12,ratio)
w3=build_wave(f3,fech,fech/12,ratio)
w4=build_wave(f4,fech,3*fech/12,ratio)
w5=build_wave(f5,fech,3*fech/12,ratio)
w6=build_wave(f6,fech,fech/12,ratio)
w7=build_wave(f7,fech,fech/12,ratio)
v=np.concatenate([w1,w2,w3,w4,w3,w5,w6,w7,w6,w7,w6])
v=(255*v*(1-np.exp(-np.arange(len(v))/b_exp))+127.5*np.exp(-np.arange(len(v))/b_exp)).astype(np.uint8)
v=np.concatenate([v,np.ones(n-len(v),np.uint8)*127])
with wave.open("sonnerie_5_recréation.wav", mode="wb") as wav_file:
    wav_file.setnchannels(1)
    wav_file.setsampwidth(1)
    wav_file.setframerate(fech)
    wav_file.writeframes(v)
