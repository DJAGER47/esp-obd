import os
import sys
from threading import Thread
import ecu_config
from obd import listener as obd_listener
from uds import listener as uds_listener
from loggers import logger_app, logger_can, logger_isotp

SETUP_VCAN_FILE = "setup_vcan.sh"

SETUP_CAN_FILE = "setup_can.sh"


def main():
    logger_app.configure()
    logger_app.logger.info("Starting ECU-Simulator")
    set_up_can_interface()
    star_can_logger_thread()
    star_isotp_logger_thread()
    start_obd_listener_thread()
    start_uds_listener_thread()


def set_up_can_interface():
    can_interface = ecu_config.get_can_interface()


def star_can_logger_thread():
    Thread(target=logger_can.start).start()


def star_isotp_logger_thread():
    Thread(target=logger_isotp.start).start()


def start_obd_listener_thread():
    Thread(target=obd_listener.start).start()


def start_uds_listener_thread():
    Thread(target=uds_listener.start).start()


if __name__ == '__main__':
    sys.exit(main())
