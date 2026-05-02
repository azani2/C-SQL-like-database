#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <vector>
#include <unordered_map>
using namespace std;
mt19937 mt(21052003);
const int MAX_ABS_INT = 1'000;
const int STRING_SIZE = 10;
unordered_map<string, int> COLTYPES = { {"i", 1}, {"d",2}, {"s",3}, {"dt",4}, {"dtm",5} };

int generate_int(int min, int max, bool positive = false) {
    int res = mt() % (max + 1 - min) + min;
    if (positive) return res;
    if (mt() % 2) return (res * (-1));
    return res;
}

char generate_char() {
    int x = generate_int(33, 126, true);
    return (char)x;
}

std::string generate_date() {
    int year = generate_int(1000, 2026, true);
    int day = generate_int(1, 28, true);
    int month = generate_int(1, 12, true);
    string res = to_string(year) + "-";
    if (month < 10) res += "0";
    res += to_string(month) + "-";
    if (day < 10) res += "0";
    res += to_string(day);
    return res;
}

std::string generate_datetime() {
    int hr = generate_int(0, 23, true);
    int min = generate_int(0, 59, true);
    int sec = generate_int(0, 59, true);
    std::string res = generate_date();
    res += " ";
    if (hr < 10) res += "0";
    res += to_string(hr) + ":";
    if (min < 10) res += "0";
    res += to_string(min) + ":";
    if (sec < 10) res += "0";
    res += to_string(sec);
    return res;
}

std::string generate_double(int min, int max, bool positive = false) {
    int head = generate_int(min, max, positive);
    std::string res = to_string(head) + ".";
    int tail = generate_int(0, MAX_ABS_INT - 1, true);
    res += to_string(tail);
    return res;
}

// doesn't generate empty strings
//, and ) are replaced with \, and \)
std::string generate_text(const size_t max_size) {
    int size = 1 + mt() % max_size;
    string res = "\"";
    for (size_t i = 0; i < size; i++) {
        char c = generate_char();
        if (c == ',' || c == ')')
            res += '\\'; i++;
        res += c;
    }
    return res + "\"";
}

string generate_row(const vector<int>& coltypes) {
    string res = "(";
    for (auto t : coltypes) {
        switch (t) {
        case 1:
            res += to_string(generate_int(0, MAX_ABS_INT)) + ", ";
            break;
        case 2:
            res += generate_double(0, MAX_ABS_INT) + ", ";
            break;
        case 3:
            res += generate_text(STRING_SIZE) + ", ";
            break;
        case 4:
            res += generate_date() + ", ";
            break;
        case 5:
            res += generate_datetime() + ", ";
            break;
        default:
            break;
        }
    }
    res.pop_back();
    res.pop_back();
    res += ")";
    return res;
}

int main() {
    string q = "Insert INTO ";
    cout << "Enter table name: ";
    string tb_name;
    cin >> tb_name;

    q.append(tb_name);
    q += " {";

    cout << "Enter rows count: ";
    size_t n;
    cin >> n;
    cout << "Enter columns count: ";
    size_t m;
    cin >> m;
    cout << "Column types: i, d, s, dt, dtm\n";

    vector<int> coltypes;
    for (size_t i = 0; i < m; i++) {
        cout << "Enter a column type: ";
        string colt;
        cin >> colt;
        if (!COLTYPES.count(colt)) {
            cout << "Invalid column type.\n";
            i--;
            continue;
        }
        coltypes.push_back(COLTYPES.at(colt));
    }
    
    for (size_t i = 1; i <= n; i++) {
        q += generate_row(coltypes);
        if (i < n) q += ", ";
    }
    q += "}";

    cout << q << endl;

    ofstream fout("insert_query.txt", ios::trunc);
    fout << q << endl;
    fout.close();
}