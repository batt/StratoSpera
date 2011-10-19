#!/usr/bin/env python

from ax25 import AX25_ESC, HDLC_FLAG, HDLC_RESET
from collections import deque
from math import sin, pi

SAMPLERATE = 9600
BITRATE = 1200
SAMPLE_PER_BIT = 8
PHASE_BIT = 8
PHASE_INC = 1
PHASE_MAX = SAMPLE_PER_BIT * PHASE_BIT
PHASE_THRES = PHASE_MAX / 2

#Modulator constants
SIN_LEN = 1024

MARK_FREQ = 1200.0
MARK_INC = int(round((SIN_LEN * MARK_FREQ) / SAMPLERATE))

SPACE_FREQ = 2200.0
SPACE_INC = int(round((SIN_LEN * SPACE_FREQ) / SAMPLERATE))

def bit_differ(bitline1, bitline2):
    return (bitline1 ^ bitline2) & 0x01

def edge_found(bitline):
    return bit_differ(bitline, bitline >> 1)

class Hdlc(object):
    def __init__(self):
        self.demod_bits = 0
        self.rxstart = False
        self.currchar = 0
        self.bit_idx = 0

    def parse(self, bit):
        self.demod_bits <<= 1
        self.demod_bits &= 0xFF
        self.demod_bits |= 1 if bit else 0

       	#HDLC Flag
        if self.demod_bits == ord(HDLC_FLAG):
            self.rxstart = True
            self.currchar = 0
            self.bit_idx = 0
            return HDLC_FLAG

	    #Reset
        if (self.demod_bits & ord(HDLC_RESET)) == ord(HDLC_RESET):
            self.rxstart = False
            return ""

        if not self.rxstart:
            return ""

        #Stuffed bit
        if (self.demod_bits & 0x3f) == 0x3e:
            return ""

        if self.demod_bits & 0x01:
            self.currchar |= 0x80

        self.bit_idx += 1
        out = ""
        if self.bit_idx >= 8:
            if self.currchar in (ord(HDLC_FLAG), ord(HDLC_RESET), ord(AX25_ESC)):
                out += AX25_ESC

            out += chr(self.currchar)

            self.currchar = 0
            self.bit_idx = 0
        else:
            self.currchar >>= 1

        return out

class Afsk(object):
    def __init__(self, stream, preamble_len=500, trailer_len=50):
        self.stream = stream

        #RX section
        assert(SAMPLERATE == 9600)
        assert(BITRATE == 1200)
        assert(SAMPLE_PER_BIT == 8)

        self.delay_fifo = deque([0] * (SAMPLE_PER_BIT / 2))
        self.iir_x = [0] * 2
        self.iir_y = [0] * 2
        self.sampled_bits = 0
        self.curr_phase = 0
        self.found_bits = 0
        self.hdlc = Hdlc()
        self.out = deque()

        #TX section
        self.dds_table = [chr(int(round(127.5 * (1 + sin(2 * pi * i / SIN_LEN))))) for i in range(SIN_LEN)]
        self.preamble = (preamble_len * BITRATE + 4000) / 8000
        self.trailer = (trailer_len * BITRATE + 4000) / 8000
        self.resetTx()
        self.sending = False

    def switchTone(self):
        return MARK_INC if self.phase_inc == SPACE_INC else SPACE_INC

    def genBit(self, bit):
        if not bit:
            self.phase_inc = self.switchTone()

        for i in range(SAMPLE_PER_BIT):
            self.stream.write(self.dds_table[self.phase_acc])
            self.phase_acc += self.phase_inc
            self.phase_acc %= SIN_LEN

    def sendChar(self, c, stuff=True):
        c = ord(c)
        for i in range(8):
            bit = (1<<i) & c
            self.genBit(bit)
            if bit and stuff:
                self.stuff_cnt +=1
                if self.stuff_cnt >= 5:
                    self.genBit(0)
                    self.stuff_cnt = 0
            else:
                self.stuff_cnt = 0

    def resetTx(self):
        self.phase_acc = 0
        self.phase_inc = MARK_INC
        self.stuff_cnt = 0
        self.escape = False

    def sendSync(self, c):
        l = self.preamble if self.sending else self.trailer
        for i in range(l):
            self.sendChar(c, stuff=False)

    def write(self, data):
        for c in data:
            if c == AX25_ESC and not self.escape:
                self.escape = True
                continue

            if c == HDLC_FLAG and not self.escape:
                self.sending = not self.sending
                if self.sending:
                    self.resetTx()
                self.sendSync(HDLC_FLAG)
            elif c == HDLC_RESET and not self.escape:
                self.sendChar(c, stuff=False)
            else:
                self.sendChar(c)

            self.escape = False

    def _processSample(self, sample):
        sample = ord(sample) - 127

        self.iir_x[0] = self.iir_x[1]
        self.iir_x[1] = (self.delay_fifo.popleft() * sample) >> 2
        #self.iir_x[1] = (self.delay_fifo.popleft() * sample) / 3.558147322
        self.iir_y[0] = self.iir_y[1]
        self.iir_y[1] = self.iir_x[0] + self.iir_x[1] + (self.iir_y[0] >> 1)
        #self.iir_y[1] = self.iir_x[0] + self.iir_x[1] + self.iir_y[0] * 0.4379097269

        self.sampled_bits <<= 1
        self.sampled_bits |= 1 if self.iir_y[1] > 0 else 0

        self.delay_fifo.append(sample)

        if edge_found(self.sampled_bits):
            if self.curr_phase < PHASE_THRES:
                self.curr_phase += PHASE_INC
            else:
                self.curr_phase -= PHASE_INC
        self.curr_phase += PHASE_BIT

        if self.curr_phase >= PHASE_MAX:
            self.curr_phase %= PHASE_MAX
            self.found_bits <<= 1
            self.found_bits &= 0xFF

            self.sampled_bits &= 0x07

            if self.sampled_bits in (0x07, 0x06, 0x05, 0x03):
                self.found_bits |= 1
            return self.hdlc.parse(not edge_found(self.found_bits))
        else:
            return ""

    def read(self, size=1):
        while len(self.out) < size:
            for c in self.stream.read(512):
                self.out.extend(self._processSample(c))

        out = [self.out.popleft() for i in range(size)]
        return "".join(out)

if __name__ == "__main__":
    import pyaudio
    import sys
    p = pyaudio.PyAudio()

    stream = p.open(format = pyaudio.paUInt8,
        channels = 1,
        rate = SAMPLERATE,
        input = True,
        output = True,
        frames_per_buffer = 1024)
    afsk = Afsk(stream)
    #sys.stdout.write(afsk.read(1024))
    afsk.write(HDLC_FLAG)
    afsk.write(">this is a test")
    stream.close()
    p.terminate()


