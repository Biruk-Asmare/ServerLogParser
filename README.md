
# ServerLogParser
C++ code that parses the server log data for world cup 98 dataset and computes some features and statistics. Adopted from world cup 98 dataset tools. This code is used to extract featues or information from server access logs. 
The features are computed using session time value given by the user. For any session time the program generates a CSV dataset containing request rate, page populariy, download rate, request inter-arrival rate and ratio-of successful requests from a server log file.

The code is modified from Worldcup 98 datset tools (ftp://ita.ee.lbl.gov/software/WorldCup_tools.tar.gz). This code uses a C++ library (AssociativeArray.php) for associative array from Arnavguddu (https://www.codeproject.com/Articles/149879/Associative-Array-in-C).

This code depends on boost c++ library, so bosst should be installed before compiling this code.

Page popularity file is required to compute page popularity. The file is a CSV file that contains the page popularity value of each page in a website computed by the ratio of # of requests to a page divided by total number of requests. 
 
