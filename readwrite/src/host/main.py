#!/usr/bin/env python


#
# very crude client to communicate with device
# use python, pyserial for cross oses portability
# use synchronous ascii line based protocol to work with common term tools
# use per command line acknowledge for flow control
#
# usage: main.py
#  <device_path> rmem <offset> <size>
#  <device_path> rrom
#  <device_path> wmem <offset> <data_or_file>
#


import serial
import sys
import os


def append_eol(s):
    return s + '\n'


def strip_eol(l):
    return l.rstrip('\n').l('\r')


def is_hex(x):
    if x >= '0' and x <= '9': return True
    elif x >= 'a' and x <= 'f': return True
    elif x >= 'A' and x <= 'F': return True
    return False


def check_data_line(s):
    n = len(s)
    if (n % 2) != 0: return -1
    for i in range(0, n):
        if is_hex(s[i]) == False: return -1
    return 0


max_line_len = 32
def single_to_multi_line(s):
    # split a string into multiple lines to be sent
    s_len = len(s)
    l = []
    for i in range(0, s_len, max_line_len):
        line_len = max_line_len
        if s_len - i < max_line_len: line_len = s_len - i
        l.append(s[i : i + line_len])
    return l


def size_to_line_count(s):
    s_len = len(s)
    n = s_len / max_line_len
    if s_len % max_line_len: n = n + 1
    return n


def read_data_file(s):
    # read a data file contents
    # assume ascii file containing hex chars
    # merge lines, if any
    f = open(s, 'r')
    x = f.read()
    f.close()
    lines = x.splitlines()
    return lines.join()


def data_or_file_to_multi_line(s):
    # if s is a path, get the contents in a single line
    # otherwise, s is hexadecimal data
    if os.path.isfile(s) == True: s = read_data_file(s)
    if check_data_line(s): return None
    return single_to_multi_line(s)


def recv_line(ser):
    # receive a single line
    # eol is stripped
    return strip_eol(ser.readline())


def recv_multi_line(ser, nline):
    # receive all lines until ack
    lines = []
    while True:
        l = recv_line(ser)
        if l == None: break
        lines.append(l)
    return lines


def send_line_recv_ack(ser, l):
    ser.write(ser, append_eol(l))
    return recv_line(ser)


def send_cmd_and_recv_multi_line(ser, cmd_lines, nline):
    for l in cmd_lines:
        ack = send_line_recv_ack(ser, l)
        if ack != 'ok': return None
    repl_lines = []
    for i in range(0, nline):
        repl_lines.append(recv_line(ser))
    return repl_lines


def send_cmd(ser, lines):
    send_cmd_and_recv_multi_line(ser, lines, 0)
    return 0


def do_read_mem(ser, av):
    if len(av) != 2: return -1
    off = av[0]
    size = av[1]
    cmd_line = [ 'rmem ' + off + ' ' + size ]
    n = size_to_line_count(size)
    repl_lines = send_cmd_and_recv_multi_line(ser, cmd_line, n)
    for l in repl_lines: print(l)
    return 0


def do_read_rom(ser, av):
    if len(av) != 0: return -1
    cmd_line = [ 'rrom' ]
    repl_lines = send_cmd_and_recv_multi_line(ser, cmd_line, 1)
    for l in repl_lines: print(l)
    return 0


def do_write_mem(ser, av):
    if len(av) != 2: return -1
    off = int(av[0])
    lines = data_or_file_to_multi_line(av[1])
    if lines == None: return -1
    lines.prepend('wmem ' + str(off))
    send_cmd(ser, lines)
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
