#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <list>
#include <ctime>
#include <algorithm>
#include <vector>


#define MAX_STRLEN 2048

#define NO_STRING_KEYWORD_ERR 100
#define NO_NUMERIC_KEYWORD_ERR 101
#define BAD_NUMERIC_KEYWORD_ERR 102

using namespace std;


// function deletes leading and trailing whitesaces
static void trim_spaces(std::string& str, const std::string& whitespace = " \t")
{
    std::size_t strBegin = str.find_first_not_of(whitespace);

    if (strBegin == std::string::npos) {
        str.clear();
        return; // empty string
    }

    std::size_t strEnd = str.find_last_not_of(whitespace);
    std::size_t strRange = strEnd - strBegin + 1;

    str.assign(str, strBegin, strRange);
}


static bool comp_key(string word, string key)
{
    int ret = word.compare(0,6,key);
    if ( ret == 0 ) return true; else return false;
}


static string get_string_value(list<string> &block, string key, bool *is_present)
{
    vector<string> zemax_key(1);

    zemax_key[0] = key;

    list<string>::iterator it = search(block.begin(),block.end(),zemax_key.begin(),zemax_key.end(),comp_key);
    if ( it == block.end() ) {
        *is_present = false;
        return "";
    }

    *is_present = true;
    string val = *it;

    block.erase(it);

    return val.substr(key.length());
}


static vector<double> get_numeric_value(list<string> &block, string key, bool *is_present, bool strict = false)
{
    const char* attr_value;
    char *p;
    string str_value;
    vector<double> nums;
    double val;
    bool ok;

    str_value = get_string_value(block,key,&ok);
    if ( !ok ) {
        *is_present = false;
        return nums;
    }

    *is_present = true;

    attr_value = str_value.c_str();

    // try to convert to doubles
    for (;;) {
        val = strtod(attr_value,&p);
        if ( p!= attr_value ) nums.push_back(val); else break;
        attr_value = p;
    }

    if ( strict ) { // check whether string did contain strictly a number or numeric vector
        if ( attr_value[0] != '\0') {
            throw BAD_NUMERIC_KEYWORD_ERR;
        }
    }

    if ( nums.empty() ) throw BAD_NUMERIC_KEYWORD_ERR;

    return nums;
}

static int parse_block(list<string> &block)
{
    cout << "\n<surface>\n";

    string str_value, sclass, stype, glass;
    vector<double> conic_const, aux_pars;
    bool ok,spheric_surf;

    char* sym_nums = "123456";

    // COMM keyword
    str_value = get_string_value(block,"  COMM",&ok);
    trim_spaces(str_value);
    if ( str_value.empty() ) str_value = "no comment";

    cout << "  <comment> " << str_value << " </comment>\n";


    // CURV keyword
    vector<double> pars = get_numeric_value(block,"  CURV",&ok,false);
    if ( !ok ) {
        return 1;
    }

    // CONI keyword
    spheric_surf = true;
    conic_const = get_numeric_value(block,"  CONI",&ok,false);
    if ( ok ) { // aspherical surface
        spheric_surf = false;
    }

    // TYPE keyword
    str_value = get_string_value(block,"  TYPE",&ok);
    if ( !ok ) {
        cerr << "No TYPE keyword!\n";
    }

    trim_spaces(str_value);


    if ( str_value == "STANDARD" || str_value == "DGRATING" || "TILTSURF") {
        if ( pars[0] == 0 ) {
            sclass = "plane";
        } else {
            sclass = "conic";
        }
    } else if ( str_value == "TOROGRAT" ) {
        sclass = "toric";
    } else if ( str_value == "EVENASPH" ) {
        sclass = "conic";
    } else if ( str_value == "COORDBRK" ) {
        sclass = "aux";
    } else sclass = "unknown";


    // GLASS keyword
    glass = get_string_value(block,"  GLAS",&ok);
    if ( !ok ) { // just AUX surface?!
        sclass = "aux";
    } else {
        trim_spaces(glass);
        size_t pos = glass.find(" ");
        if ( pos != string::npos ) {
            glass = glass.substr(0,pos);
        }
//        cout << "---" << glass << "---" << endl;
    }

    cout << "  <class> " << sclass << " </class>\n";


    cout << "  <type> ";
    if ( str_value == "DGARTING" || str_value == "TOROGRAT" ) {
        stype = "grating";
        cout << stype << " </type>\n";
//        if ( glass.compare(0,6,"MIRROR") != 0 ) { // transmission grating
        if ( glass != "MIRROR" ) { // transmission grating
            cout << "  <grating_type> transparent </grating_type>\n";
            cout << "  <n1> 1.0 </n1>\n";
            cout << "  <n2> " << glass << " </n2>\n";
        } else {
            cout << "  <n1> 1.0 </n1>\n";
            cout << "  <n2> 1.0 </n2>\n";
        }
    } else if ( str_value == "STANDARD" ||  str_value == "EVENASPH" ) {
        if ( sclass == "aux" ) {
            stype = "aux";
            cout << stype << " </type>\n";
        } else {
//            if ( glass.compare(0,6,"MIRROR") == 0 ) {
            if ( glass == "MIRROR" ) {
                stype = "mirror";
                cout << stype << " </type>\n";
            } else {
                stype = "lens";
                cout << stype << " </type>\n";
                cout << "  <n1> 1.0 </n1>\n";
                cout << "  <n2> " << glass << " </n2>\n";
            }
        }
    } else if ( str_value == "COORDBRK" ) {
        stype = "aux";
        cout << stype << " </type>\n";
    }

    if ( str_value == "COORDBRK" ) {
        cout << "  <angles> ";
        for ( int i = 0; i < 3; ++i ) {
            aux_pars = get_numeric_value(block,"  PARM",&ok,false);
            if ( !ok ) {
                cerr << "No PARM keyword!!!\n";
                return 1;
            }
            cout << aux_pars[1] << " ";
        }
        cout << "  </angles>\n";

        cout << "  <distance> ";
        for ( int i = 3; i < 6; ++i ) {
            aux_pars = get_numeric_value(block,"  PARM",&ok,false);
            if ( !ok ) {
                cerr << "No COORDBRK parameter!!!\n";
                return 1;
            }
            cout << aux_pars[1] << " ";
        }
        cout << "  </distance>\n";
    }

    cout << "\n</surface>\n";
    return 0;
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
    string scheme_name;


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
    // find NAME keyword

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

        if ( strncmp(zemax_str,"NAME",4) == 0 ) {
            scheme_name = zemax_str;
            scheme_name = scheme_name.substr(5);
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

    xfile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    xfile << "\n";
    xfile << "\n";

    xfile << "<!--\n";
    xfile << "  The file was generated by Zemax-XML, " << ctime(&rowtime);
    xfile << "-->\n";

    xfile << "\n";
    xfile << "\n";
    xfile << "<general>\n";
    xfile << "  <name> " << scheme_name << " </name>\n";
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

//    init_parser();

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
                if ( exit_flag ) {
                    cerr << "An error occured while parsing surface description block!!!\n";
                    break;
                }
                surf_block.clear();
            }
        } else {
            exit_flag = parse_block(surf_block);
            if ( exit_flag ) {
                cerr << "An error occured while parsing surface description block!!!\n";
                break;
            }
            break;
        }

    } while ( 1 == 1 );


    zfile.close();
    xfile.close();

    return exit_flag;
}

