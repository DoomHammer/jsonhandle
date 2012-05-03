/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2010,2011 Thomas Davis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "JsonHandle.h"
#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char **argv) {

	if (argc != 2) {
		cerr
				<< "This program reads the output using an iostream from example3 an prints the values to the screen"
				<< endl << "	USAGE: " << argv[0] << " filename" << endl;
		return 1;
	}
	{
		ifstream in(argv[1]);

		if (!in) {
			cout << "Cannot open input file " << argv[1] << endl;
			return 1;
		}

		JsonHandle root;
		while (in.good()) {
			root.clear();
			in >> root;
			if (in.good()) {
				if (root["x"].isDefined()) {
					cout << " x:" << root["x"].longValue(0) << ", y:"
							<< root["y"].doubleValue(0) << endl;
				} else {
					cerr << "Invalid: missed object member 'x'" << endl;
					cerr << "read: " << root << endl;
				}
			}
		}

		in.close();
	}

	cout << "read file " << argv[1] << endl;
#ifdef HAS_JSON_STATS
	JsonHandle::dumpStats();
#endif
	return 0;
}
