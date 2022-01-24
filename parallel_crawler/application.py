from concurrent.futures import thread
import queue, config, utils, fileio
import threading
import mmh3
import rpc
import time
from random import shuffle
from bloomfilter import BloomFilter
from expiringdict import ExpiringDict

# The crawling application stores a list of links fetched from previously crawled web pages.
# It decides what page to fetch next and then sends requests to the crawling system to fetch 
# these pages. 
class CrawlingApplication:

    def __init__(self, id, seed, start_time) -> None:

        self.id = id
        self.start_time = start_time
        self.queue_state_to_file = False
        self.url_filter = BloomFilter(config.PAGE_LIMIT, config.BF_FALSE_POSITIVE_PROB)
        self.domain_filter = BloomFilter(config.PAGE_LIMIT / 4, config.BF_FALSE_POSITIVE_PROB)
        self.novelty_map = {}
        self.novelty_map_lock = threading.Lock()
        self.queue_mode = config.QUEUE_MODE_TO_QUEUE
        self.queue_mode_lock = threading.Lock()
        self.queue_written_to_dick = False
        self.out_lists = {}
        self.domain_rate_limit_cache = ExpiringDict(max_len=100, max_age_seconds=10)
        self.queue_reader = fileio.QueueReader({"id": id,
                                                "folder": "queue",
                                                "time": self.start_time})
        self.queue_writer = fileio.Logger({"id": id,
                                        "folder": "queue",
                                        "time": self.start_time,
                                        "limit": config.QUEUE_FILE_SIZE_LIMIT})
        
        for i in range(config.NUM_NODES):
            if i != self.id:
                self.out_lists[i] = []
        if config.SEARCH_MODE == config.MODE_PRIORITY_SEARCH:
            self.queue = queue.PriorityQueue()
            self.importance_map = {}
            self.importance_map_lock = threading.Lock()
        elif config.SEARCH_MODE == config.MODE_BFS_SEARCH:
            self.queue = queue.Queue()

        urls = [utils.clean_url(url) for url in seed] if seed != None else []
        self.insert_links(urls)
        
        self.url_in_server = rpc.RPCServer(self)
        self.url_out_client = rpc.RPCClient(self)
        self.url_in_server.start()
        self.url_out_client.start()
        
        self.queue_writer_thread = self.queue_writer.start()

        
    def put(self, urls):
        if config.SEARCH_MODE == config.MODE_PRIORITY_SEARCH:
            pass
            # page = CandidatePage(url, domain)
            # self.queue.put(page)
        elif config.SEARCH_MODE == config.MODE_BFS_SEARCH:
            self.insert_links(urls)
                

    def get(self):
        url = self.queue.get()
        if config.SEARCH_MODE == config.MODE_PRIORITY_SEARCH:
            return self.queue.get().url
        elif config.SEARCH_MODE == config.MODE_BFS_SEARCH:
            link = self.remove_link()
            while (link == None):
                link = self.remove_link()
            return link

    def remove_link(self):
        self.check_low_queue_size()
        url = self.queue.get()
        domain = utils.get_domain(url)
        if self.domain_filter.Exists(domain):
            return None
        if self.url_filter.Exists(url):
            return None
        if self.domain_rate_limit_cache.get(domain) != None:
            self.queue.put(url)
            return None
        else:
            self.domain_rate_limit_cache[domain] = 0
        with self.novelty_map_lock:
            if domain in self.novelty_map:
                novelty = self.novelty_map.get(domain) + 1
                if novelty >= config.MAX_PAGE_PER_DOMAIN:
                    self.domain_filter.Insert(domain)
                    self.novelty_map.pop(domain, None)
                else:
                    self.novelty_map[domain] = novelty
            else:
                self.novelty_map[domain] = 1
        self.url_filter.Insert(url)
        return url

    def insert_links(self, urls):
        queue_mode = self.check_queue_mode()
        if queue_mode == config.QUEUE_MODE_TO_DISK:
            for url in urls:
                self.queue_writer.jobs.put(url)
        elif queue_mode == config.QUEUE_MODE_TO_QUEUE:
            exchange = config.COMMUNICAIOON_MODE_EXCHANGE
            for url in urls:
                domain = utils.get_domain(url)
                id = mmh3.hash(domain, 0) % config.NUM_NODES
                if id != self.id:
                    if exchange:
                        self.send_link(id, url)
                    continue
                if self.domain_filter.Exists(domain):
                    continue
                if self.url_filter.Exists(url):
                    continue
                self.queue.put(url)

    def insert_links_safe(self, urls):
        for url in urls:
            if self.filter_links(url):
                self.queue.put(url)

    def send_link(self, id, url):
        self.out_lists[id].append(url)

    def check_queue_mode(self):
        with self.queue_mode_lock:
            if self.queue.qsize() >= config.QUEUE_SIZE_LIMIT:
                self.queue_mode = config.QUEUE_MODE_TO_DISK
                self.queue_written_to_dick = True
            elif self.queue.qsize() < config.QUEUE_SIZE_LIMIT / 2:
                self.queue_mode = config.QUEUE_MODE_TO_QUEUE
            return self.queue_mode

    def check_low_queue_size(self):
        if self.queue_written_to_dick and self.queue.qsize() < config.QUEUE_SIZE_LIMIT / 10:
            self.queue_written_to_dick = False
            urls = self.queue_reader.read_next_queue()
            self.insert_links(urls)
            self.queue_written_to_dick = True



    # def write_to_queue(self, urls):
    #     if self.queue_state_to_file:
    #         for url in urls:
    #             self.queue_writer.jobs.put(url)
    #         return []
    #     else:
    #         if len(urls) <= 20:
    #             return urls
    #         shuffle(urls)
    #         half = len(urls) / 2
    #         for url in urls[half:]:
    #             self.queue_writer.jobs.put(url)
    #         return urls[:half]

    def filter_links(self, url):
        if self.url_filter.Exists(url):
            return False
        domain = utils.get_domain(url)
        if self.domain_filter.Exists(domain):
            return False
        return True         
 
    
                
class CandidatePage:

    def __init__(self, url, domain=None, importance=1, novelty=0):
        self.url = url
        if domain is None:
            self.domain = utils.get_domain(self.url)
        else: 
            self.domain = domain
        self.importance = importance
        self.novelty = novelty
        self.update_priority()

    def update_priority(self):
        self.priority = self.importance / (self.novelty + 1) * -1

    def __str__(self):
        return self.url

    def __lt__(self, other):
        return self.priority < other.priority

    def __gt__(self, other):
        return self.priority > other.priority

    def __eq__(self, other):
        return self.priority == other.priority

    def __le__(self, other):
        return self.priority <= other.priority

    def __ge__(self, other):
        return self.priority >= other.priority
    
    def __ne__(self, other):
        return self.priority != other.priority

    def __hash__(self) -> int:
        return mmh3.hash(self.url, 0)

