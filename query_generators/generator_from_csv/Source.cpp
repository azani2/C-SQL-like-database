#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
using namespace std;

unordered_map<string, pair<int, string>> COLTYPES = { {"i",{ 1, "Int"}}, {"d",{2, "Double"}}, {"s",{3, "String"}} };

bool valid_char(char c) {
	return ((int)c >= 33 && (int)c <= 126);
}

void create_if_not_exists(const string& full_filename) {
	ifstream fin(full_filename);
	if (!fin.good()) {
		fin.close();
		std::fstream fs(full_filename, std::ios::out | std::ios::app);
		fs.close();
		return;
	}
	fin.close();
}

string gerenerate_create_query_from_header(const string& header, const string& table_name, const vector<pair<int, string>>& v) {
	size_t len = header.length();
	string create_query = "CreateTable " + table_name + " (";
	string buf = ""; 
	size_t col_idx = 0;
	for (size_t i = 0; i < len; i++) {
		if (header[i] == ')') {
			buf += "\\)";
			continue;
		}
		if (header[i] == ' ' || header[i] == '\t') {
			continue;
		}
		if (header[i] == ',') {
			create_query += buf + ":" + v[col_idx++].second + ", ";
			buf.clear();
		}
		else {
			if (!valid_char(header[i])) {
				continue;
			}
			buf += header[i];
		}
	}
	create_query += buf + ":" + v[col_idx++].second + ")";
	return create_query;
}

string row_from_line(const string& line) {
	size_t len = line.length();
	string row = "(";
	string buf = "";
	size_t col_idx = 0;
	bool in_quotes = false;
	for (size_t i = 0; i < len; i++) {
		if (line[i] == '"') {
			in_quotes = !in_quotes;
		}
		if (line[i] == ')') {
			buf += "\\)";
			continue;
		}
		if (line[i] == ',' && in_quotes) {
			buf += "\\,";
			continue;
		}
		if (line[i] == ',' && !in_quotes) {
			row += buf +  ", ";
			buf.clear();
		}
		else {
			buf += line[i];
		}
	}
	row += buf + ")";
	return row;
}


int main() {
	string file_name;
	cout << "Enter a csv dataset filename: ";
	getline(cin, file_name);

	string table_name;
	cout << "Enter a name for the table: ";
	cin >> table_name;

	size_t col_count;
	cout << "Enter column count: ";
	cin >> col_count;

	vector<pair<int, string>> coltypes;
	for (size_t i = 0; i < col_count; i++) {
		cout << "Enter a column type (i, s or d): ";
		string colt;
		cin >> colt;
		if (!COLTYPES.count(colt)) {
			cout << "Invalid column type.\n";
			i--;
			continue;
		}
		coltypes.push_back(COLTYPES.at(colt));
	}


	std::string line = "";

	ifstream fin(file_name);
	getline(fin, line);

	create_if_not_exists("create_query.txt");
	create_if_not_exists("insert_query.txt");
	ofstream fout("create_query.txt", ios::trunc);

	string cr_q = gerenerate_create_query_from_header(line, table_name, coltypes);
	fout << cr_q << endl;
	fout.close();
	cout << cr_q << endl;

	line.clear();
	fout = ofstream("insert_query.txt", ios::trunc);

	string ins_q = "Insert INTO " + table_name + " {";
	bool first = true;
	while (getline(fin, line)) {
		if (!first) {
			ins_q += ", ";
		}

		ins_q += row_from_line(line);

		if (ins_q.length() >= 2'000) {
			fout << ins_q;
			ins_q.clear();
		}
		first = false;
	}
	ins_q += "}";
	fout << ins_q << endl;
	fout.close();
}