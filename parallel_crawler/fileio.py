from queue import Queue
from threading import Thread
from pathlib import Path

# Module dictates how logs are written to File
# or how files are read by the crawler
class Logger:

    def __init__(self, metadata):
        self.jobs = Queue()
        self.data = metadata
        self.limit = self.data.get('limit')

    def consume(self):
        file_counter, counter = 1, 0
        folder = self.data.get('folder')
        id = str(self.data.get('id'))
        start_time = self.data.get('time')
        file = f'{folder}/node_{id}_{start_time}_{file_counter}.txt'  
        log = open(file, 'w')
        loop = True
        try:
            while loop:
                i = self.jobs.get(block=True, timeout=None)
                if i != None:
                    log.write(f'{i}\n')
                    counter += 1
                    if counter >= self.limit:
                        log.close()
                        file = f'{folder}/node_{id}_{start_time}_{file_counter}.txt'
                        log = open(file, 'w')
                        file_counter += 1
                        counter = 0
                else: 
                    loop = False
        finally:
            log.close()
        

    def start(self) -> Thread:
        consumer = Thread(target=self.consume)
        consumer.setDaemon(True)
        consumer.start()        
        return consumer     


class QueueReader:

    def __init__(self, metadata):
        self.data = metadata
        self.file_counter = 1

    def read_next_queue(self):
        folder = self.data.get('folder')
        id = str(self.data.get('id'))
        start_time = self.data.get('time')
        file = f'{folder}/node_{id}_{start_time}_{self.file_counter}.txt'
        if Path(file).is_file():
            queue_file = open(file, 'r')
            urls = queue_file.readlines()
            queue_file.close()
            self.file_counter += 1
            Path.unlink(file)
            return urls
        return []

