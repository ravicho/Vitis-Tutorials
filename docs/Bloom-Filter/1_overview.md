<table>
 <tr>
   <td align="center"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>2019.2 Vitis™ Application Acceleration Development Flow Tutorials</h1>
   <a href="https://github.com/Xilinx/SDAccel-Tutorials/branches/all">See SDAccel™ Development Environment 2019.1 Tutorials</a>
   </td>
 </tr>
 <tr>
 <td align="center"><h1>Optimizing Accelerated FPGA Applications: Bloom Filter Example
 </td>
 </tr>
</table>

# Overview of Original Application


Document filtering is a process where a system monitors a stream of incoming documents, classifies them according to their content, and selects those that are deemed relevant to a specific user or topic. The ability to filter out irrelevant information and organize relevant information into meaningful categories is important in a world where big data is playing an increasingly central role. Document filtering is used extensively in the everyday querying, retrieval, and analysis of information that helps to identify relevant documents of interest.

 In practical scenarios, the number of documents to be searched for an event can be very large and because the monitoring of events must run in real time, a smaller execution time is required for processing all the documents. In this tutorial, you compute a score for each document, which represents the document relevance. Performance of this application is measured in execution time and throughput for processing all the documents in the database.

 The user's interest is represented by a search array, which contains words of interest and has a weight associated with it, which indicates the prominence of the word based on user's interest. While monitoring the incoming stream of documents, you want to find the weight associated with words stored in the database. A native implementation would access the database for every word in the document to check if a word is present in the database and if present, retrieve the weight of the word. A more optimized approach would be using space-efficient bloom filter in your cache that can report whether a word is present in the database, which reduces the number of expensive database queries.

The bloom filter is a hash-table-based data structure that can be used to test whether an element is present in the dataset. False positive matches are possible, but false negatives are not; in other words, a query returns either "possibly in set" or "definitely not in set". The advantage of using a bloom filter is that it is space-efficient and reduces the number of database queries drastically for data which is not present in the database. A bloom filter is also useful in applications for implementing search engines and database management systems, such as Cassandra and Postgres where it can reduce the number of disk queries thereby increasing performance.

The false-positive rate (the number of cases where the bloom filter incorrectly reports that the data might be present in the database) can be reduced to less than 1% with less than 10 bits per element independent of the size or number of elements in the set.

Depending on the false-positive rate required, the number of hash-functions (number of elements in the set and number of bits in the bloom filter) can be adjusted.

The following figure shows an example of a bloom filter representing the set {x, y, z}. The colored arrows show the positions in the bit array that each set element is mapped to. The element 'w' is not in the set {x, y, z} because it hashes to a one bit-array position containing 0. In the following figure, the number of elements are 18 and the number of hash functions compute for each element is 3.

![](./images/bloom.PNG)

## Implementation

In this example, each document consists of an array of words where each word is a 32-bit unsigned integer comprised of a 24-bit word ID and an 8-bit representing the frequency. The search array consists of words of interest to the user. It represents a smaller set of 24-bit word IDs, where each word ID has a weight associated with it, determining the importance of the word. If you navigate to `bloom` directory and then navigate to the `cpu_src` directory and look at line 65 in the `main.cpp` file, you can see that the bloom filter size is 64 KB, which is implemented as `1L<<bloom_size` where `bloom_size` is defined as 14 in header file `sizes.h` thereby (2^14)*4B = 64 KB.

The score for each document is obtained by the cumulative product of multiplying the weight of word ID with its frequency. The greater the score, the more relevant the document matching the search array.

Example:

* **Search Array**: [{word_1,10},{word_3,20},{word_6,30}]
* **Document**: [{word_1,20} ,{word_2,40},{word_3,50}]
* **Compute Score for document**:  20x10 + 0x40 + 50x20 = 1200

In this step, you will build and run this application to create baseline performance data for the original, non-accelerated application.


</br>
<hr/>
<p align="center"><b><a href="/docs/vitis-getting-started/">Return to Getting Started Pathway</a> — <a href="./README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>

<p align="center"><b>
Start the next step: <a href="./2_experience-acceleration.md">2: Experience the Acceleration </a>
</b></p>
