from time import sleep
import datetime
from sup.display_driver import DisplayDriver
import unittest

DEFAULT_HOST='raspi2-dev.wunderbar.internal'


def main():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('--server', type=str, default='localhost', help='Server to connect to')
    parser.add_argument('--port', type=int, default=4321, help='Port to connect to')

    opts = parser.parse_args()

    try:
        with DisplayDriver(host=opts.server, port=opts.port) as disp:
            disp.gpio_mode(20, 'OUT')

            try:
                for n in range(0, 10):
                    disp.gpio_write(20, 0)
                    sleep(5)
                    disp.gpio_write(20, 1)
                    sleep(5)

            except KeyboardInterrupt as e:
                print('Quitting..')
                # Reset the display before exit...

            disp.reset()
            disp.elapsed_reset()

    except ConnectionRefusedError:
        print('Unable to establish connection with remote display...')
    except BrokenPipeError as e:
        print('Remote display has disconnected unexpectedly...')


if __name__ == '__main__':
    main()
