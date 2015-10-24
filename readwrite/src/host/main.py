#!/usr/bin/env python


#
# very crude client to communicate with device
# use python, pyserial for cross oses portability
# use sync ascii based protocol to work with common term tools
# use per line acknowledge for flow control
#
# command line usage
# note: address and size interpreted as hex numbers 
#
# set the current memory address:
# main.py <device_path> addr <addr>
#
# print the current memory address:
# main.py <device_path> addr
#
# read data from memory:
# main.py <device_path> rmem <size>
#
# write data to memory:
# main.py <device_path> wmem <data_or_file>
#
# read eeprom identifier:
# main.py <device_path> rrom
#
# retrieve max line length:
# main.py <device_path> llen
#


import serial
import sys
import os


def append_eol(s):
    return s + '\n'


def strip_eol(l):
    return l.rstrip('\n').rstrip('\r')


def is_hex(x):
    if x >= '0' and x <= '9': return True
    elif x >= 'a' and x <= 'f': return True
    elif x >= 'A' and x <= 'F': return True
    return False


def check_data_line(l):
    n = len(l)
    if (n % 2) != 0: return -1
    for c in l:
        if is_hex(c) == False: return -1
    return 0


max_line_len = 16
def single_to_multi_line(s):
    # split a string into multiple lines to be sent
    s_len = len(s)
    l = []
    for i in range(0, s_len, max_line_len):
        line_len = max_line_len
        if s_len - i < max_line_len: line_len = s_len - i
        l.append(s[i : i + line_len])
    return l


def size_to_line_count(size):
    n = size / max_line_len
    if size % max_line_len: n = n + 1
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
    ser.write(append_eol(l))
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
    res = send_cmd_and_recv_multi_line(ser, lines, 0)
    if res == None: return -1
    return 0


def format_uint16(x):
    try: s = '%04x' % int(x, 16)
    except: s = None
    return s


def do_addr(ser, av):
    if len(av) == 1:
        addr = format_uint16(av[0])
        if addr == None: return -1
        cmd_line = [ 'addr ' + addr ]
        nline = 0
    elif len(av) == 0:
        cmd_line = [ 'addr' ]
        nline = 1
    else: return -1
    lines = send_cmd_and_recv_multi_line(ser, cmd_line, nline)
    if lines == None: return -1
    for l in lines: print(l)
    return 0


def do_rmem(ser, av):
    if len(av) != 1: return -1
    size = format_uint16(av[0])
    if size == None: return -1
    cmd_line = [ 'rmem ' + size ]
    n = size_to_line_count(int('0x' + size, 16))
    repl_lines = send_cmd_and_recv_multi_line(ser, cmd_line, n)
    if repl_lines == None: return -1
    for l in repl_lines: print(l)
    return 0


def do_rrom(ser, av):
    if len(av) != 0: return -1
    cmd_line = [ 'rrom' ]
    repl_lines = send_cmd_and_recv_multi_line(ser, cmd_line, 1)
    if repl_lines == None: return -1
    for l in repl_lines: print(l)
    return 0


def do_wmem(ser, av):
    if len(av) != 1: return -1
    lines = data_or_file_to_multi_line(av[0])
    if lines == None: return -1
    lines.insert(0, 'wmem')
    # append empty line to end data
    lines.append('')
    send_cmd(ser, lines)
    return 0


def do_llen(ser, av, do_print = True):
    if len(av) != 0: return -1
    cmd_line = [ 'llen' ]
    repl_lines = send_cmd_and_recv_multi_line(ser, cmd_line, 1)
    if repl_lines == None: return -1
    if do_print == True:
        for l in repl_lines: print(l)
    max_line_len = int('0x' + repl_lines[0], 16)
    return 0


def main(av):
    dev = av[1]
    ser = serial.Serial(dev, 9600, timeout = 1)
    do_llen(ser, [], do_print = False)

    if av[2] == 'addr': err = do_addr(ser, av[3:])
    elif av[2] == 'rmem': err = do_rmem(ser, av[3:])
    elif av[2] == 'wmem': err = do_wmem(ser, av[3:])
    elif av[2] == 'rrom': err = do_rrom(ser, av[3:])
    elif av[2] == 'llen': err = do_llen(ser, av[3:])
    else: err = -1

    ser.close()

    if err: print('error')
    return err


main(sys.argv)
