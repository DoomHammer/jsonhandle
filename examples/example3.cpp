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
				<< "This program creates 10 JsonHandles and saves them to a single file using an iostream"
				<< endl << "	USAGE: " << argv[0] << " filename" << endl;
		return 1;
	}

	// This demos the use of the stream interface to send a bunch of objects as if it was over a network.
	// Normally, if there is a set of objects known, one would just send a single array, but we will
	// send an object at a time.
	{
		ofstream out(argv[1]);

		if (!out) {
			cout << "Cannot open output file " << argv[1] << endl;
			return 1;
		}
		out.precision(8);

		cout << "Writing the following data..." << endl;

		JsonHandle root;
		for (int i = 1; i <= 10; i++) {
			cout << " x:" << i << ", y:" << (double) 1 / (double) i << endl;

			root["x"].setLong(i);
			root["y"].setDouble((double) 1 / (double) i);
			out << root;
		}
		out.close();

		cout << "Wrote file " << argv[1] << endl;
	}

#ifdef HAS_JSON_STATS
	JsonHandle::dumpStats();
#endif

	return 0;
}
