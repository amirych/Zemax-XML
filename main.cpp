#include <iostream>
#include <string>
#include <fstream>
#include <cstring>


#define MAX_STRLEN 2048


using namespace std;

int main(int argc, char* argv[])
{

    if ( argc < 2 ) {
        cerr << "Usage: Zemax-XML zemax-config-file [xml-config-file]\n";
        return 1;
    }

    string zemax_file = argv[1];
    string xml_file;

    if ( argc > 2 ) { // user's filename for XML-config
        xml_file = argv[2];
    } else { // generate XML-filename
        xml_file = zemax_file;
        size_t pos = xml_file.find_last_of(".");
        if ( pos != string::npos ) {
            xml_file.replace(xml_file.begin()+pos,xml_file.end(),".xml");
        } else {
            xml_file.append(".xml");
        }
    }

    // some checks
    if ( xml_file == zemax_file ) {
        xml_file.append(".xml");
    }

    cout << xml_file << endl;

    ifstream zfile;
    ofstream xfile;

    char zemax_str[MAX_STRLEN];


    zfile.open(zemax_file.c_str());
    if ( zfile.fail() ) {
        cerr << "Cannot open input ZEMAX file!!!\n";
        return 10;
    }

    xfile.open(xml_file.c_str());
    if ( xfile.fail() ) {
        cerr << "Cannot open output XML file!!!\n";
        return 10;
    }

    // parse ZEMAX file

    int exit_flag = 0;
    bool start_flag = false;

    do {
        zfile.getline(zemax_str,MAX_STRLEN-1);

        if ( zfile.eof() ) break;

        if ( zfile.fail() || zfile.bad() ) {
            cerr << "Error while reading ZEMAX file!!!\n";
            exit_flag = 20;
            break;
        }

        if ( strncmp(zemax_str,"SURF",4) ) { // start of surface description
            start_flag = true;
        }


    } while ( !exit_flag );

    zfile.close();
    xfile.close();

    return exit_flag;
}

