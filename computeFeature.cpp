#include <iostream>

#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <locale>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "AssocativeArray.hpp"
/*namespace*/
using namespace boost;
using namespace posix_time;
using namespace std;
/*function declaration*/
void load_Popularity(const string popularityFile);
ptime parseFromString(string _time);
void read_Log(string logFile, int session_time, string outputFileName);
/*Structure dec*/
struct Feature {
	double num_request; // number of requests per session
	double num_success_requests; //number of successful requests
	double page_popularity; // sum of page popularities
	double download_rate; // sum of replay size
	time_duration client_session_time; // time between first and last request
	ptime current_time; //holds current time for request interarrival calculation
	ptime firstTime; // stores the first request time
	time_duration request_inter_arrival; // request inter arrival
};
struct WebObject {
	int id; //object Id
	double popularity;
};

/*Web object address*/
AssocArray<WebObject> object; // stores popularity of each url or object
/*Vector of feature structure*/
AssocArray<Feature> feature; //stores extracted features from the logs

int main() {
	string log;
	int session_time;
	cout << "Enter the log file name" << endl;
	cin >> log;
	cout << "Enter the session time (seconds)" << endl;
	cin >> session_time;
	load_Popularity("finalPopFile.csv");
	read_Log(log, session_time, "dataset.csv");

	return 0;
}
ptime parseFromString(string _t) {
	string _time = _t;
	trim_left_if(_time, is_any_of("["));
	ptime t;
	try {
		stringstream ss(_time.c_str());
		time_input_facet* timefacet = new time_input_facet("%d/%b/%Y:%H:%M:%S");
		ss.imbue(locale(locale::classic(), timefacet));
		// turn on error
		ss.exceptions(std::ios_base::failbit);
		ss >> t;
	} catch (std::exception& e) {
		cout << e.what() << endl;
	}
	return t;

}
void load_Popularity(const string popularityFile) {
	string url[90000]; // store URL
	fstream file;
	file.open("object_mappings.sort");

	if (!file.is_open()) {
		cout << "File not found in the directory" << endl;
		exit(EXIT_FAILURE);
	}

	string _id, urlName, line;
	int id;

	while (getline(file, line)) {
		trim(line);
		vector<string> values;
		split(values, line, is_any_of(" "));

		try {
			id = boost::lexical_cast<int>(values[0]);

		} catch (boost::bad_lexical_cast & e) {
			std::cout << "Exception caught : " << e.what() << _id << std::endl;
		}
		url[id] = values[1];
	}

	file.close();

	file.open(popularityFile.c_str());
	if (!file.is_open()) {
		cout << popularityFile << " not found in the directory" << endl;
		exit(EXIT_FAILURE);
	}

	//read popularity from the file

	while (getline(file, line)) {
		//split the line by ","
		vector<string> values;
		trim(line);
		split(values, line, is_any_of(","));
		//build a structure
		WebObject wo;
		//cout << values[0]<<","<<values[1] <<endl;
		try {
			wo.id = boost::lexical_cast<int>(values[0]);
			//cout << url[wo.id] << endl;
		} catch (boost::bad_lexical_cast & e) {
			std::cout << "Exception caught : " << e.what() << _id << std::endl;
		}

		try {
			wo.popularity = boost::lexical_cast<double>(values[1]);

		} catch (boost::bad_lexical_cast & e) {
			std::cout << "Exception caught : " << e.what() << _id << std::endl;
		}
		object.AddItem(url[wo.id], wo);

	}
	file.close();
}

void read_Log(string logFile, int session_time, string outputFileName) {
	//create output file
	ofstream output(outputFileName.c_str(), fstream::out | fstream::app);
	if (output.is_open()) {
		cout << "output file ready to run" << endl;
	} else {
		cout << "output file failed" << endl;
	}

	//1.Open server log file
	fstream file;
	file.open(logFile.c_str());

	if (!file.is_open()) {
		cout << logFile << " not found in the directory" << endl;
		exit(EXIT_FAILURE);
	}
	int count = 1;
	// read through the log file
	string line;

	//file>>_id>>_user>>_password>>_time>>_offset>>_request>>_url>>_http>>_code>>_reply
	ptime sessionEnd; // used to track session end time

	// read first request for initialization
	getline(file, line);
	vector<string> token;
	trim(line);
	split(token, line, is_any_of(" "));

	Feature f;
	f.current_time = parseFromString(token[3]);
	f.firstTime = f.current_time; // assign current time as first time
	sessionEnd = f.current_time + seconds(session_time);
	if (starts_with(token[8], "2")) {
		//success code
		f.num_success_requests = 1;
	}
	try {
		f.download_rate = lexical_cast<double>(token[9]);
		f.num_request = 1;
		f.request_inter_arrival = seconds(0);
		if (object.IsItem(token[6])) {
			f.page_popularity = object[token[6]].popularity;
		} else {
			f.page_popularity = 0.0;
		}
		feature.AddItem(token[0], f);

	} catch (boost::bad_lexical_cast & e) {
		std::cout << "Exception caught : " << e.what() << endl;
	}

	//continue reading file
	while (getline(file, line)) {

		vector<string> token;
		trim(line);
		split(token, line, is_any_of(" "));
		ptime t = parseFromString(token[3]);

		if (t > sessionEnd) {
			sessionEnd = parseFromString(token[3]) + seconds(session_time);
			//dump the data to csv file
			for (int i = 0; i < feature.Size(); i++) {
				Feature f = feature[feature.GetItemName(i)];
				time_duration session = f.current_time - f.firstTime;
				output << f.num_request / session_time << ","
						<< f.page_popularity << ","
						<< f.download_rate / session_time << "," << ","
						<< f.num_success_requests / f.num_request << ","
						<< session.seconds() << ",1" << endl;
			}
			feature.clearData();
		} else {
			//search if the request exists
			if (feature.IsItem(token[0])) {
				try {

					if (starts_with(token[8], "2")) {
						//success code
						feature[token[0]].num_success_requests++;
						feature[token[0]].download_rate += lexical_cast<double>(
								token[9]);
					}

					feature[token[0]].num_request++;
					feature[token[0]].request_inter_arrival += t
							- feature[token[0]].current_time;
					feature[token[0]].current_time = t;
					if (object.IsItem(token[6])) {
						feature[token[0]].page_popularity +=
								object[token[6]].popularity;
					}

				} catch (boost::bad_lexical_cast & e) {
					std::cout << "Exception caught : " << e.what() << endl;
				}

			} else {

				try {
					Feature f;
					f.current_time = parseFromString(token[3]);
					f.firstTime = f.current_time;
					if (starts_with(token[8], "2")) {
						//success code
						f.num_success_requests = 1;
						f.download_rate = lexical_cast<double>(token[9]);
					} else {
						f.download_rate = 0;
						f.num_success_requests = 0;
					}

					f.num_request = 1;
					f.request_inter_arrival = seconds(0);
					if (object.IsItem(token[6])) {
						f.page_popularity = object[token[6]].popularity;
					} else {
						f.page_popularity = 0.0;
					}

					feature.AddItem(token[0], f);

				} catch (boost::bad_lexical_cast & e) {
					std::cout << "Exception caught : " << e.what() << endl;
				}

			}
		}

		count++;

		if (count % 100000 == 0) {
			cout << count << "requests processed" << endl;
		}

	}
	output.close(); //close the dataset file
}

