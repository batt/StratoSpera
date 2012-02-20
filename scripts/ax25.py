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
        self.outbuf = []

    def _write(self, data):
        for c in data:
            if c == HDLC_FLAG or c == HDLC_RESET or c == AX25_ESC:
		        self.outbuf.append(AX25_ESC)
            self.crc_out = updcrc_ccitt(c, self.crc_out);
            self.outbuf.append(c)

    def sendCall(self, call, last=False):
        call = call.upper()
        if call.find('-') != -1:
            call, ssid = call.split('-')
        else:
            ssid = '0'
        call = call.ljust(6)
        call = ''.join([chr((ord(c) << 1)) for c in call])
        ssid = chr(0x60 | ((int(ssid) & 0x0f) << 1) | (0x01 if last else 0))
        self._write(call + ssid)

    def send(self, path, data):
        self.crc_out = CRC_CCITT_INIT_VAL
        self.outbuf.append(HDLC_FLAG)

        last = len(path) - 1
        for i, p in enumerate(path):
            self.sendCall(p, i == last)

        self._write(chr(AX25_CTRL_UI))
        self._write(chr(AX25_PID_NOLAYER3))
        self._write(data)

        crcl = chr((self.crc_out & 0xff) ^ 0xff)
        crch = chr((self.crc_out >> 8) ^ 0xff)
        self._write(crcl)
        self._write(crch)
        self.outbuf.append(HDLC_FLAG)
        self.stream.write(''.join(self.outbuf))
        self.outbuf = []


    def _process(self, c):
        if not self.escape and c == HDLC_FLAG:
            msg = None
            if len(self.buf) >= AX25_MIN_FRAME_LEN:
                if self.crc_in == AX25_CRC_CORRECT:
                    msg = decode_packet(self.buf)

            self.sync = True
            self.crc_in = CRC_CCITT_INIT_VAL
            self.buf.clear()
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

    def recv(self):
        msg = None
        while not msg:
            msg = self._process(self.stream.read(1))

        return msg

if __name__ == "__main__":

    import sys
    import pyaudio
    import afsk
    import time

    p = pyaudio.PyAudio()

    stream = p.open(format = pyaudio.paUInt8,
        channels = 1,
        rate = 9600,
        input = True,
        output = True,
        frames_per_buffer = 0)
    afsk = afsk.Afsk(stream)
    ax25 = Ax25(afsk)

    if len(sys.argv) > 1:
        sender = sys.argv[1]
        assert(len(sender) <= 6)
        assert(sender.isalnum())
        sys.stderr.write("AX25 in send mode, sender [%s], reading from stdin...\n" % sender.upper())
        data = sys.stdin.read()
        sys.stderr.write("sending...\n")
        ax25.send(["apzbrt", sender], data)
        time.sleep(1)
        sys.stderr.write("Done.\n")
    else:
        sys.stderr.write("AX25 in receive mode...\n")
        while 1:
            m = ax25.recv()
            sys.stdout.write("AFSK1200: fm %s\n" % m['src'])
            sys.stdout.write(m['data'].strip() + '\n')
            sys.stdout.flush()

    stream.close()
    p.terminate()
