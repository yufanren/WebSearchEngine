import urllib, tldextract
import requests
import config, rpc

from multiprocessing import Process
from xmlrpc.server import SimpleXMLRPCServer
from xmlrpc.client import ServerProxy
import crawler
from requests_html import HTMLSession


# Methods for starting and seeding multiple nodes 
# also contains code for terminating the node cluster
class CrawlStarter:

    def __init__(self):
        self.completion_count = 0

    def start(self, seeds):
        processes = []
        for id in range(config.NUM_NODES):
            p = Process(target=self.start_crawler, args=(id, seeds))
            processes.append(p)
            p.start()
        self.listen_for_completion()    
        for p in processes:
            p.join()

    def start_crawler(self, id, seed):
        process = crawler.Crawler(id, seed)
        process.crawl()

    def listen_for_completion(self):
        server = SimpleXMLRPCServer(('127.0.0.1', 9080), allow_none=True, logRequests=False)
        server.register_function(self.register_completion)

        while self.completion_count < config.NUM_NODES:
            server.handle_request()
            print(f'{self.completion_count} nodes completed.')
        self.kill_servers()

    def register_completion(self):
        self.completion_count += 1;

    def kill_servers(self):
        for i in range(config.NUM_NODES):
            proxy = ServerProxy('http://127.0.0.1:' + str(8080 + i))
            try:
                proxy.kill()
            except Exception as e:
                print(e)


def scrape_google(search_engine, query, num):

    query = urllib.parse.quote_plus(query)
    response = get_source(search_engine + query)
    
    links = list(response.html.absolute_links)
    unique_tld = set()
    top_links, count = [], 0

    for url in links:
        if 'google' in url:
            continue
        tld = tldextract.extract(url)
        if tld not in unique_tld:
            unique_tld.add(tld)
            top_links.append(url)
            count += 1
        if count >= num:
            break

    return top_links

def get_source(url):

    try:
        session = HTMLSession()
        response = session.get(url)
        return response
    except requests.exceptions.RequestException as e:
        print(e)

if __name__ == "__main__":
    print("seeding...")
    seeds = scrape_google(config.SEED_ENGINE, config.SEED_QUERY, config.NUM_NODES*10)
    client = rpc.RPCClient(application=None, server=0)
    client.send_urls(0, seeds)
    starter = CrawlStarter()
    starter.listen_for_completion()