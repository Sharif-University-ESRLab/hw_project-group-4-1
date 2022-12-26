import threading

from server import tcp, udp

TCP_THROUGHPUT = (2001, 5)
TCP_LATENCY = (2002, 5)
UDP_THROUGHPUT = (2003, 5)
UDP_LATENCY = (2004, 5)


def main():
    # tcp.start_tcp_throughput(2000, 5, True)
    # tcp.start_tcp_throughput(2000, 5, False)
    udp.start_latency(2001, 5)
    ...
    # threads = [
    #     threading.Thread(target=tcp.start_tcp_throughput, args=TCP_THROUGHPUT),
    #     threading.Thread(target=tcp.start_tcp_latency, args=TCP_LATENCY),
    #     threading.Thread(target=udp.start_udp_throughput, args=UDP_LATENCY),
    #     threading.Thread(target=udp.start_udp_latency, args=UDP_LATENCY),
    # ]
    # list(map(threading.Thread.start, threads))


if __name__ == '__main__':
    main()
