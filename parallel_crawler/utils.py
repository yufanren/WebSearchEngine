import tldextract
from urllib.parse import urlparse, urljoin

def get_domain(url):
    tld = tldextract.extract(url)
    return f'{tld.domain}.{tld.suffix}'

def is_absolute_url(url):
    return bool(urlparse(url).netloc)

# removes query/ref from a url, as urlparse sometimes miss
def url_remove_query_ref(url):
    pos = url.find('?')
    url = url[:pos if pos > 0 else len(url)]
    pos = url.find('ref=')
    url = url[:pos if pos > 0 else len(url)]
    return url

# strps queries, fragments etc from url
def clean_url(url):
    cleaned_url = urljoin(url, urlparse(url).path)
    return  url_remove_query_ref(cleaned_url)

# join url and its link, removing queries, fragments etc at the same time
def normalize_url(base_url, link):
    if is_absolute_url(link):
        link = clean_url(link)
    else:
        link = urlparse(link).path
    return urljoin(base_url, link)

# modify list of parsed links to correct absolute urls.
def normalize_url_list(base_url, links):
    return [normalize_url(base_url, link) for link in links]