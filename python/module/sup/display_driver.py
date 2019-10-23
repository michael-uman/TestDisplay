import socket
# import sys
# import time


class DisplayDriver:
    display_host = None
    display_port = 4321

    sock = None

    def __init__(self, host='localhost', port=4321):
        self.display_host = host
        self.display_port = port

    def __enter__(self):
        return self.open()

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def open(self):
        """
        Open the socket to communicate with the TestDisplay application.
        :return:
        """
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # Connect the socket to the port where the server is listening
        server_address = (self.display_host, self.display_port)
        self.sock.connect(server_address)
        return self

    def close(self):
        """
        Terminate connection to the TestDisplay application.
        :return:
        """
        if self.sock:
            self.sock.sendall('QUIT:\n'.encode())
            self.sock.close()
            # self.sock = None
        else:
            raise Exception('Not Connected')
        # pass

    def reset(self) -> bool:
        """
        Reset TestDisplay application to default state.
        :return:
        """
        full_line = 'REST:\n'
        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            result = (data == 'OK')
        else:
            raise Exception('Not Connected')

        return result

    def set_heading(self, heading: str) -> bool:
        """
        Set the Test Display heading
        :param heading: Text to display
        :return: True on success, False on failure.
        """
        result = False
        full_line = "HEAD:" + heading + "\n"

        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            result = (data == 'OK')
        else:
            raise Exception('Not Connected')

        return result

    def set_message(self, message: str) -> bool:
        """
        Set the Test Display message
        :param message: Text to display
        :return: True on success, False on failure.
        """
        result = False
        full_line = "MESG:" + message + "\n"

        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            result = (data == 'OK')
        else:
            raise Exception('Not Connected')

        return result

    def set_text(self, heading: str, message: str) -> bool:
        """
        Set both heading and message in a single call.
        :param heading: Text for heading
        :param message: Text for message
        :return: True on success.
        """
        full_line = "TEXT:" + heading + ":" + message + "\n"
        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            result = (data == 'OK')
        else:
            raise Exception('Not Connected')

        return result

    def set_time(self, status:bool) -> bool:
        """
        Enable/Disable time display.
        :param status: True to enable, False to disable.
        :return:
        """
        full_line = "TIME:{}\n".format('1' if status else '0')
        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            result = (data == 'OK')
        else:
            raise Exception('Not Connected')

        return result

    def get_script_list(self) -> list:
        """
        Return a list of script objects
        """
        full_line = "LIST:SCRIPT\n"
        result = []
        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            data_list = data.split('\n')

            for item in data_list:
                if ':' in item:
                    script_info = item.split(':')
                    script_obj = {
                        'key': script_info[0],
                        'name': script_info[1],
                        'path': script_info[2],
                    }
                    result.append(script_obj)
                if item == 'OK':
                    pass
            # result = (data == 'OK')
        else:
            raise Exception('Not Connected')

        return result

    def get_style_list(self) -> list:
        """
        Return a list of style objects
        """
        full_line = "LIST:STYLE\n"
        result = []
        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            data_list = data.split('\n')

            for item in data_list:
                if ':' in item:
                    style_info = item.split(':')
                    style_obj = {
                        'name': style_info[0],
                        'heading': {
                            'font': style_info[1],
                            'size': int(style_info[2]),
                        },
                        'message': {
                            'font': style_info[3],
                            'size': int(style_info[4]),
                        },
                        'foreground': style_info[5],
                        'background': style_info[6],
                    }
                    result.append(style_obj)
                if item == 'OK':
                    pass
        else:
            raise Exception('Not Connected')

        return result

    def get_status(self) -> object:
        """
        Get TestDisplay status.
        :return: Dictionary containing status.
        """
        full_line = "STAT:\n"
        if self.sock:
            self.sock.sendall(full_line.encode())
            data = self.sock.recv(8192).decode('ascii').strip()
            data_list = data.split(':')
            status_obj = {
                'heading': data_list[1],
                'message': data_list[2],
                'style': data_list[3],
                'time': True if data_list[4] == '1' else False,
                'running': True if data_list[5] == '1' else False,
                'script': data_list[6]
            }
            return status_obj

