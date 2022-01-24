import utils
import mmh3, time
from expiringdict import ExpiringDict
import bloomfilter

bf = bloomfilter.BloomFilter(1000000, 0.01)
print(bf.get_size(1000000, 0.01))
# dict = ExpiringDict(max_len=50, max_age_seconds=5)
# dict['test'] = 0
# print(dict.get("test"))
# time.sleep(6)
# print(dict.get("test"))
# url = 'https://en.wikipedia.org/wiki/Compilation_album'
# url='https://www.youtube.com/user/NewYorkerDotCom'
# seed = 0
# domain = utils.get_domain(url)
# print(domain)
# print(mmh3.hash(domain, 0)%3)

# from bloomfilter import BloomFilter
# from random import shuffle

# n = 20
# p = 0.05
# bloomf = BloomFilter(n,p)
# print("Size of bit array:{}".format(bloomf.size))
# print("False positive Probability:{}".format(bloomf.false_positive_prob))
# print("Number of hash functions:{}".format(bloomf.hash_count))

# word_present = ['abound','abounds','abundance','abundant','accessable',
#                 'bloom','blossom','bolster','bonny','bonus','bonuses',
#                 'coherent','cohesive','colorful','comely','comfort',
#                 'gems','generosity','generous','generously','genial']

# word_absent = ['bluff','cheater','hate','war','humanity',
#                'racism','hurt','nuke','gloomy','facebook',
#                'geeksforgeeks','twitter']

# for item in word_present:
#     bloomf.add(item)
 
# shuffle(word_present)
# shuffle(word_absent)              

# test_words = word_present[:10] + word_absent
# shuffle(test_words)

# for word in test_words:
#     if bloomf.check(word):
#         if word in word_absent:
#             print("'{}' is a false positive!".format(word))
#         else:
#             print("'{}' is probably present!".format(word))
#     else:
#         print("'{}' is definitely not present!".format(word))