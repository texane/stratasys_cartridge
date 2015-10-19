#!/usr/bin/env python


# very crude client to communicate with device
# use python, pyserial and ascii to work across oses
# usage: main.py
#  <device_path> rmem <offset> <size>
#  <device_path> rrom
#  <device_path> wmem <offset> <data_or_file>


# TODO: read write by line
# TODO: verify after write
# TODO: ack after each command


import serial
import sys
import os


def makeline(s):
    return s + '\n'


def stripline(l):
    return l.replace('\r', '').replace('\n', '')


def read_ack(ser):
    # TODO
    return 0


def multiline(s):
    # split a string into multiple lines to be sent
    line_max_len = 16
    s_len = len(s)
    l = []
    for i in range(0, s_len, line_max_len):
        line_len = line_max_len
        if s_len - i < line_max_len: line_len = s_len - i
        l.append(s[i : i + line_len])
    return l


def do_read_mem(ser, av):
    if len(av) != 2: return -1
    off = av[0]
    size = av[1]
    s = makeline('rmem ' + off + ' ' + size)
    ser.write(s)
    return 0


def do_read_rom(ser, av):
    if len(av) != 0: return -1
    s = makeline('rrom')
    ser.write(s)
    return 0


def get_data_or_file(s):
    # if s is a path, get the contents
    # otherwise, s is hexadecimal data
    if os.path.isfile(s) == False: return s
    f = open(s, 'r')
    s = f.read()
    f.close()
    return stripline(s)


def is_hex(x):
    if x >= '0' and x <= '9': return True
    elif x >= 'a' and x <= 'f': return True
    return False


def check_data(s):
    n = len(s)
    if (n % 2) != 0: return -1
    for i in range(0, n):
        if is_hex(s[i]) == False: return -1
    return 0


def do_write_mem(ser, av):
    if len(av) != 2: return -1
    off = int(av[0])
    data = get_data_or_file(av[1])
    if check_data(data): return -1
    n = len(data)
    for i in range(0, n / 2):
        j = i * 2
        x = data[j : j + 2]
        s = makeline('wmem ' + str(off + i) + ' ' + x)
        ser.write(s)
    return 0


def main(av):
    dev = av[1]
    ser = serial.Serial(dev, 115200, timeout = 1)
    if av[2] == 'rmem': err = do_read_mem(ser, av[3:])
    elif av[2] == 'rrom': err = do_read_rom(ser, av[3:])
    elif av[2] == 'wmem': err = do_write_mem(ser, av[3:])
    else: err = -1
    ser.close()
    if err: print('an error occured')
    return err


main(sys.argv)
