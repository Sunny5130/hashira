#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <gmpxx.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Lagrange interpolation at point x (usually x=0)
mpz_class lagrangeInterpolation(const vector<pair<mpz_class, mpz_class>>& points, const mpz_class& x) {
    mpq_class result = 0;

    int n = points.size();
    for (int i = 0; i < n; i++) {
        mpz_class xi = points[i].first;
        mpz_class yi = points[i].second;
        mpq_class term = yi;

        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            mpz_class xj = points[j].first;
            mpz_class numerator = x - xj;
            mpz_class denominator = xi - xj;
            // Multiply term by numerator/denominator
            term *= mpq_class(numerator, denominator);
        }
        result += term;
    }

    // Convert rational result to integer (rounded)
    mpz_class res = result.get_num() / result.get_den();
    mpz_class remainder = result.get_num() % result.get_den();
    if (remainder * 2 >= result.get_den()) res += 1; // round up if needed
    return res;
}

// Reads JSON from file and extracts points, then computes secret
mpz_class solveSecretFromFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) throw runtime_error("Cannot open file: " + filename);

    json j;
    fin >> j;

    // Read n and k
    int n = j["keys"]["n"];
    int k = j["keys"]["k"];

    vector<pair<mpz_class, mpz_class>> points;

    for (auto it = j.begin(); it != j.end(); ++it) {
        string key = it.key();
        if (key == "keys") continue;

        int x = stoi(key);
        int base = stoi(it.value()["base"].get<string>());
        string val = it.value()["value"].get<string>();

        mpz_class y(val, base);
        points.emplace_back(mpz_class(x), y);
    }

    // Sort points by x
    sort(points.begin(), points.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

    if ((int)points.size() < k) throw runtime_error("Not enough points");

    vector<pair<mpz_class, mpz_class>> selectedPoints(points.begin(), points.begin() + k);

    // Interpolate polynomial at x=0 (to get constant term)
    return lagrangeInterpolation(selectedPoints, 0);
}

int main() {
    try {
        mpz_class secret1 = solveSecretFromFile("testcase1.json");
        mpz_class secret2 = solveSecretFromFile("testcase2.json");

        cout << "Secret from Test Case 1: " << secret1.get_str() << "\n";
        cout << "Secret from Test Case 2: " << secret2.get_str() << "\n";
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
