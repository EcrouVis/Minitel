Parity=0b0110100110010110

def setParity(b):
    return (b^((Parity>>(b&0x0F))^(Parity>>(b>>4)))<<7)&0xFF

def checkParity(b):
    return ((Parity>>(b&0x0F))^(Parity>>(b>>4)))&0x01==0

def updateCRC7(d,R):
    R=(R<<1)^d
    for i in range(8):
        if R&0x80==0x80:
            R=(0x12^(R<<1))&0xFF
        else:
            R=(R<<1)&0xFF
    return R>>1

def singleBitErrorFinderCRC7(length,R):
    for i in reversed(range(length)):
        for j in range(8):
            if R==0x09:
                return (i,j)
            if R&0x01==0x01:
                R=((R^0x89)>>1)
            else:
                R=R>>1
    return (-1,0)

def PCESend(data_in):#exemple simple pour montrer comment calculer le bloc / ne prends pas en compte les cas spéciaux tel que PRO1 RET
    data_out=[]
    if (len(data_in)%15!=0):#data_in doit être d'une longueur multiple de 15
        for i in range(15-(len(data_in)%15)):
            data_in+=[0]
    CRCR=0
    for i in range(len(data_in)):
        d=setParity(data_in[i])
        CRCR=updateCRC7(d,CRCR)
        data_out+=[d]
        if (i%15)==14:
            d=setParity(CRCR)
            data_out+=[d,0]
            CRCR=0
    return data_out

def PCEReceive(data_in):#exemple simple pour montrer comment gérer les erreurs dans un bloc / ne prends pas en compte les cas spéciaux
    CRCR=0
    parity_error=[False]*16
    data_out=[]
    for i in range(len(data_in)):
        if (i%17==16):
            if (data_in[i]!=0):
                print("Octet "+str(i)+" n'est pas nul dans le bloc "+str(i//17))
                break
            else:
                for j in range(15):
                    data_out+=[data_in[i-16+j]&0x7F]
        elif (i%17==15):
            if (not checkParity(data_in[i])):
                parity_error[15]=True
                print("Erreur de parité à l'octet "+str(i))
            if CRCR!=data_in[i]&0x7F:
                print("Le code CRC n'est pas bon à l'octet "+str(i)+": calculé="+hex(CRCR)+" reçu="+hex(data_in[i]&0x7F))
            if sum(parity_error)==0:
                if CRCR!=data_in[i]&0x7F:
                    print("Pas d'erreurs de parité détectées mais le CRC n'est pas bon")
                    print("Trop d'erreurs dans le bloc "+str(i//17))
                    break
                else:
                    print("Pas d'erreurs dans le bloc "+str(i//17))
            elif sum(parity_error)>1:
                print("Trop d'erreurs de parité dans le bloc "+str(i//17))
                break
            else:
                if parity_error[15]:
                    if (data_in[i]&0x7F)^CRCR in [0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80]:
                        print("Erreur dans le CRC détecté")
                        print("Données récupérées")
                        data_in[i]=setParity(CRCR)
                    else:
                        print("CRC trop différent")
                        print("Trop d'erreurs dans le bloc "+str(i//17))
                        break
                else:
                    if CRCR!=data_in[i]&0x7F:
                        o,b=singleBitErrorFinderCRC7(15,(data_in[i]&0x7F)^CRCR)
                        if o==-1 or not parity_error[o]:
                            print("Le CRC n'est pas bon, il y a une erreur de parité mais l'erreur n'est pas détectée au bon endroit: "+str(o)+"!="+str(parity_error.index(True)))
                            print("Trop d'erreurs dans le bloc "+str(i//17))
                            break
                        else:
                            print("Erreur dans le bloc détecté: octet="+str(o)+" bit="+str(b))
                            print("Données récupérées")
                            data_in[i-15+o]=data_in[i-15+o]^(1<<b)
                    else:
                        print("Le CRC est bon mais il y a une erreur de parité")
                        print("Trop d'erreurs dans le bloc "+str(i//17))
                        break
            CRCR=0
            parity_error=[False]*16
        else:
            if (not checkParity(data_in[i])):
                print("Erreur de parité à l'octet "+str(i))
                parity_error[i%17]=True
            CRCR=updateCRC7(data_in[i],CRCR)
    return data_out



if __name__=="__main__":

    data_pre_pce=[0x1B,0x2F,0x3F,0x1B,0x39,0x7F,0x1B,0x3A,0x64,0x52,0x1B,0x3B,0x69,0x59,0x41]
    data_post_pce=[0x1B,0xAF,0x3F,0x1B,0x39,0xFF,0x1B,0x3A,0xE4,0xD2,0x1B,0xBB,0x69,0x59,0x41,0x44,0x00]

    data_post_pce_bad1=[0x1B,0xAF,0x3F,0x1B,0x38,0xFF,0x1B,0x3A,0xE4,0xD2,0x1B,0xBB,0x69,0x59,0x41,0x44,0x00]#bit inversé (4,0)
    data_post_pce_bad2=[0x1B,0xAF,0x3F,0x1B,0x39,0xFF,0x1B,0x3A,0xE4,0xD2,0x1B,0xBB,0x69,0x59,0x41,0x40,0x00]#mauvais CRC
    data_post_pce_bad3=[0x1B,0xAE,0x3F,0x1B,0x38,0xFF,0x1B,0x3A,0xE4,0xD2,0x1B,0xBB,0x69,0x59,0x43,0x44,0x00]#bits inversés (1,0), (4,0), (15,1)
    data_post_pce_bad4=[0x1B,0xAF,0x3F,0x1B,0x39,0xFF,0x1B,0x3A,0xE4,0xD1,0x1B,0xBB,0x69,0x59,0x41,0x44,0x00]#bits inversés (9,0), (9,1)
    data_post_pce_bad5=[0x1B,0xAF,0x3F,0x1B,0x39,0xFF,0x1B,0x3A,0xE4,0xD1,0x1B,0xBB,0x69,0x59,0x41,0x41,0x00]#bits inversés (9,0), (9,1) + mauvais CRC / pas d'erreurs de parité

    print("message avant parité / PCE")
    print(" ".join(hex(n) for n in data_pre_pce))
    print("message après PCE")
    print(" ".join(hex(n) for n in PCESend(data_pre_pce)))
    print("message après PCE (but)")
    print(" ".join(hex(n) for n in data_post_pce))

    print("\nmessage original")
    print(" ".join(hex(n) for n in PCEReceive(data_post_pce)))
    print("\nbit inversé (4,0)")
    print(" ".join(hex(n) for n in PCEReceive(data_post_pce_bad1)))
    print("\nmauvais CRC")
    print(" ".join(hex(n) for n in PCEReceive(data_post_pce_bad2)))
    print("\nbits inversés (1,0), (4,0), (15,1)")
    print(" ".join(hex(n) for n in PCEReceive(data_post_pce_bad3)))
    print("\nbits inversés (9,0), (9,1)")
    print(" ".join(hex(n) for n in PCEReceive(data_post_pce_bad4)))
    print("\nbits inversés (9,0), (9,1) + mauvais CRC / pas d'erreurs de parité")
    print(" ".join(hex(n) for n in PCEReceive(data_post_pce_bad5)))
