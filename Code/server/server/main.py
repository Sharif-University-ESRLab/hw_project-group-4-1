import argparse
import os
import signal
import sys

import udp
import uploadserver
from glogger.logger import get_logger
from server import tcp

PORT = 9999  # Server port
PERIOD = 10  # Time of download & upload test in seconds
NUMBER_OF_PACKETS = 10  # Number of packets to send for latenccy test
FILE_SIZE = 512  # File size of http download test
FILE_NAME = "dummyFile"  # Name of file to test download for http
EMPTY_FILE_NAME = "emptyDummyFile"  # Name of file to test latency for http
PACKET_SIZE = 2000  # Size of packets to send and receive


def main():
    # Initialize logger
    logger = get_logger("main")
    # Parse arguments
    parser = argparse.ArgumentParser(description="Run tests")
    parser.add_argument(
        "--protocol",
        "-p",
        type=str,
        help="Protocol\nValues: tcp, udp, http",
        required=True,
    )
    parser.add_argument(
        "--test",
        "-t",
        type=str,
        help="Type of test\nValues: latency, download, upload",
        required=False,
    )
    args = parser.parse_args()
    protocol: str = args.protocol  # tcp, udp, http
    test: str = args.test  # download, upload, latency

    logger.info("Protocol: %s, test: %s, port: %d", protocol, test, PORT)

    if protocol == "tcp":
        if test == "download":
            # TCP, Download, Throughput
            tcp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=False,
                packet_size=PACKET_SIZE,
            )
        elif test == "upload":
            # TCP, Upload, Throughput
            tcp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=True,
                packet_size=PACKET_SIZE,
            )
        else:
            tcp.start_latency(
                port=PORT,
                number_of_packets=NUMBER_OF_PACKETS,
            )
    elif protocol == "udp":
        if test == "download":
            # UDP, Download, Throughput
            udp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=False,
                packet_size=PACKET_SIZE,
            )
        elif test == "upload":
            # UDP, Upload, Throughput
            udp.start_throughput(
                port=PORT,
                period=PERIOD,
                upload=True,
                packet_size=PACKET_SIZE,
            )
        else:
            # UDP, Latency
            udp.start_latency(
                port=PORT,
                number_of_packets=NUMBER_OF_PACKETS,
            )
    elif protocol == "http":
        # We should remove arguments for uploadserver.main to work
        sys.argv = sys.argv[:1]
        sys.argv.append(str(PORT))
        # Creating a huge file to test download
        os.system(f"dd if=/dev/zero of={FILE_NAME} bs={FILE_SIZE}MB count=1")
        # Creating an empty file to test latency
        os.system(f"touch {EMPTY_FILE_NAME}")

        # Clean dummy files when programm recived SIGINT
        def sigint_handler(_, __):
            logger.info("Cleaning dummy file")
            os.system(f"rm -f {FILE_NAME} {EMPTY_FILE_NAME}")
            sys.exit(0)

        signal.signal(signal.SIGINT, sigint_handler)

        # Server http server
        uploadserver.main()


if __name__ == "__main__":
    main()
