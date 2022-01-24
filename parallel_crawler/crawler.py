import threading
import requests
import sys
import fileio, config, utils
import time
from datetime import datetime

from bs4 import BeautifulSoup
from application import CrawlingApplication as app
from concurrent.futures import ThreadPoolExecutor
from functools import lru_cache
from urllib.parse import urlparse
from urllib.robotparser import RobotFileParser

# The crawling system downloads the requested pages and supplies them to the crawling application 
# for analysis and storage. The crawling system is in charge of how to fetch those pages, how to 
# do preliminary clean-ups of the urls and how to implement “politeness policies” etc.
class Crawler:

    def __init__(self, id, seed=None) -> None:
        start_time = str(datetime.now())
        self.application = app(id, seed, start_time) 
        self.logger = fileio.Logger({"id": id,
                                     "folder": "complete",
                                     "time": start_time,
                                     "limit": config.CRAWLED_URLS_SIZE_LIMIT})
        self.port = 80060 + id
        self.header = {'User-Agent': 'Mozilla/5.0 (compatible; yufan_bot http://engineering.nyu.edu/~suel/cs6913/'} 
        self.continue_work = True
        self.robot_parser = RobotParse()
        self.black_list = BlackList()

        self.page_completed = 0
        self.page_completed_lock = threading.Lock()

    def crawl(self):
        start_time = time.time()
        logger_thread = self.start_log()
        # self.logger.jobs.put('Completion Time\t\t\t\tDepth\t\tReturn Code\t\tResponse Size\t\tURL')
        thread_count = config.THREAD_COUNT
        with ThreadPoolExecutor(max_workers=thread_count) as executor:
            for _ in range(thread_count):
                executor.submit(self.worker)
        executor.shutdown(wait=True)    
        print(f'Queue size is: {self.application.queue.qsize()}')
        logger_thread.join()
        self.application.queue_writer_thread.join()
        self.application.url_out_client.register_completion() 

    def start_log(self):
        thread = self.logger.start()
        self.logger.jobs.put("COMPLETED URLS\n")
        return thread        

    def worker(self):
        # while self.continue_work:
        while True:
            next_url = self.application.get()
            response = self.fetch_page(next_url)
            if not response:
                continue
            urls = self.parse_links(response, next_url)
            self.insert_links(urls)
            self.logger.jobs.put(next_url)

            with self.page_completed_lock:
                self.page_completed += 1
                if self.page_completed > config.PAGE_LIMIT_PER_NODE:
                    break
           
        self.logger.jobs.put(None)
        self.application.queue_writer.jobs.put(None)

    def fetch_page(self, url):
        if not self.robot_parser.is_robot_allowed(url):
            return
        try:
            response = requests.get(url, headers=self.header, timeout=config.TIMEOUT_LIMIT)
        except Exception:
            return
        return response

    # check and parse the http Response
    def parse_links(self, http_response, base_url):

        if not self.check_MIME_type(http_response):
            return []

        soup = BeautifulSoup(http_response.text, 'html.parser')
        links = soup.find_all('a')
        urls = [link.get('href') for link in links]
        return self.normalize_url_list(urls, base_url)

    # make sure the response is of type 'text/html'
    def check_MIME_type(self, response):
        content_type = response.headers['content-type']
        if not content_type:
            return False
        return 'text/html' in content_type

    def insert_links(self, links):
        self.application.put(links)

    def normalize_url_list(self, urls, base_url):
        result = []
        for url in urls:
            url = utils.normalize_url(base_url, url)
            if not self.black_list.in_black_list(url):
                result.append(url)
        return result


# class implements the robot exclusion protocol
class RobotParse:

    def is_robot_allowed(self, url):

        parser = self.get_parser(self.get_site(url))
        if not parser:
            return True
        return parser.is_robot_allowed('*', url)

    @lru_cache(maxsize=200)
    def get_parser(self, url):
        if url == "":
            return False
        site = 'https://' + url + 'robots.txt'
        try:
            parser = RobotFileParser(site)
            parser.read()
        except Exception:
            return False

    def get_site(self, url):
        return urlparse(url).netloc        

# A black list of file extensions not to download
class BlackList:

    def __init__(self):
        list = ['.mp4','.mp3','.jpg','.gif','.avi','.mov','.navi','.rm','.asf','.flv','.3gp','.wma','.rmvb','.mpg','.mkv','.png','.jpeg','.svg','.tif','.js']
        self.black_list = set(list)

    def in_black_list(self, url):
        
        suffix = url[url.rfind('.'):]
        return suffix in self.black_list  

if __name__ == "__main__":
    arg_length = len(sys.argv)
    if arg_length < 2:
        sys.exit("please specify node id.")
    try:
        id = int(sys.argv[1])
    except:
        sys.exit("node id incorrect")
    node = Crawler(id)
    node.crawl()