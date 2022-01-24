import math
import mmh3
import threading
from bitarray import bitarray

# Memory efficient data structure
# used to test if an element is a member of a set
class BloomFilter(object):
    
    def __init__(self, num_items, fp_prob):
        self.false_positive_prob = fp_prob
        self.size = self.get_size(num_items, fp_prob)
        self.hash_count = self.get_hash_count(self.size, num_items)
        self.bit_array = bitarray(self.size)
        self.bit_array.setall(0) 
        self.lock = threading.Lock()
        

    def Insert(self, item):
        # print(f'bloom filter check {item}')
        with self.lock:
            for i in range(self.hash_count):
                digest = mmh3.hash128(item, i) % self.size
                self.bit_array[digest] = True

    def Exists(self, item):
        with self.lock:
            for i in range(self.hash_count):
                digest = mmh3.hash128(item, i) % self.size
                if self.bit_array[digest] == False:
                    return False
            return True

    @classmethod
    def get_size(self, num, prob):
        m = -1 * (num * math.log(prob)) / (math.log(2) ** 2)
        return int(m)

    @classmethod
    def get_hash_count(self, size, num):
        k = (size / num) * math.log(2)
        return int(k)
