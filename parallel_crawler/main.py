import seed, config
import time

# automatically start a number of nodes according to config.py and seed the nodes.
if __name__ == "__main__":
    start_time = time.time()
    seeds = seed.scrape_google(config.SEED_ENGINE, config.SEED_QUERY, config.NUM_NODES*10)
    seed.CrawlStarter().start(seeds)
    print(f'total time: {time.time() - start_time} seconds')