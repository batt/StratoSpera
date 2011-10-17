#!/usr/bin/env python

from crc_ccitt import *
from collections import deque
AX25_ESC = "\x1B"
HDLC_FLAG = "\x7E"
HDLC_RESET = "\x7F"

AX25_CRC_CORRECT = 0xF0B8
AX25_MIN_FRAME_LEN = 18
AX25_CTRL_UI = 0x03
AX25_PID_NOLAYER3 = 0xF0


def decode_call(buf):
    addr = ""
    for i in range(6):
        addr += chr(ord(buf.popleft()) >> 1)
    addr = addr.strip() + "-" + str((ord(buf[0]) >> 1) & 0x0f)
    return addr

def decode_packet(buf):
    msg = {}
    msg["dst"] = decode_call(buf)
    buf.popleft()
    msg["src"] = decode_call(buf)
    i = 0
    while not (ord(buf.popleft()) & 0x01):
        msg["rpt%d" % i] = decode_call(buf)
        i += 1

    msg["ctrl"] = ord(buf.popleft())

    if msg["ctrl"] != AX25_CTRL_UI:
        print "Only UI frames are handled, got [%02X]" % msg["ctrl"]
        return

    msg["pid"] = ord(buf.popleft())
    if msg["pid"] != AX25_PID_NOLAYER3:
        print "Only frames without layer3 protocol are handled, got [%02X]" % msg["pid"]
        return

    msg["data"] = "".join(buf)[:-2]
    return msg

class Ax25(object):
    def __init__(self, stream):
        self.stream = stream
        self.escape = False
        self.crc_in = CRC_CCITT_INIT_VAL;
        self.sync = False
        self.buf = deque()

    def _process(self, c):
        if not self.escape and c == HDLC_FLAG:
            msg = None
            if len(self.buf) >= AX25_MIN_FRAME_LEN:
                if self.crc_in == AX25_CRC_CORRECT:
                    msg = decode_packet(self.buf)

            self.sync = True
            self.crc_in = CRC_CCITT_INIT_VAL
            self.buf = deque()
            return msg


        if not self.escape and c == HDLC_RESET:
            print "HDLC reset"
            self.sync = False
            return

        if not self.escape and c == AX25_ESC:
            self.escape = True
            return

        if self.sync:
            self.buf.append(c)
            self.crc_in = updcrc_ccitt(c, self.crc_in)

        self.escape = False

    def recv(self, size = 1):
        msg = []
        while len(msg) < size:
            m = self._process(self.stream.read(1))
            if m:
                msg.append(m)

        return msg

if __name__ == "__main__":
    import pyaudio
    import afsk
    p = pyaudio.PyAudio()

    stream = p.open(format = pyaudio.paUInt8,
        channels = 1,
        rate = 9600,
        input = True,
        frames_per_buffer = 1024)
    afsk = afsk.Afsk(stream)
    ax25 = Ax25(afsk)
    while 1:
        m = ax25.recv()
        print "AFSK1200: fm %s" % m[0]['src']
        print m[0]['data']

    stream.close()
    p.terminate()


