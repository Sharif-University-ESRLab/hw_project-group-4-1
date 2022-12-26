import datetime
import socket
import time

ADDR = ('127.0.0.1', 2001)


def main():
    # s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    tcp = False
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM if tcp else socket.SOCK_DGRAM)
    data = b'*' * 512

    s.connect(ADDR)

    # # upload
    # now = datetime.datetime.now()
    # while (datetime.datetime.now() - now).total_seconds() < 6:
    #     s.sendto(data, ADDR)

    # # download
    # recv_bytes = [0] * 20
    # now = datetime.datetime.now()
    # while True:
    #     tm = int((datetime.datetime.now() - now).total_seconds())
    #     data = s.recvfrom(1024)
    #     recv_bytes[tm] += len(data)
    #     if tm >= 10:
    #         break
    # print(recv_bytes)

    # # latency
    # if not tcp:
    #     s.sendto(b'$', ADDR)
    # for i in range(5):
    #     s.recvfrom(1)
    #     s.sendto(b'$', ADDR)



if __name__ == '__main__':
    main()
