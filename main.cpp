#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <list>
#include <ctime>


#define MAX_STRLEN 2048


using namespace std;


int parse_block(list<string> &block) {
    cout << "\nBLOCK:\n";
    for ( list<string>::iterator it = block.begin(); it != block.end(); ++it ) cout << *it << endl;
}


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

    cout << "Zemax input file: " << zemax_file << endl;
    cout << "XML output file: " << xml_file << endl;

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

    // skip lines upto surface description pat of ZEMAX file

    int exit_flag = 0;

    do {
        zfile.getline(zemax_str,MAX_STRLEN-1);

        if ( zfile.eof() ) {
            exit_flag = 11;
            break;
        }

        if ( zfile.fail() || zfile.bad() ) {
            cerr << "Error while reading ZEMAX file!!!\n";
            exit_flag = 20;
            break;
        }
    } while ( strncmp(zemax_str,"SURF",4) );

    if ( exit_flag ) {
        if ( exit_flag == 11 ) {
            cerr << "There was no surfaces description on the input Zemax file!!!\n";
        }
        zfile.close();
        xfile.close();
        return exit_flag;
    }

    // write XML-file header, generic general and beam description

    time_t rowtime;
    time(&rowtime);
//    struct tm *timeinfo = localtime(&rowtime);

    xfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xfile << "\n";
    xfile << "\n";

    xfile << "<!--\n";
//    xfile << "  The file was generated by Zemax-XML (" << asctime(timeinfo) << ")\n";
    xfile << "  The file was generated by Zemax-XML, " << ctime(&rowtime);
    xfile << "-->\n";

    xfile << "\n";
    xfile << "\n";
    xfile << "<general>\n";
    xfile << "  <name> GENERIC SCHEME </name>\n";
    xfile << "  <result_file>  RT_OUTPUT </result_file>\n";
    xfile << "</general>\n";

    xfile << "\n";
    xfile << "\n";
    xfile << "<beam>\n";
    xfile << "  <type> parallel </type>\n";
    xfile << "  <shape> circle </shape>\n";
    xfile << "  <profile> random </profile>\n";
    xfile << "  <params> 10 0 1 </params>\n";
    xfile << "  <nlambda> 1 </nlambda>\n";
    xfile << "  <center> 0 0 0 </center>\n";
    xfile << "  <range> 0.1 0.2 </range>\n";
    xfile << "  <range_distr> random </range_distr>\n";
    xfile << "</beam>\n";
    xfile << "\n";
    xfile << "\n";


    // start parsing

    list<string> surf_block;

    do {
        zfile.getline(zemax_str,MAX_STRLEN-1);

        if ( zfile.eof() ) {
            exit_flag = 11;
            break;
        }

        if ( zfile.fail() || zfile.bad() ) {
            cerr << "Error while reading ZEMAX file!!!\n";
            exit_flag = 20;
            break;
        }

        if ( strncmp(zemax_str,"CONF",4) ) {
            if ( strncmp(zemax_str,"SURF",4) != 0 ) {
                surf_block.push_back(zemax_str);
            } else {
                exit_flag = parse_block(surf_block);
                surf_block.clear();
            }
        } else break;

    } while ( 1 == 1 );


    zfile.close();
    xfile.close();

    return exit_flag;
}

