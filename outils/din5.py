import asyncio
from websockets.asyncio.client import connect
from websockets.exceptions import ConnectionClosed
T=0b0110100110010110
async def m2s(sws,mws):
    while True:
        async for message in mws:
            try:
                message=bytes([i&0x7F for i in message]).decode("utf-8")
                await sws.send(message)
            except ConnectionClosed as e:
                print("connection closed -> server")
                print(e)
                continue

async def s2m(sws,mws):
    while True:
        async for message in sws:
            try:
                message=message.encode("utf-8")
                message=bytes([i|((((T>>(i&0x0F))^(T>>(i>>4)))&1)<<7) for i in message])
                await mws.send(message)
            except ConnectionClosed as e:
                print("connection closed -> minitel")
                print(e)
                continue

async def main():
    async with connect("ws://localhost:8080") as mws:
        await mws.send(b"\x1B\xBB\x60\x5A\xD1")
        n=5
        while n>0:
            n-=len(await mws.recv())
        async with connect("wss://go.minipavi.fr:8181") as sws:
            asyncio.create_task(m2s(sws,mws))
            await s2m(sws,mws)

if __name__=="__main__":
    print("Liaison entre l'émulateur minitel M12 Philips et Minipavi")
    print("! Travail en cours !")
    print("Ce script python est utile pour se connecter à Minipavi tant qu'une meilleure solution n'est pas implémentée dans l'émulateur lui même!")
    print("Ce script n'a donc pas pour vocation de rester")
    asyncio.run(main())
