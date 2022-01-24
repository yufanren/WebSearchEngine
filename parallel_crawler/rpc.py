from xmlrpc.server import SimpleXMLRPCServer
from xmlrpc.client import ServerProxy
from threading import Thread
from requests.exceptions import ConnectionError
import config
import time, warnings

# communication module between nodes
# defines remote procedure call methods for client and server
class RPCServer:

    def __init__(self, application):
        self.port = 8080 + application.id
        self.application = application
        self.server = SimpleXMLRPCServer(('127.0.0.1', self.port), allow_none=True, logRequests=False)
        self.server.register_function(self.add_urls)
        self.server.register_function(self.kill)

    def add_urls(self, urls):
        self.application.insert_links_safe(urls)

    def serve(self):
        self.quit = 0
        while not self.quit:
            self.server.handle_request()

    def kill(self):
        self.quit = 1
        return 1

    def start(self) -> Thread:
        urls_in = Thread(target=self.serve)
        urls_in.start()        
        return urls_in        


class RPCClient:

    def __init__(self, application, server=None):
        self.application = application
        self.proxys = {}
        if application != None:
            for i in range(config.NUM_NODES):
                if i != application.id:
                    self.proxys[i] = ServerProxy('http://127.0.0.1:' + str(8080 + i))
        else:
            self.proxys[0] = ServerProxy('http://127.0.0.1:' + str(8080 + server))

    def send_urls(self, proxy_id, urls):
        try:
            self.proxys.get(proxy_id).add_urls(urls)
        except (ConnectionError, ConnectionRefusedError):
            warnings.warn(f"can not connect to server {proxy_id}")


    def register_completion(self):
        proxy = ServerProxy('http://127.0.0.1:9080')
        try:
            proxy.register_completion()
        except (ConnectionError, ConnectionRefusedError):
            warnings.warn(f"can not connect to starter node")


    def run(self):
        self.quit = 0
        while not self.quit:
            idle = True
            for proxy_id, url_list in self.application.out_lists.items():
                if len(url_list) >= 100:
                    idle = False
                    message = url_list
                    self.application.out_lists[proxy_id] = []
                    self.send_urls(proxy_id, message)
            if idle:
                time.sleep(0.1)

    def start(self) -> Thread:
        urls_out = Thread(target=self.run)
        urls_out.setDaemon(True)
        urls_out.start()        
        return urls_out 



