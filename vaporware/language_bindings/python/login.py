import llvp

TOKEN = "sixteen letters."
HOST = "vaporlight.intranet.entropia.de"
PORT = 7534
NUM_LEDS = 20

def connect(token=TOKEN):
    return llvp.SocketController(token, HOST, PORT)

