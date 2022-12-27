import datetime
import socket
from typing import List

from glogger.logger import get_logger

logger = get_logger('udp')


def start(port: int):
    """
    Starts a udp socket on given port

    Args:
        port: socket port
    """

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind(('0.0.0.0', port))
    while True:
        data, addr = s.recvfrom(1)
        if data is not None:
            break
    return s, addr


def start_throughput(port: int, period: int, upload: bool):
    """
    Start a udp socket server to test throughput

    Args:
        port: socket port
        period: time of test in seconds
        upload: test upload or download
    """

    conn, addr = start(port)
    logger.info("Start udp throughput with period: %d", period)
    now = datetime.datetime.now()
    bytes_cnt = [0] * period
    while True:
        total = (datetime.datetime.now() - now).total_seconds()
        if total >= period:
            break
        if upload:
            data, _ = conn.recvfrom(1024)
            bytes_cnt[int(total)] += len(data)
        else:
            conn.sendto(b'*'*512, addr)
            bytes_cnt[int(total)] += 512
    logger.info("Result: %s", bytes_cnt)
    conn.close()
    return bytes_cnt


def start_latency(port: int, number_of_packets: int) -> List[int]:
    """
    Starts latency test

    Args:
        port: socket port
        number_of_packets: number of packets to test latency
    """
    conn, addr = start(port)
    logger.info("Start udp latency with %d packets", number_of_packets)
    result = []
    for i in range(number_of_packets):
        now = datetime.datetime.now()
        conn.sendto(b'$', addr)
        conn.recv(1)
        result.append(int((datetime.datetime.now() - now).total_seconds() * 1_000))
    conn.close()
    logger.info("Result: %s", result)
    return result
