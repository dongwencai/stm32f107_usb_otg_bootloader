#! c:\python27\python.exe
import sys
import struct
import binascii
import time
from ctypes import create_string_buffer
class pkg_head:
    def __init__(self):
        # magic type depends ver time[6] len addr crc[150] reserve[30]
        self.magic = 'W'
        self.type = 0
        self.depends = 0
        self.ver = 0
        self.len = 0
        self.addr = 0x0800A000

        self.time = [0 for x in range(0, 6)]
        self.crc = [0 for x in range(0, 150)]
        self.reserve = [0 for x in range(0, 30)]

    def generate_crc(self, data, len):
        crc = 0
        for i in range(0, len, 1):
            crc ^= ord(data[i])
            crc %= 256
        return crc

def check_app_valid(file_path):
    head = pkg_head()
    infile = open(file_path, "rb")

    buf = infile.read(200)
    head.magic, head.type, head.depends, head.ver, head.len, head.addr = struct.unpack("sB2H6x2I180x", buf)
    head.time = struct.unpack_from('6B', buf, 6)
    head.crc = struct.unpack_from('150B', buf, 20)
    head.reserve = struct.unpack_from('30B', buf, 170)
    print head.magic
    print head.type
    print head.time
    print head.crc
    for i in range(200):
        buf = infile.read(2048)
        if(len(buf) == 2048):
            if(head.crc[i] != head.generate_crc(buf, 2048)):
                print('%s is in invalid'%file_path)
                infile.close()
                exit(1)
        else:
            if(head.crc[i] != head.generate_crc(buf, len(buf))):
                print('%s is in invalid'%file_path)
                infile.close()
                exit(1)
            break
    print "APP" if head.type == 1 else "BL",'%s is valid'%file_path

    infile.close()
    exit(0)

def usage():
    print('[usage]: mkimage --filetype pcb_ver infile outfile')
    print('filetype: --application or --bootloader')
    exit(1)

if __name__ == '__main__':
    if(len(sys.argv) < 3):
        usage()
    else:
        head = pkg_head()
        if(sys.argv[1] == '--application'):
            head.type = 1
        elif(sys.argv[1] == '--bootloader'):
            head.type = 2
        elif(sys.argv[1] == '--check'):
            check_app_valid(sys.argv[2])
        else:
            usage()
    head.depends = int(sys.argv[2])
    infile = open(sys.argv[3], "rb")
    infile.seek(0, 2)
    in_file_len = infile.tell()
    head.len = in_file_len
    time_str = time.strftime('%Y-%m-%d-%H-%M-%S',time.localtime(time.time()))
    str_list = time_str.split('-')
    head.time[0] = (int(str_list[0]) - 2000) &0xff
    for i in range(1 , len(str_list)):
        head.time[i] = (int(str_list[i]) & 0xff)
    print head.time
    outfile = open(sys.argv[4], "wb")
    outfile.seek(200)
    infile.seek(0)
    for i in range(200):
        frame_list = []
        if in_file_len >= 2048:
            frame = infile.read(2048)
            frame_list = list(frame)
            in_file_len -= 2048
            head.crc[i] = head.generate_crc(frame_list, 2048)
            outfile.write(''.join(frame_list))
        else:
            frame = infile.read(in_file_len)
            frame_list = list(frame)

            head.crc[i] = head.generate_crc(frame_list, in_file_len)
            in_file_len = 0
            outfile.write(''.join(frame_list))
            break
    outfile.seek(0)
    buf = create_string_buffer(200)
    args = (head.magic, head.type, head.depends, head.ver,head.time, head.len, head.addr, head.crc, head.reserve)
    bin_head = struct.pack('sB', head.magic,head.type)
    struct.pack_into("sB2H6x2I",buf,0, head.magic,head.type, head.depends, head.ver,head.len, head.addr)
    struct.pack_into("6B", buf, 6, *head.time)
    struct.pack_into("150B", buf, 20, *head.crc)
    struct.pack_into("30B", buf, 170, *head.reserve)

    outfile.write(buf)
    infile.close()
    outfile.close()