from time import sleep
import datetime
from sup.display_driver import DisplayDriver
import unittest

#DEFAULT_HOST='raspberrypi2.wunderbar.internal'
DEFAULT_HOST='localhost'


class DisplayTests(unittest.TestCase):
    screen = None

    def get_status(self) -> str:
        status = self.screen.get_status()
        return status

    @classmethod
    def setUpClass(cls) -> None:
        cls.screen = DisplayDriver(host=DEFAULT_HOST)
        cls.screen.open()
        cls.screen.reset()

    @classmethod
    def tearDownClass(cls) -> None:
        cls.screen.reset()
        cls.screen.close()
        cls.screen = None

    def test_set_heading(self):
        """
        Test the set heading function.
        """
        heading_text = 'Current Date {}'.format(str(datetime.datetime.now()))
        heading_text = heading_text.replace(':', '.')
        self.assertTrue(self.screen.set_heading(heading_text), msg='Unable to set heading')
        stats = self.get_status()
        self.assertTrue(stats['heading'] == heading_text, msg='Heading text is not what it was set to')

    def test_set_message(self):
        """
        Test the set message function.
        """
        message_text = 'Current Date {}'.format(str(datetime.datetime.now()))
        message_text = message_text.replace(':', '.')
        self.assertTrue(self.screen.set_message(message_text), msg='Unable to set message')
        stats = self.get_status()
        self.assertTrue(stats['message'] == message_text, msg='Message text is not what it was set to')

    def test_set_text(self):
        """
        Test the set text function.
        """
        text_text = 'Current Date {}'.format(str(datetime.datetime.now()))
        text_text = text_text.replace(':', '.')
        self.assertTrue(self.screen.set_text(text_text, text_text), msg='Unable to set text')
        stats = self.get_status()
        self.assertTrue(stats['heading'] == text_text, msg='Heading text is not what it was set to')
        self.assertTrue(stats['message'] == text_text, msg='Message text is not what it was set to')

    def test_set_time(self):
        time_stat = self.get_status()['time']
        self.screen.set_time(not time_stat)
        self.assertTrue(self.get_status()['time'] != time_stat, msg='Unable to toggle time setting')
        self.screen.set_time(time_stat)
        self.assertTrue(self.get_status()['time'] == time_stat, msg='Unable to toggle time setting')

    def test_list_scripts(self):
        scripts = self.screen.get_script_list()
        self.assertTrue(len(scripts) == 2, msg='Did not find 2 scripts in the list')

    def test_list_styles(self):
        styles = self.screen.get_style_list()
        self.assertTrue(len(styles) == 2, msg='Did not find 2 scripts in the list')


def main():
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument('--server', type=str, default='localhost', help='Server to connect to')
    parser.add_argument('--port', type=int, default=4321, help='Port to connect to')

    opts = parser.parse_args()

    try:
        with DisplayDriver(host=opts.server, port=opts.port) as disp:
            try:
                disp.set_text('Joe-Tap Automated Unit Test',
                              'Connection Established')
                disp.set_elapsed(0, 1)
                disp.set_elapsed(1, 1)
                disp.start_elapsed(1)
                sleep(1)

                for x in range(0,3):
                    disp.start_elapsed(0)
                    disp.set_message('Running \'Invalid Voltage\' test...')
                    sleep(4)
                    disp.start_elapsed(0)
                    disp.set_message('Running \'Communication Down\' test...')
                    sleep(4)
                    disp.start_elapsed(0)
                    disp.set_message('Running \'Dispense Liquid\' test...')
                    sleep(4)
                    disp.start_elapsed(0)
                    disp.set_message('Running \'Low Pressure\' test...')
                    sleep(4)
                    disp.start_elapsed(0)
                    disp.set_message('Running \'Lockout\' test...')
                    sleep(6)
                    disp.start_elapsed(0)
                    disp.set_message('Running \'Clean Warn\' test...')
                    sleep(6)
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
