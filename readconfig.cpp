//
// Created by andrewz on 3/4/23.
//

#include "readconfig.h"
#include <iostream>
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <cstdio>
using namespace std;
using namespace rapidjson;
int exposure, width, height;
int getVals() {
    // Open the file
    FILE * fp = fopen("../config.json", "rb");

    // Check if the file was opened successfully
    if (!fp) {
        cerr << "Error: unable to open file"
                  << endl;
        return 1;
    }

    // Read the file into a buffer
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer,
                                 sizeof(readBuffer));

    // Parse the JSON document
    Document doc;
    doc.ParseStream(is);

    // Check if the document is valid
    if (doc.HasParseError()) {
        cerr << "Error: failed to parse JSON document"
                  << endl;
        fclose(fp);
        return 1;
    }

    // Close the file
    fclose(fp);

    exposure = doc["camProperties"]["exposureTime"].GetInt();
    width = doc["camProperties"]["width"].GetInt();
    height = doc["camProperties"]["width"].GetInt();

//    cout << doc["camProperties"]["exposureTime"].GetInt() << endl;
//    cout << doc["camProperties"]["width"].GetInt() << endl;
//    cout << doc["camProperties"]["height"].GetInt() << endl;
    return 0;
}

